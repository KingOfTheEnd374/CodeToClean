// Fill out your copyright notice in the Description page of Project Settings.


#include "WarTornProjectile.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetStringLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Curves/CurveFloat.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Components/SceneComponent.h"
#include "TimerManager.h"
#include "Runtime/Engine/Private/KismetTraceUtils.h"
#include "Runtime/Engine/Public/CollisionQueryParams.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"


AWarTornProjectile::AWarTornProjectile()
{
	// Set this actor to call Tick() every frame
	PrimaryActorTick.bCanEverTick = true;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	SetRootComponent(StaticMesh);
	StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWarTornProjectile::BeginPlay()
{
	Super::BeginPlay();

	// If the bullet is active when spawned
	if (bIsActive)
	{
		Activate(GetActorLocation(), GetActorRotation(), InitialForce, CanDamage, Shooter, Seed);
	}
}

void AWarTornProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate to everyone
	DOREPLIFETIME(AWarTornProjectile, InitialLocation);
}

void AWarTornProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	DeltaT = StepTime;

	if (bIsActive)
	{
		StepExtraTime += DeltaTime;

		// Extra time fills with frames times, do bullet movement steps if it has enough time
		if (StepTime <= StepExtraTime)
		{
			while (StepExtraTime >= StepTime)
			{
				StepExtraTime -= StepTime;
				MoveStep();
			}
		}
	}
}

// Initialize and activate the projectile
void AWarTornProjectile::Activate(FVector StartLoc, FRotator StartRot, float StartForce, bool DoesDamage, APlayerController* ShootingActor, int32 RandSeed)
{
	Seed = RandSeed;

	Stream.Initialize(Seed);

	CanDamage = DoesDamage;

	Shooter = ShootingActor;

	InitialLocation = StartLoc;
	SetActorLocation(InitialLocation);
	SetActorRotation(StartRot);

	InitialForce = StartForce;

	InitialVelocity = GetActorForwardVector() * InitialForce;
	Velocity = InitialVelocity;

	StepExtraTime = StepTime;
	StepTimeLeft = 0.0f;

	bStartedInWall = false;

	bIsFirstFrame = true;
	bIsActive = true;

	Log(this, "Started", this, StaticMesh, GetActorLocation());

	GetWorldTimerManager().SetTimer(DespawnTimer, this, &AWarTornProjectile::DespawnCaller, DespawnTime, false);
}

// Despawn after existing too long
void AWarTornProjectile::DespawnCaller()
{
	//Print("", Velocity);
	Despawn(/*"Ran out of time"*/);
}

void AWarTornProjectile::Despawn(/*FString Cause*/)
{
	if (DestroyOnDespawn)
	{
		Destroy();
	}

	Log(this, "died:"/* + Cause*/, this, StaticMesh, GetActorLocation());
	bIsActive = false;
	GetWorldTimerManager().ClearTimer(DespawnTimer);
	InitialForce = 0.0f;
	InitialLocation = FVector(0.0f, 0.0f, 0.0f);
	InitialVelocity = FVector(0.0f, 0.0f, 0.0f);
	Velocity = FVector(0.0f, 0.0f, 0.0f);
	StepExtraTime = 0.0f;
	StepTimeLeft = 0.0f;

	CanDamage = false;

	Shooter = nullptr;
	HitActor = nullptr;
	HitComponent = nullptr;

	SetActorLocation(FVector(0.0f, 0.0f, 0.0f));
	SetActorRotation(FRotator(0.0f, 0.0f, 0.0f));

	bIsInsideWall = false;
	TimeLeftOutsideWall = 0.0f;
	TimeLeftInsideWall = 0.0f;

	ExitDecalLocation = FVector(0.0f, 0.0f, 0.0f);
	ExitDecalRotation = FRotator(0.0f, 0.0f, 0.0f);
	ShouldCreateExitDecal = true;

	bStartedInWall = false;

	Seed = 0;

}

