// Fill out your copyright notice in the Description page of Project Settings.


#include "CoverGenerator.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/StaticMeshActor.h"
//#include "AI/CoverPoint.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

// Sets default values
ACoverGenerator::ACoverGenerator()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("GridOrigin")));
}

// Called when the game starts or when spawned
void ACoverGenerator::BeginPlay()
{
	Super::BeginPlay();

	ChunkCount = FMath::CeilToInt((GridTileCount * GridTileSize) / ChunkSize);

	if (GenerateAtBeginPlay)
	{
		InitCoverChunksArray();
		BatchFinished = true;
		GenerateCoverOverTime();
	}
}

// Called every frame
void ACoverGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACoverGenerator::InitCoverChunksArray()
{
	CoverChunks2D.Empty();
	ChunkCount = FMath::CeilToInt((GridTileCount * GridTileSize) / ChunkSize);

	for (int32 i = 0; i < ChunkCount; i++)
	{
		CoverChunks2D.Add(FCoverChunk2D());
		for (int32 j = 0; j < ChunkCount; j++)
		{
			CoverChunks2D[i].Add(FCoverChunk());
		}
	}
}

void ACoverGenerator::GenerateCoverOverTime()
{
	FTimerHandle Timer;
	if (BatchFinished)
	{
		BatchFinished = false;
		int32 CurrentEnd = FMath::Min(CurrentStart + GeneratePerBatch, GridTileCount * GridTileCount);

		GenerateCover(CurrentStart, CurrentEnd);

		CurrentStart = CurrentEnd;
		if (CurrentEnd < GridTileCount * GridTileCount)
		{
			CurrentEnd += GeneratePerBatch;

			GetWorldTimerManager().SetTimer(Timer, this, &ACoverGenerator::GenerateCoverOverTime, 0.05f, false);
		}
	}
	else
	{
		GetWorldTimerManager().SetTimer(Timer, this, &ACoverGenerator::GenerateCoverOverTime, 0.1f, false);
	}
}

void ACoverGenerator::GenerateCover(int32 Start, int32 End)
{
	int32 XStart = Start / GridTileCount;
	int32 XEnd = End / GridTileCount + 1;
	int32 YStart = Start;
	int32 YEnd = End;
	if (End % GridTileCount == 0)
	{
		XEnd--;
	}

	if (XStart != 0)
	{
		YStart = Start % GridTileCount;
	}

	if (XEnd != 0)
	{
		YEnd = End % GridTileCount;
		if (YEnd == 0)
		{
			YEnd = GridTileCount;
		}
	}
	for (int32 x = XStart; x < XEnd; x++)
	{
		int32 Y1 = 0;
		int32 Y2 = GridTileCount;
		if (x == XStart)
		{
			Y1 = YStart;
		}
		if (x == XEnd - 1)
		{
			Y2 = YEnd;
		}
		for (int32 y = Y1; y < Y2; y++)
		{
			// Grid Point center locations
			float x1 = GetActorLocation().X + x * GridTileSize + GridTileSize / 2.0f;
			float y1 = GetActorLocation().Y + y * GridTileSize + GridTileSize / 2.0f;

			TArray<FHitResult> hitResults;
			float currentDepth = 0.0f;

			while (1)
			{
				FHitResult hitResult;
				UKismetSystemLibrary::LineTraceSingleForObjects(GetWorld(), FVector(x1, y1, GetActorLocation().Z - currentDepth), FVector(x1, y1, GetActorLocation().Z - VerticalDepth), { UEngineTypes::ConvertToObjectType(ECC_WorldStatic) }, true, {}, DrawVerticalTraces ? EDrawDebugTrace::Persistent : EDrawDebugTrace::None, hitResult, true);

				if (!hitResult.bBlockingHit || currentDepth + hitResult.Distance + 10.0f >= VerticalDepth)
				{
					break;
				}
				hitResults.Add(hitResult);
				currentDepth += hitResult.Distance + 10.0f;
			}


			for (int32 z = 0; z < hitResults.Num(); z++)
			{
				float z1 = hitResults[z].ImpactPoint.Z + CapsuleTraceHalfHeight + 5.0f;

				float HalfSize = GridTileSize / 2.0f;

				TArray<FVector> GridTracePoints =
				{
					FVector(x1 + HalfSize, y1, z1),
					FVector(x1, y1 + HalfSize, z1),
					FVector(x1 - HalfSize, y1 , z1),
					FVector(x1, y1 - HalfSize, z1)
				};

				CheckForCover(GridTracePoints);
			}

		}
	}
	BatchFinished = true;
}

