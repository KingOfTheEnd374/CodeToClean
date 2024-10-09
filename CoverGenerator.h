// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoverGenerator.generated.h"

USTRUCT(BlueprintType)
struct FCoverData
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector Location;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector Direction;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool Occupied;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool Top;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool Left;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool Right;
};

USTRUCT(BlueprintType)
struct FCoverChunk
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FCoverData> Covers = {};

	FCoverData& operator[] (int32 i)
	{
		return Covers[i];
	}

	void Add(FCoverData Cover)
	{
		Covers.Add(Cover);
	}
};

USTRUCT(BlueprintType)
struct FCoverChunk2D
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FCoverChunk> CoverChunks;

	FCoverChunk& operator[] (int32 i)
	{
		return CoverChunks[i];
	}

	void Add(FCoverChunk Cover)
	{
		CoverChunks.Add(Cover);
	}
};

UCLASS()
class ACoverGenerator : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACoverGenerator();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FCoverChunk2D> CoverChunks2D;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool GenerateAtBeginPlay = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float GridTileSize = 10.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 GridTileCount = 100;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float VerticalDepth = 10000.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float DistanceFromWall = 30.0f;

	// This is unnecessary, because vertical trace moves up a specific distance, only need to check down the same distance
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float GroundSearchDepth = 200.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float CapsuleTraceHalfHeight = 70.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float CapsuleTraceRadius = 20.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float SideTraceLenght = 35.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float TopTraceHeight = 70.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float PeekTraceDepth = 200.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MinDistanceBetweenCoverPoints = 40.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 GeneratePerBatch = 10;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ChunkSize = 1500.0f;

	UPROPERTY(BlueprintReadOnly)
	int32 ChunkCount;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool DrawFoundCover = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool DrawTraces = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool DrawGrid = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool DrawVerticalTraces = false;

	UFUNCTION(BlueprintCallable)
	void GenerateCover(int32 Start, int32 End);

	UFUNCTION(BlueprintCallable)
	void GenerateCoverOverTime();

	UFUNCTION(BlueprintCallable)
	void InitCoverChunksArray();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsValidChunkIndex(int32 X, int32 Y);

	UFUNCTION(BlueprintCallable)
	bool LocationToChunkIndex(FVector Location, int32& X, int32& Y);

	// Get 2x2 chunks, 2*Radius must be less than ChunckSize, because result is always 2x2 chunks.
	UFUNCTION(BlueprintCallable)
	bool LocationToNearbyChunkIndexes(FVector Location, float Radius, TArray<int32>& X, TArray<int32>& Y);


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void CheckForCover(TArray<FVector> TracePoints);

	UFUNCTION()
	bool CapsuleTrace(FVector Location);

	UFUNCTION()
	bool SideTrace(bool isRight, FVector Location, FVector Normal);

	UFUNCTION()
	bool TopTrace(FVector Location, FVector Normal);

	UFUNCTION()
	bool CheckForNearbyCoverPoints(FVector Location);


	UFUNCTION(BlueprintImplementableEvent)
	bool SpawnCover(FVector Location, FVector Direction, bool Top, bool Left, bool Right);

	UFUNCTION()
	void AddCover(FVector Location, FVector Direction, bool Top, bool Left, bool Right);


private:

	UPROPERTY()
	int32 CurrentStart = 0;

	UPROPERTY()
	bool BatchFinished = false;
};