void AWarTornProjectile::MoveStep()
{

	//if (o100 && (GetActorLocation().X - InitialLoc.X > 10000/*9144.0f*/))
	//{
	//	o100 = false;
	//	Print("100yrds: ", Velocity);
	//}
	//if (o200 && (GetActorLocation().X - InitialLoc.X > 20000/*18288.0f*/))
	//{
	//	o200 = false;
	//	Print("200yrds: ", Velocity);
	//}
	//if (o300 && (GetActorLocation().X - InitialLoc.X > 30000/*27432.0f*/))
	//{
	//	o300 = false;
	//	Print("300yrds: ", Velocity);
	//}
	//if (o400 && (GetActorLocation().X - InitialLoc.X > 40000/*36576.0f*/))
	//{
	//	o400 = false;
	//	Print("400yrds: ", Velocity);
	//}
	//if (o500 && (GetActorLocation().X - InitialLoc.X > 50000/*45720.0f*/))
	//{
	//	o500 = false;
	//	Print("500yrds: ", Velocity);
	//}
	//if (o600 && (GetActorLocation().X - InitialLoc.X > 60000/*54864.0f*/))
	//{
	//	o600 = false;
	//	Print("600yrds: ", Velocity);
	//}
	//if (o700 && (GetActorLocation().X - InitialLoc.X > 70000/*64008.0f*/))
	//{
	//	o700 = false;
	//	Print("700yrds: ", Velocity);
	//}
	//if (o800 && (GetActorLocation().X - InitialLoc.X > 80000/*73152.0f*/))
	//{
	//	o800 = false;
	//	Print("800yrds: ", Velocity);
	//}
	//if (o900 && (GetActorLocation().X - InitialLoc.X > 90000/*82296.0f*/))
	//{
	//	o900 = false;
	//	Print("900yrds: ", Velocity);
	//}
	//if (o1000 && (GetActorLocation().X - InitialLoc.X > 100000/*91440.0f*/))
	//{
	//	o1000 = false;
	//	Print("1000yrds: ", Velocity);
	//}


	StepTimeLeft = DeltaT;

	if (Velocity.Size() < 1000.0f)
	{
		Despawn();
	}

	if (!bIsInsideWall)
	{
		ApplyGravity(DeltaT);
		ApplyAirDrag(DeltaT);
		bStartedInWall = false;
	}
	else
	{
		bStartedInWall = true;
	}

	MoveTrace(StepTime);
}

void AWarTornProjectile::ApplyGravity(float DeltaTime)
{
	Velocity += FVector(0.0f, 0.0f, -(Gravity * DeltaTime));
}

void AWarTornProjectile::ApplyAirDrag(float DeltaTime)
{
	float Area = FMath::Square(Diameter / 2.0f) * PI;
	float DragCoef;
	if (DragCoefficentCurve)
	{
		DragCoef = DragCoefficentCurve->GetFloatValue(Velocity.Size() / 34300.0f);
	}
	else
	{
		DragCoef = 0.297f;
	}
	if (Tumbling)
	{
		DragCoef *= 4.0f;
	}
	float ForceLoss = (0.5f * EnviromentDensity * Area * FMath::Square(Velocity.Size() / 10.0f) * DragCoef) / Mass;
	Velocity -= Velocity.GetSafeNormal() * ForceLoss * DeltaTime;
}