void ACoverGenerator::CheckForCover(TArray<FVector> TracePoints)
{
	for (int32 i = 0; i < 4; ++i)
	{
		FVector TraceEnd;
		if (i == 3)
		{
			TraceEnd = TracePoints[0];
		}
		else
		{
			TraceEnd = TracePoints[i + 1];
		}
		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypesArray = { UEngineTypes::ConvertToObjectType(ECC_WorldStatic) };
		FHitResult Hit;
		// Grid square trace
		if (UKismetSystemLibrary::LineTraceSingleForObjects(GetWorld(), TracePoints[i], TraceEnd, ObjectTypesArray, true, {}, DrawGrid ? EDrawDebugTrace::Persistent : EDrawDebugTrace::None, Hit, true, FColor::Red, FColor::Green, 5.0f))
		{
			FVector ImpactNormalXY = FVector(Hit.ImpactNormal.X, Hit.ImpactNormal.Y, 0.0f).GetSafeNormal();

			FVector TraceStart = Hit.ImpactPoint + DistanceFromWall * ImpactNormalXY;
			TraceEnd = TraceStart + FVector(0.0f, 0.0f, -GroundSearchDepth);
			FHitResult Hit2;
			// Ground trace
			if (UKismetSystemLibrary::LineTraceSingleForObjects(GetWorld(), TraceStart, TraceEnd, ObjectTypesArray, true, {}, DrawTraces ? EDrawDebugTrace::Persistent : EDrawDebugTrace::None, Hit2, true, FColor::Red, FColor::Green, 5.0f))
			{
				FVector CoverLocation = Hit2.ImpactPoint + FVector(0.0f, 0.0f, CapsuleTraceHalfHeight + 5.0f);
				if (abs(TracePoints[0].Z - CoverLocation.Z) > 5.0f || UKismetMathLibrary::DegAcos(FVector::DotProduct(FVector(0.0f, 0.0f, 1.0f), Hit2.ImpactNormal)) > 30.0f)
				{
					return;
				}
				if (CapsuleTrace(CoverLocation))
				{
					bool Top = TopTrace(CoverLocation, ImpactNormalXY);
					bool Left = SideTrace(false, CoverLocation, ImpactNormalXY);
					bool Right = SideTrace(true, CoverLocation, ImpactNormalXY);
					if (Top || Left || Right)
					{
						if (CheckForNearbyCoverPoints(CoverLocation))
						{
							SpawnCover(CoverLocation, -ImpactNormalXY, Top, Left, Right);
							AddCover(CoverLocation, ImpactNormalXY, Top, Left, Right);

							if (DrawFoundCover)
							{
								DrawDebugCapsule(GetWorld(), CoverLocation, CapsuleTraceHalfHeight, CapsuleTraceRadius, FQuat(ForceInit), FColor::Blue, true, -1.0f, (uint8)0U, 1.0f);
							}
						}
					}
				}
			}
		}
	}
}

bool ACoverGenerator::CapsuleTrace(FVector Location)
{
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypesArray = { UEngineTypes::ConvertToObjectType(ECC_WorldStatic) };
	TArray<AActor*> Out;
	//DrawDebugCapsule(GetWorld(), Location, CapsuleTraceHalfHeight, CapsuleTraceRadius, FQuat(ForceInit), FColor::Blue, true, -1.0f, (uint8)0U, 1.0f);
	if (!UKismetSystemLibrary::CapsuleOverlapActors(GetWorld(), Location, CapsuleTraceRadius, CapsuleTraceHalfHeight, ObjectTypesArray, AActor::StaticClass()/*AStaticMeshActor::StaticClass()*/, {}, Out))
	{
		return true;
	}
	return false;
}

bool ACoverGenerator::SideTrace(bool isRight, FVector Location, FVector Normal)
{
	float Side;
	if (isRight)
	{
		Side = -SideTraceLenght;
	}
	else
	{
		Side = SideTraceLenght;
	}

	FVector NormalRight = UKismetMathLibrary::GetRightVector(FRotator(0.0f, UKismetMathLibrary::MakeRotFromX(Normal).Yaw, 0.0f));

	FVector TraceStart = Location;
	FVector TraceEnd = TraceStart + Side * NormalRight;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypesArray = { UEngineTypes::ConvertToObjectType(ECC_WorldStatic) };
	FHitResult Hit2;
	if (!UKismetSystemLibrary::LineTraceSingleForObjects(GetWorld(), TraceStart, TraceEnd, ObjectTypesArray, true, {}, DrawTraces ? EDrawDebugTrace::Persistent : EDrawDebugTrace::None, Hit2, true, FColor::Red, FColor::Green, 5.0f))
	{
		TraceStart = TraceEnd;
		TraceEnd = TraceStart + -PeekTraceDepth * Normal;
		if (!UKismetSystemLibrary::LineTraceSingleForObjects(GetWorld(), TraceStart, TraceEnd, ObjectTypesArray, true, {}, DrawTraces ? EDrawDebugTrace::Persistent : EDrawDebugTrace::None, Hit2, true, FColor::Red, FColor::Green, 5.0f))
		{
			return true;
		}
	}
	return false;
}

