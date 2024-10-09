// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "WarTornProjectile.generated.h"

USTRUCT(BlueprintType)
struct FSurfaceData
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite)
	float PenetrationResistance = -1.0f;

	UPROPERTY(BlueprintReadWrite)
	bool Rough = false;

	UPROPERTY(BlueprintReadWrite)
	bool CanRicochet = false;
};

UCLASS()
class WARTORN_API AWarTornProjectile : public AActor
{
	GENERATED_BODY()

public:

	AWarTornProjectile();

	/** Seed used for projectile random rotation changes after penetrations and richochets. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere/*, meta = (ExposeOnSpawn = true)*/)
	int32 Seed;

	UPROPERTY()
	FRandomStream Stream;

	UPROPERTY(BlueprintReadOnly, Replicated)
	FVector InitialLocation;


	/*bool o100 = true;
	bool o200 = true;
	bool o300 = true;
	bool o400 = true;
	bool o500 = true;
	bool o600 = true;
	bool o700 = true;
	bool o800 = true;
	bool o900 = true;
	bool o1000 = true;*/

	//FVector InitialLoc;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool DrawBulletPath = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float BulletPathDrawTime = 10.0f;

	/** Time between ticks. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	float StepTime = 0.05f;

	/** Leftover time from previous frame. */
	UPROPERTY()
	float StepExtraTime = 0.0f;

	/** Time left to move in current tick. */
	UPROPERTY()
	float StepTimeLeft = 0.0f;

	/** Started current tick inside a wall. */
	UPROPERTY()
	bool bStartedInWall = false;

	/** Projectile static mesh component. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UStaticMeshComponent* StaticMesh;

	/** Current velocity in cm/s. */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	FVector Velocity;

	/** Initial velocity in cm/s. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	FVector InitialVelocity;

	/** Initial foce of the projectile for movement. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true))
	float InitialForce;

	/** Mass of the projectile in kg. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	float Mass = 0.008f;

	/** Projectile diameter in m. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	float Diameter = 0.00785f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float EnviromentDensity = 1.2f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	float BallisticCoefficient;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UCurveFloat* DragCoefficentCurve;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Damage;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Pain;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float PenetrationAbility;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float RicochetAbility;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	float CrateringDepth = 1.0f;

	/** Delta time between two ticks in seconds. Updates every tick. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	float DeltaT;

	/** An actor who shot the projectile and can't be damaged by it for X(distance?/time?). */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true))
	APlayerController* Shooter;

	/** Gravitational force that pulls the projectile down in cm/s^2. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Gravity = 980.0f;

	/** Object types the projectile can collide with. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere/*, meta = (ExposeOnSpawn = true)*/)
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectsToCollide;

	/** A map of penetration resistance factors for each surface type. */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere/*, meta = (ExposeOnSpawn = true)*/)
	TMap<TEnumAsByte<EPhysicalSurface>, FSurfaceData> PenetrationResistanceBySurfaceType;

	/** Is the projectile activated. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true))
	bool bIsActive;

	/** Is the projectile visual representation of the real bullet on the client. This projectile can't cause damage. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true))
	bool CanDamage = false;;

	// SHOULD BE RESET WHEN REACTIVATING THE BULLET
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	bool Tumbling = false;

	/** Is it the first frame for the active projectile. */
	UPROPERTY()
	bool bIsFirstFrame;

	/** End location of the last movement trace. */
	UPROPERTY()
	FVector LastTraceEndLocation;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true))
	float DespawnTime = 20.0f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	bool DestroyOnDespawn = false;

	UPROPERTY()
	FTimerHandle DespawnTimer;

	/** If the bullet hit angle on the surface is smaller, the bullet will graze */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MaxGrazeAngle = 30.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float RoughSurfaceGrazeChance = 0.3f;

	/** Is the projectile currently inside a surface. */
	UPROPERTY()
	bool bIsInsideWall = false;

	/** How long in seconds will the projectile be in a wall while penetrating. Updates every tick. */
	UPROPERTY()
	float TimeLeftInsideWall = 0.0f;

	/** How long in seconds has the projectile been outside of a wall in a frame, where it exited the wall. */
	UPROPERTY()
	float TimeLeftOutsideWall = 0.0f;

	/** Penetration exit location where to spawn decal. */
	UPROPERTY()
	FVector ExitDecalLocation;

	/** Rotation which to spawn decal with. */
	UPROPERTY()
	FRotator ExitDecalRotation;

	UPROPERTY()
	bool ShouldCreateExitDecal;

	UPROPERTY()
	AActor* HitActor;

	UPROPERTY()
	UActorComponent* HitComponent;

	/**
	* Generate random integer in range from seed.
	*
	* @param Maxint - Maximum return value.
	* @param Minint - Minimum return value.
	*
	* @return Random integer.
	*/
	/*UFUNCTION(BlueprintCallable)
	int32 RandomIntInRange(int32 MaxInt, int32 MinInt);*/

	/*UFUNCTION(BlueprintCallable)
	bool RandomBoolWeighted(float Weight);*/

	UFUNCTION()
	FColor VelocityToColor(const FVector& CurrentVelocity);

	UFUNCTION(BlueprintImplementableEvent)
	void Log(AWarTornProjectile* BulletSelf, const FString& LogEntry, AActor* HitAct, UActorComponent* HitComp, FVector Location);

	UFUNCTION(BlueprintImplementableEvent)
	void Print(const FString& Text, FVector Vel);


	/** Test Debug function to run code in Blueprints. */
	UFUNCTION(BlueprintImplementableEvent)
	void DoBP(FVector StartT, FVector EndT);

	UFUNCTION(BlueprintImplementableEvent)
	void SpawnDecal(FVector DecalLoc, FRotator DecalRot, int32 Type);

	/** Take the projectile from the projectiles pool, initialize and activate it. */
	UFUNCTION(BlueprintCallable)
	void Activate(FVector StartLoc, FRotator StartRot, float StartForce, bool DoesDamage, APlayerController* ShootingActor, int32 RandSeed);

	UFUNCTION()
	void DespawnCaller();

	/** Deactivate the projectile and move it back to the projectiles pool. */
	UFUNCTION(/*Server, Unreliable, */BlueprintCallable)
	void Despawn(/*FName Cause*/);

	UFUNCTION()
	void EndInSurface(const FHitResult& EndHit);

	/**
	* Check if the projectile can graze off from the surface.
	*
	* @param SurfaceNormal - Hit surface normal that will be grazed off.
	*
	* @return Can the projectile graze off that surface.
	*/
	UFUNCTION()
	bool CanGrazeRicochet(FVector SurfaceNormal);

	/**
	* Calculate new velocity and direction of the bullet after ricochet.
	*
	* @param SurfacePoint32 - The location where the projectile will ricochet.
	* @param SurfaceNormal - Hit surface normal that will be ricocheted off.
	*/
	UFUNCTION()
	void GrazeRicochet(FVector SurfacePoint, FVector SurfaceNormal, bool RoughSurface);

	UFUNCTION()
	void CraterRicochet(FVector SurfacePoint, FVector SurfaceNormal);

	/**
	* Calculate the lenght of the penetration path for the projectile.
	*
	* @param ProjectileVector - Forward vector of the projectile after applying random rotation.
	* @param EntryLocation - Location of the projectile entry point, where it hit the surface.
	* @param Surface - Surface actor reference.
	* @param PenetrationResistance - Surface penetration resistance factor.
	* @param OutExitLocation - Location where the projectile exits the surface.
	* @param OutExitNormal - Exit location surface normal.
	*
	* @return Depth of the surface in cm, -1 when the surface is impenetrable.
	*/
	UFUNCTION()
	float CalculatePenetrationDepth(FVector ProjectileVector, FVector EntryLocation, UActorComponent* Surface, float PenetrationResistance, FVector& OutExitLocation, FVector& OutExitNormal);

	/**
	* Get penetration resistance factor from surface type.
	*
	* @param SurfaceType - The surface type.
	* @return The penetration resistance factor of the surface type
	*/
	UFUNCTION()
	FSurfaceData GetSurfaceData(EPhysicalSurface SurfaceType);

	UFUNCTION()
	void Penetrate(const FHitResult& PenetrationHit, FVector PenetrationDir, FVector ExitLoc, FVector ExitNorm, float PenetrationResistance, float PenetrationDepth);

	UFUNCTION()
	void MoveTrace(float TimeLeft);

	/*
	* 1 - MoveWithCollision
	* 2 - FullMove
	* 3 - MoveTillOutOfWall
	*/
	UFUNCTION()
	int8 FindValidTraceEnd(FHitResult& hitResult, const TArray<FHitResult>& hitResults, FVector& TraceEnd, float TimeLeft);

	UFUNCTION()
	void MoveWithCollision(FHitResult& hitResult, float TimeLeft);

	UFUNCTION()
	void FullMove(const FVector& TraceEnd, float TimeLeft);

	UFUNCTION()
	void MoveTillOutOfWall(const FVector& TraceEnd, float TimeLeft);

	UFUNCTION()
	void MoveStep();

	UFUNCTION()
	void ApplyAirDrag(float DeltaTime);

	UFUNCTION()
	void ApplyGravity(float DeltaTime);

	UFUNCTION()
	void CheckCollision(const FHitResult& HitResult);

	UFUNCTION()
	void CreateCavity(const FHitResult& HitResult);

	UFUNCTION(BlueprintImplementableEvent)
	void TryDamage(AActor* TargetActor, UActorComponent* TargetComponent, const FHitResult& DamageHitResult, APlayerController* BulletShooter, bool EndedIn = false);

	UFUNCTION(BlueprintImplementableEvent)
	void Suppress(AActor* Target, APlayerController* BulletShooter);

protected:

	virtual void BeginPlay() override;

public:

	virtual void Tick(float DeltaTime) override;

};