void AWarTornProjectile::MoveTrace(float TimeLeft)
{
	FVector TraceStart;
	if (bIsFirstFrame)
	{
		bIsFirstFrame = false;
		TraceStart = GetActorLocation();
	}
	else
	{
		TraceStart = LastTraceEndLocation;
	}
	FVector TraceEnd = TraceStart + Velocity * TimeLeft;
	TArray<FHitResult> hitResults;

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes = ObjectsToCollide;
	ObjectTypes.Add(UCollisionProfile::Get()->ConvertToObjectType(ECollisionChannel::ECC_GameTraceChannel7));
	

	if (UKismetSystemLibrary::LineTraceMultiForObjects(GetWorld(), TraceStart, TraceEnd, ObjectTypes, true, { /*Shooter*/}, /*DrawBulletPath ? EDrawDebugTrace::Persistent : */EDrawDebugTrace::None, hitResults, true, VelocityToColor(Velocity)/*HasAuthority() ? FLinearColor::Red : FLinearColor::Blue*/, VelocityToColor(Velocity)))
	{
		while (hitResults.Num() > 0 
			&& ((hitResults[0].Component.Get()->ComponentTags.Num() > 0 && hitResults[0].Component.Get()->ComponentTags[0].IsEqual("Suppress"))
			|| hitResults[0].Component.Get()->GetCollisionObjectType() == ECollisionChannel::ECC_PhysicsBody
			|| hitResults[0].PhysMaterial.Get()->SurfaceType == EPhysicalSurface::SurfaceType5))
		{
			Suppress(hitResults[0].Actor.Get(), Shooter);
			hitResults.RemoveAt(0);
		}
		if (hitResults.Num() > 0)
		{
			FHitResult hitResult;
			switch (FindValidTraceEnd(hitResult, hitResults, TraceEnd, TimeLeft))
			{
			case 1:
				if (DrawBulletPath)
				{
					DrawDebugLine(GetWorld(), TraceStart, hitResult.ImpactPoint, VelocityToColor(Velocity), false, BulletPathDrawTime);
				}
				MoveWithCollision(hitResult, TimeLeft);
				break;

			case 2:
				if (DrawBulletPath)
				{
					DrawDebugLine(GetWorld(), TraceStart, TraceEnd, VelocityToColor(Velocity), false, BulletPathDrawTime);
				}
				FullMove(TraceEnd, TimeLeft);
				break;

			case 3:
				if (DrawBulletPath)
				{
					DrawDebugLine(GetWorld(), TraceStart, TraceEnd, VelocityToColor(Velocity), false, BulletPathDrawTime);
				}
				MoveTillOutOfWall(TraceEnd, TimeLeft);
				break;
			}
		}
		else
		{
			if (DrawBulletPath)
			{
				DrawDebugLine(GetWorld(), TraceStart, TraceEnd, VelocityToColor(Velocity), false, BulletPathDrawTime);
			}
			FullMove(TraceEnd, TimeLeft);
		}

	}
	else
	{
		if (DrawBulletPath)
		{
			DrawDebugLine(GetWorld(), TraceStart, TraceEnd, VelocityToColor(Velocity), false, BulletPathDrawTime);
		}
		FullMove(TraceEnd, TimeLeft);
	}

	if (bIsActive)
	{
		// Set the projectile rotation to face forward
		SetActorRotation(UKismetMathLibrary::MakeRotFromX(Velocity.GetSafeNormal()));

		SetActorLocation(LastTraceEndLocation);
	}
}

int8 AWarTornProjectile::FindValidTraceEnd(FHitResult& hitResult, const TArray<FHitResult>& hitResults, FVector& TraceEnd, float TimeLeft)
{
	/*
	* 1 - MoveWithCollision
	* 2 - FullMove
	* 3 - MoveTillOutOfWall
	*/
	hitResult = hitResults[0];
	FVector TraceStart = hitResult.TraceStart;

	if (bIsInsideWall && TraceStart.Equals(hitResult.ImpactPoint, 0.01f))
	{
		for (int32 i = 1; i < hitResults.Num(); i++)
		{
			if (!TraceStart.Equals(hitResults[i].ImpactPoint, 0.01f))
			{
				hitResult = hitResults[i];

				if (hitResults[i].Time * TimeLeft > TimeLeftInsideWall)
				{
					TraceEnd = TraceStart + Velocity * TimeLeftInsideWall + 0.01f * Velocity.GetSafeNormal();
					// Next valid TraceEnd is further than wall end it is currently in
					return 3;
				}

				// Found next valid TraceEnd
				return 1;
			}
		}
		// Found no valid TraceEnd
		return 2;
	}
	// Not inside a wall or TraceEnd is valid
	return 1;
}

void AWarTornProjectile::MoveWithCollision(FHitResult& hitResult, float TimeLeft)
{
	//UKismetSystemLibrary::DrawDebugString(GetWorld(), hitResult.ImpactPoint, FString::FromInt(Velocity.Size() / 100), (AActor*)0, FLinearColor::Red, 10000.0f);

	FVector TraceImpact = hitResult.ImpactPoint;

	LastTraceEndLocation = TraceImpact;

	float time = hitResult.Time * TimeLeft;
	StepTimeLeft = TimeLeft - time;

	if (bIsInsideWall)
	{
		if (TimeLeftInsideWall - time <= 0.0f)
		{
			TimeLeftOutsideWall = DeltaT - TimeLeftInsideWall;
			TimeLeftInsideWall = 0.0f;

			bIsInsideWall = false;

			ApplyGravity(TimeLeftOutsideWall);
			ApplyAirDrag(TimeLeftOutsideWall);

			if (ShouldCreateExitDecal)
			{
				SpawnDecal(ExitDecalLocation, ExitDecalRotation, 0);
			}
		}
		else
		{
			TimeLeftInsideWall -= time;
		}
	}

	if (hitResult.bBlockingHit)
	{
		CheckCollision(hitResult);
	}
	if (StepTimeLeft > 0.0f && bIsActive)
	{
		MoveTrace(StepTimeLeft);
	}
}