bool ACoverGenerator::TopTrace(FVector Location, FVector Normal)
{
	FVector TraceStart = Location + FVector(0.0f, 0.0f, TopTraceHeight);
	FVector TraceEnd = TraceStart + -PeekTraceDepth * Normal;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypesArray = { UEngineTypes::ConvertToObjectType(ECC_WorldStatic) };
	FHitResult Hit2;
	if (!UKismetSystemLibrary::LineTraceSingleForObjects(GetWorld(), TraceStart, TraceEnd, ObjectTypesArray, true, {}, DrawTraces ? EDrawDebugTrace::Persistent : EDrawDebugTrace::None, Hit2, true, FColor::Red, FColor::Green, 5.0f))
	{
		return true;
	}
	return false;
}

bool ACoverGenerator::CheckForNearbyCoverPoints(FVector Location)
{
	/*int32 X, Y;

	if (!LocationToChunkIndex(Location, X, Y))
	{
		return false;
	}*/

	TArray<int32> X, Y;

	if (!LocationToNearbyChunkIndexes(Location, MinDistanceBetweenCoverPoints, X, Y))
	{
		return false;
	}

	for (int32 i = 0; i < X.Num(); i++)
	{
		for (int32 j = 0; j < CoverChunks2D[Y[i]][X[i]].Covers.Num(); j++)
		{
			if (FVector::Dist(CoverChunks2D[Y[i]][X[i]][j].Location, Location) < MinDistanceBetweenCoverPoints)
			{
				return false;
			}
		}
	}
	return true;
}

void ACoverGenerator::AddCover(FVector Location, FVector Direction, bool Top, bool Left, bool Right)
{
	int32 X, Y;

	if (!LocationToChunkIndex(Location, X, Y))
	{
		return;
	}

	FCoverData Cover;
	Cover.Location = Location;
	Cover.Direction = Direction;
	Cover.Top = Top;
	Cover.Left = Left;
	Cover.Right = Right;

	CoverChunks2D[Y][X].Add(Cover);
}

bool ACoverGenerator::IsValidChunkIndex(int32 X, int32 Y)
{
	return X >= 0 && X < ChunkCount && Y >= 0 && Y < ChunkCount;
}

bool ACoverGenerator::LocationToChunkIndex(FVector Location, int32& X, int32& Y)
{
	FVector RelativeLoc = Location - GetActorLocation();
	X = RelativeLoc.X / ChunkSize;
	Y = RelativeLoc.Y / ChunkSize;

	// Are indexes valid
	return IsValidChunkIndex(X, Y);
}

bool ACoverGenerator::LocationToNearbyChunkIndexes(FVector Location, float Radius, TArray<int32>& X, TArray<int32>& Y)
{
	FVector RelativeLoc = Location - GetActorLocation();
	int32 XIndex = RelativeLoc.X / ChunkSize;
	int32 YIndex = RelativeLoc.Y / ChunkSize;

	// Are base indexes valid
	if (!IsValidChunkIndex(XIndex, YIndex))
	{
		return false;
	}

	X.Add(XIndex);
	Y.Add(YIndex);

	// Does it have valid extra X and Y chunks? 0 for none, -1 and 1 is direction of next chunk.
	int32 XExtra = 0, YExtra = 0;

	// Where inside Chunk X coordinate is the Location (between 0-1)
	float XPartInChunk = (RelativeLoc.X / ChunkSize) - (int)(RelativeLoc.X / ChunkSize);
	// Check if the Location is distance to the Chunk edge is smaller than Radius, which edge is it closer to, and is the next index valid
	if (FMath::Min(XPartInChunk, 1.0f - XPartInChunk) < Radius && XPartInChunk < 0.5f && IsValidChunkIndex(XIndex - 1, YIndex))
	{
		X.Add(XIndex - 1);
		Y.Add(YIndex);

		XExtra = -1;
	}
	else if (IsValidChunkIndex(XIndex + 1, YIndex))
	{
		X.Add(XIndex + 1);
		Y.Add(YIndex);
		
		XExtra = 1;
	}

	// Where inside Chunk Y coordinate is the Location (between 0-1)
	float YPartInChunk = (RelativeLoc.Y / ChunkSize) - (int)(RelativeLoc.Y / ChunkSize);
	// Check if the Location is distance to the Chunk edge is smaller than Radius, which edge is it closer to, and is the next index valid
	if (FMath::Min(YPartInChunk, 1.0f - YPartInChunk) < Radius && YPartInChunk < 0.5f && IsValidChunkIndex(XIndex, YIndex - 1))
	{
		X.Add(XIndex);
		Y.Add(YIndex - 1);

		YExtra = -1;
	}
	else if (IsValidChunkIndex(XIndex, YIndex + 1))
	{
		X.Add(XIndex);
		Y.Add(YIndex + 1);

		YExtra = 1;
	}

	// If both X and Y have extra chunk, add an extra corner chunk
	if (XExtra != 0 && YExtra != 0 && IsValidChunkIndex(XIndex + XExtra, YIndex + YExtra))
	{
		X.Add(XIndex + XExtra);
		Y.Add(YIndex + YExtra);
	}

	return true;
}