void AWarTornProjectile::FullMove(const FVector& TraceEnd, float TimeLeft)
{
	//UKismetSystemLibrary::DrawDebugString(GetWorld(), TraceEnd, FString::FromInt(Velocity.Size() / 100), (AActor*)0, FLinearColor::Red, 10000.0f);
	LastTraceEndLocation = TraceEnd;
	StepTimeLeft = 0.0f;
	float time = TimeLeft;

	if (bIsInsideWall)
	{
		if (TimeLeftInsideWall - time <= 0.0f)
		{
			TimeLeftOutsideWall = DeltaT - TimeLeftInsideWall;
			TimeLeftInsideWall = 0.0f;

			bIsInsideWall = false;

			ApplyGravity(TimeLeftOutsideWall);
			ApplyAirDrag(TimeLeftOutsideWall);

			if (ShouldCreateExitDecal)
			{
				SpawnDecal(ExitDecalLocation, ExitDecalRotation, 0);
			}
		}
		else
		{
			TimeLeftInsideWall -= time;
		}
	}
}

void AWarTornProjectile::MoveTillOutOfWall(const FVector& TraceEnd, float TimeLeft)
{
	//UKismetSystemLibrary::DrawDebugString(GetWorld(), TraceEnd, FString::FromInt(Velocity.Size() / 100), (AActor*)0, FLinearColor::Red, 10000.0f);
	LastTraceEndLocation = TraceEnd;
	StepTimeLeft = TimeLeft - TimeLeftInsideWall;

	TimeLeftInsideWall = 0.0f;

	bIsInsideWall = false;

	if (ShouldCreateExitDecal)
	{
		SpawnDecal(ExitDecalLocation, ExitDecalRotation, 0);
	}
}

void AWarTornProjectile::CheckCollision(const FHitResult& HitResult)
{
	HitActor = HitResult.Actor.Get();
	HitComponent = HitResult.Component.Get();

	if (!bIsActive)
	{
		return;
	}

	UPhysicalMaterial* physicalMaterial = HitResult.PhysMaterial.Get();

	if (CanDamage/* && !bIsInsideWall*/)
	{
		TryDamage(HitActor, HitComponent, HitResult, Shooter);
		/*if (Cast<AWarTornCharacter>(HitActor))
		{
			Log(this, "damaged", HitActor, HitComponent, HitResult.ImpactPoint);
			Shooter->Server_GiveDamage(Cast<AWarTornCharacter>(HitActor), Damage, Bleed, HitResult);	//Need to also handle when shooter is invalid
			//Velocity.Size() / (0.9f * InitialForce); //Damage scaling
		}*/
	}

	FSurfaceData SurfaceData = GetSurfaceData(physicalMaterial->SurfaceType);

	float PenetrationResistance = SurfaceData.PenetrationResistance * 10000.0f;

	// Not penetrable or richochetable
	if (PenetrationResistance < 0.0f)
	{
		//Print("Not penetrable material");
		EndInSurface(HitResult);
		return;
	}

	if (PenetrationResistance != 0.0f && SurfaceData.CanRicochet && CanGrazeRicochet(HitResult.ImpactNormal) && (!SurfaceData.Rough || Stream.FRand() < RoughSurfaceGrazeChance))
	{
		GrazeRicochet(HitResult.ImpactPoint, HitResult.ImpactNormal, SurfaceData.Rough);
		return;
	}

	FVector Dir = Velocity.GetSafeNormal();//Stream.VRandCone(Velocity.GetSafeNormal(), FMath::DegreesToRadians(5.0f));

	if (!bIsInsideWall && PenetrationResistance > 0.0f)
	{
		Dir = Stream.VRandCone(Velocity.GetSafeNormal(), FMath::DegreesToRadians(5.0f));
		/*int32 x = 0;
		do
		{
			// It is fine for now, because up to 20 deg angle is a gaze, so 5 deg angle can't come outside of the wall.
			//FRotator DirR = UKismetMathLibrary::MakeRotFromX(Velocity.GetSafeNormal()) + FRotator(float(RandomIntInRange(500, -500)) / 100.0f, float(RandomIntInRange(500, -500)) / 100.0f, float(RandomIntInRange(500, -500)) / 100.0f);
			//FRotator RandomRot = FRotator(float(RandomIntInRange(500, -500)) / 100.0f, float(RandomIntInRange(500, -500)) / 100.0f, float(RandomIntInRange(500, -500)) / 100.0f);
			FRotator RandomRot = FRotator(Stream.FRandRange(-5.0f, 5.0f), Stream.FRandRange(-5.0f, 5.0f), Stream.FRandRange(-5.0f, 5.0f));

			Dir = RandomRot.RotateVector(Velocity.GetSafeNormal());
			//Dir = UKismetMathLibrary::GetForwardVector(DirR);
			x++;
		}
		// THIS IS BULLSHIT SOLUTION, REWORK THIS
		while (x < 4 && 90.0f > UKismetMathLibrary::DegAcos(FVector::DotProduct(Dir, HitResult.ImpactNormal) / (Dir.Size() * HitResult.ImpactNormal.Size())));

		if (x == 4)
		{
			Dir = Velocity.GetSafeNormal();
		}*/
	}

	FVector ExitLoc;
	FVector ExitNorm;
	float PenetrationDepth = CalculatePenetrationDepth(Dir, HitResult.ImpactPoint, HitResult.Component.Get(), PenetrationResistance, ExitLoc, ExitNorm);
	Log(this, "out", this, StaticMesh, ExitLoc);

	float PenetrationStrenght = PenetrationAbility * (Velocity * 0.01f).SizeSquared();

	if (PenetrationStrenght > PenetrationResistance * CrateringDepth)
	{
		if (PenetrationDepth > 0.0f && PenetrationStrenght > PenetrationResistance * PenetrationDepth)
		{
			Penetrate(HitResult, Dir, ExitLoc, ExitNorm, PenetrationResistance, PenetrationDepth);
			return;
		}
		else
		{
			EndInSurface(HitResult);
			return;
		}
	}
	else if (PenetrationDepth > 0.0f && PenetrationStrenght > PenetrationResistance * PenetrationDepth)
	{
		Penetrate(HitResult, Dir, ExitLoc, ExitNorm, PenetrationResistance, PenetrationDepth);
		return;
	}
	else if (SurfaceData.CanRicochet && Stream.FRand() < RicochetAbility)
	{
		CraterRicochet(HitResult.ImpactPoint, HitResult.ImpactNormal);
		return;
	}
	else
	{
		EndInSurface(HitResult);
		return;
	}
}

void AWarTornProjectile::EndInSurface(const FHitResult& EndHit)
{
	Log(this, "ended in", HitActor, HitComponent, EndHit.ImpactPoint);
	SpawnDecal(EndHit.ImpactPoint, UKismetMathLibrary::MakeRotFromX(EndHit.ImpactNormal), 0);
	if (CanDamage)
	{
		TryDamage(EndHit.Actor.Get(), EndHit.Component.Get(), EndHit, Shooter, true);
	}
	Despawn(/*"Hit a wall"*/);
}

float AWarTornProjectile::CalculatePenetrationDepth(FVector ProjectileVector, FVector EntryLocation, UActorComponent* Surface, float PenetrationResistance, FVector& OutExitLocation, FVector& OutExitNormal)
{
	int32 n = 0;
	float StepLenght = 20.0f;

	do
	{
		n++;
		FVector end = EntryLocation;
		FVector start = end + (ProjectileVector * StepLenght * n);
		TArray<FHitResult> hitResults;

		bool bResult = UKismetSystemLibrary::LineTraceMultiForObjects(
			GetWorld(),
			start,
			end,
			ObjectsToCollide,
			true,
			{},
			EDrawDebugTrace::None,
			hitResults,
			true,
			FLinearColor::Blue,
			FLinearColor::Green
		);

		if (!bResult)
		{
			continue;
		}

		for (int32 i = 0; i < hitResults.Num(); i++)
		{
			FHitResult endHit = hitResults[i];
			UActorComponent* actorBehind = endHit.Component.Get();
			if (!IsValid(actorBehind) || start.Equals(endHit.ImpactPoint, 0.01f))
			{
				continue;
			}
			if (Surface == actorBehind)
			{
				//Check if exit location is inside another bone collider
				if (Cast<USkeletalMeshComponent>(Surface))
				{
					FVector newStart = endHit.ImpactPoint + ProjectileVector * 0.1f;
					FVector newEnd = endHit.ImpactPoint + ProjectileVector * 0.05f;
					FHitResult result;
					if (UKismetSystemLibrary::LineTraceSingleForObjects(GetWorld(), newStart, newEnd, ObjectsToCollide, true, {}, EDrawDebugTrace::None, result, true, FLinearColor::Blue, FLinearColor::Green))
					{
						continue;
					}
				}
				OutExitLocation = endHit.ImpactPoint;
				OutExitNormal = endHit.ImpactNormal;

				return (OutExitLocation - EntryLocation).Size();
			}

		}

	} while (n < 50);

	return -1.0f;
}

void AWarTornProjectile::Penetrate(const FHitResult& PenetrationHit, FVector PenetrationDir, FVector ExitLoc, FVector ExitNorm, float PenetrationResistance, float PenetrationDepth)
{
	Log(this, "penetrated", HitActor, HitComponent, PenetrationHit.ImpactPoint);
	if (PenetrationHit.PhysMaterial.Get()->SurfaceType != EPhysicalSurface::SurfaceType4)
	{
		SpawnDecal(PenetrationHit.ImpactPoint, UKismetMathLibrary::MakeRotFromX(PenetrationHit.ImpactNormal), 0);
	}

	if (bIsInsideWall)
	{
		if (ShouldCreateExitDecal)
		{
			SpawnDecal(ExitDecalLocation, ExitDecalRotation, 0);
		}
	}

	ExitDecalLocation = ExitLoc;
	ExitDecalRotation = UKismetMathLibrary::MakeRotFromX(ExitNorm);
	ShouldCreateExitDecal = PenetrationHit.PhysMaterial.Get()->SurfaceType != EPhysicalSurface::SurfaceType4;

	bIsInsideWall = true;

	LastTraceEndLocation = PenetrationHit.ImpactPoint + PenetrationDir * 0.01f;

	float SpeedLoss = 100.0f * sqrtf((PenetrationResistance * PenetrationDepth) / PenetrationAbility);

	if (SpeedLoss / Velocity.Size() > 0.3f)
	{
		Tumbling = true;
	}

	// 100.0f is to convert m/s to cm/s
	Velocity = PenetrationDir * (Velocity.Size() - SpeedLoss);

	TimeLeftInsideWall = FMath::Max(TimeLeftInsideWall, PenetrationDepth / Velocity.Size());

}

FSurfaceData AWarTornProjectile::GetSurfaceData(EPhysicalSurface SurfaceType)
{
	if (PenetrationResistanceBySurfaceType.Find(SurfaceType))
	{
		return PenetrationResistanceBySurfaceType[SurfaceType];
	}
	else
	{
		return FSurfaceData();
	}
}

bool AWarTornProjectile::CanGrazeRicochet(FVector SurfaceNormal)
{
	// Projectiles can't ricochet while penetrating something
	if (bIsInsideWall)
	{
		return false;
	}

	float SurfaceHitAngle = UKismetMathLibrary::DegAcos(UKismetMathLibrary::Dot_VectorVector(Velocity.GetSafeNormal(), SurfaceNormal)) - 90.0f;

	//Surface hit angle needs to be smaller than max graze angle or else the bullet will try to penetrate
	if (SurfaceHitAngle > MaxGrazeAngle)
	{
		return false;
	}

	return true;
}

void AWarTornProjectile::GrazeRicochet(FVector SurfacePoint, FVector SurfaceNormal, bool RoughSurface)
{
	float SurfaceHitAngle = UKismetMathLibrary::DegAcos(UKismetMathLibrary::Dot_VectorVector(Velocity.GetSafeNormal(), SurfaceNormal)) - 90.0f;
	FVector Reflection = UKismetMathLibrary::GetReflectionVector(Velocity.GetSafeNormal(), SurfaceNormal);

	FVector PerpendicularVector = FVector::CrossProduct(SurfaceNormal, Reflection).GetSafeNormal();

	FVector GrazeDirection;

	float GrazeVelocityLoss;

	if (!RoughSurface)
	{
		FQuat RotationQuat = FQuat(PerpendicularVector, FMath::DegreesToRadians(SurfaceHitAngle - Stream.FRandRange(1.0f, 5.0f)));
		GrazeDirection = RotationQuat.RotateVector(Reflection);
		GrazeVelocityLoss = (-0.25f * (SurfaceHitAngle / MaxGrazeAngle) + 0.9f);
	}
	else
	{
		FQuat RotationQuat = FQuat(PerpendicularVector, FMath::DegreesToRadians(SurfaceHitAngle - 45.0f));
		FVector Dir = RotationQuat.RotateVector(Reflection);
		GrazeDirection = Stream.VRandCone(Dir, FMath::DegreesToRadians(40.0f));
		GrazeVelocityLoss = 0.1f;
	}

	Tumbling = true;

	Velocity = GrazeDirection * (GrazeVelocityLoss * Velocity.Size());

	SpawnDecal(SurfacePoint, UKismetMathLibrary::MakeRotFromX(SurfaceNormal), 1);

	Log(this, "Graze ricochet off", HitActor, HitComponent, SurfacePoint);
}

void AWarTornProjectile::CraterRicochet(FVector SurfacePoint, FVector SurfaceNormal)
{
	//FRotator RandomRot = FRotator(float(RandomIntInRange(7500, -7500)) / 100.0f, float(RandomIntInRange(7500, -7500)) / 100.0f, float(RandomIntInRange(7500, -7500)) / 100.0f);
	//FRotator RandomRot = FRotator(Stream.FRandRange(-75.0f, 75.0f), Stream.FRandRange(-75.0f, 75.0f), Stream.FRandRange(-75.0f, 75.0f));
	//FVector RicochetDir = RandomRot.RotateVector(SurfaceNormal);
	FVector RicochetDir = Stream.VRandCone(SurfaceNormal, FMath::DegreesToRadians(75.0f));

	Tumbling = true;
	Velocity = RicochetDir * (0.3f * Velocity.Size());

	SpawnDecal(SurfacePoint, UKismetMathLibrary::MakeRotFromX(SurfaceNormal), 1);

	Log(this, "Crater richochetted off", HitActor, HitComponent, SurfacePoint);
}

/*int32 AWarTornProjectile::RandomIntInRange(int32 MaxInt, int32 MinInt)
{
	int32 a = 1103515245;
	int32 c = 12345;
	int32 m = 2147483648;

	Seed = (Seed * a + c) % m;

	int32 RandInt = Seed % (MaxInt + 1 - MinInt) + MinInt;

	return RandInt;
}*/

/*bool AWarTornProjectile::RandomBoolWeighted(float Weight)
{
	int32 a = 1103515245;
	int32 c = 12345;
	int32 m = 2147483648;

	Seed = (Seed * a + c) % m;

	float NormalizedRandomNum = Seed / m;

	return NormalizedRandomNum < Weight;
}*/

FColor AWarTornProjectile::VelocityToColor(const FVector& CurrentVelocity)
{
	float Speed = Velocity.Size();

	float SpeedPrecentage = Speed / InitialForce;

	if (SpeedPrecentage >= 0.75f)
	{
		return FColor::Red;
	}
	else if (SpeedPrecentage >= 0.5f)
	{
		return FColor::Orange;
	}
	else if (SpeedPrecentage >= 0.25f)
	{
		return FColor::Green;
	}
	else if (SpeedPrecentage >= 0.1f)
	{
		return FColor::Blue;
	}
	else
	{
		return FColor::White;;
	}
}

void AWarTornProjectile::CreateCavity(const FHitResult& HitResult)
{
}