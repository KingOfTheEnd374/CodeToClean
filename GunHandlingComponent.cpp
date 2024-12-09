// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/GunHandlingComponent.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"

// Sets default values for this component's properties
UGunHandlingComponent::UGunHandlingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UGunHandlingComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UGunHandlingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}

	if (!HasValidGun)
	{
		return;
	}

	DisrubtedHandling = RecoilRecovering || GetOwner()->GetVelocity().Size() > MoveSpeedToDisruptHandling;

	MainHandRandomOffset();

	InterpMainHandOffset(DeltaTime);
	
	CalculateMainHandTransform();

	if (OwnerCharacter->IsLocallyControlled())
	{
		MouseMovingInputs(DeltaTime);
	}

	MainHandRandomRotation();
	
	InterpMainHandRotation(DeltaTime);
}


void UGunHandlingComponent::MainHandRandomOffset()
{
	if (MainHandOffset.Equals(RandomMainHandOffset))
	{
		float OffsetY, OffsetZ;

		OffsetY = FMath::RandRange(-HandMaxOffset / 5.0f, HandMaxOffset / 5.0f);
		OffsetZ = FMath::RandRange(-HandMaxOffset, HandMaxOffset);
		if (DisrubtedHandling)
		{
			OffsetY = 2.0f * OffsetY;
			OffsetZ = 2.0f * OffsetZ;
		}

		RandomMainHandOffset = FVector(0.0f, OffsetY, OffsetZ);
	}
}


void UGunHandlingComponent::InterpMainHandOffset(float DeltaTime)
{
	float MainHandInterpSpeed;

	float BaseSpeed = DisrubtedHandling ? 0.5f : 0.1f;

	float TargetOffsetDist = FVector::Dist(MainHandOffset, RandomMainHandOffset);
	if (TargetOffsetDist < 0.015f)
	{
		MainHandInterpSpeed = FMath::Lerp(0.01f, BaseSpeed, TargetOffsetDist / 0.015f);
	}

	MainHandOffset = FMath::VInterpConstantTo(MainHandOffset, RandomMainHandOffset, DeltaTime, MainHandInterpSpeed);
}


void UGunHandlingComponent::CalculateMainHandTransform()
{
	FTransform HandTransform = OwnerCharacter->GetMesh()->GetSocketTransform("Arm03_R");
	FTransform MainHandRelativeToSight = HandTransform.GetRelativeTransform(SightTransform);

	//DrawDebugSphere(GetWorld(), SightTransform.GetLocation(), 5.0f, 4, FColor::Red, false, 0.05f);
	//DrawDebugDirectionalArrow(GetWorld(), SightTransform.GetLocation(), OwnerCharacter->GetMesh()->GetSocketTransform("Arm03_R").GetLocation(), 5.0f, FColor::Red, false, 0.05f);

	FVector TempLoc = MainHandRelativeToSight.GetLocation();
	TempLoc.X = MainHandForwardDistanceFromCamera;

	MainHandRelativeToSight.SetLocation(TempLoc);

	// Combine Camera transform with Sight to Hand offset.
	FTransform MainHandWorldTransform = MainHandRelativeToSight * CameraTransform;//OwnerCharacter->GetMesh()->GetSocketTransform("FPCamera");

	//DrawDebugSphere(GetWorld(), /*OwnerCharacter->GetMesh()->GetSocketTransform("FPCamera")*/CameraTransform.GetLocation(), 5.0f, 4, FColor::Green, false, 0.05f);

	//DrawDebugSphere(GetWorld(), MainHandWorldTransform.GetLocation(), 5.0f, 4, FColor::Blue, false, 0.05f);

	MainHandTransform = MainHandWorldTransform.GetRelativeTransform(OwnerCharacter->GetMesh()->GetSocketTransform("Head"));
}


void UGunHandlingComponent::MouseMovingInputs(float DeltaTime)
{
	float LookRightTarget = FMath::Clamp(OwnerCharacter->InputComponent->GetAxisValue("LookRight"), -1.0f, 1.0f);
	float LookUpTarget = FMath::Clamp(OwnerCharacter->InputComponent->GetAxisValue("LookUp"), -1.0f, 1.0f);

	FVector2D MouseMovementNormalized = FVector2D(LookRightTarget, LookUpTarget).GetSafeNormal();

	// Calculate speed better and use FInterpToConstant
	MouseLookRight = FMath::FInterpTo(MouseLookRight, MouseMovementNormalized.X, DeltaTime, MouseMovingSwaySpeed);
	MouseLookUp = FMath::FInterpTo(MouseLookUp, MouseMovementNormalized.Y, DeltaTime, MouseMovingSwaySpeed);
}


void UGunHandlingComponent::MainHandRandomRotation()
{
	// Checks if any rotation axis has been met, if so, find new random value.
	if (FMath::IsNearlyEqual(MainHandRotation.Roll, RandomMainHandRotation.Roll, 0.01f))
	{
		RandomMainHandRotation.Roll = DisrubtedHandling ?
			FMath::RandRange(-0.2f, 0.2f) :
			FMath::RandRange(-0.1f, 0.1f);
	}

	if (FMath::IsNearlyEqual(MainHandRotation.Pitch, RandomMainHandRotation.Pitch, 0.01f))
	{
		RandomMainHandRotation.Pitch = DisrubtedHandling ?
			FMath::RandRange(-0.2f, 0.2f) :
			FMath::RandRange(-0.1f, 0.1f);
	}

	if (FMath::IsNearlyEqual(MainHandRotation.Yaw, RandomMainHandRotation.Yaw, 0.01f))
	{
		RandomMainHandRotation.Yaw = DisrubtedHandling ?
			FMath::RandRange(-0.2f, 0.2f) :
			FMath::RandRange(-0.1f, 0.1f);
	}
}


void UGunHandlingComponent::InterpMainHandRotation(float DeltaTime)
{
	float InterpSpeed = DisrubtedHandling ? 1.0f : 0.3f;

	MainHandRotation = FRotator(
		FMath::FInterpConstantTo(MainHandRotation.Pitch, RandomMainHandRotation.Pitch, DeltaTime, InterpSpeed),
		FMath::FInterpConstantTo(MainHandRotation.Yaw, RandomMainHandRotation.Yaw, DeltaTime, InterpSpeed),
		FMath::FInterpConstantTo(MainHandRotation.Roll, RandomMainHandRotation.Roll, DeltaTime, InterpSpeed)
	);

	// No idea why these are pitch and roll, not pitch and yaw. Probably error comes from animation blueprint/rig.
	float MouseMovingSwayRotPitch = FMath::Lerp(0.0f, MouseMovingMaxSway, MouseLookRight);
	float MouseMovingSwayRotRoll = FMath::Lerp(0.0f, MouseMovingMaxSway, MouseLookUp);

	// THIS IS BULLSHIT, NEEDS REDOING
	float AimCorrectionPitch = atanf(MainHandOffset.Y / MainHandForwardDistanceFromCamera);
	float AimCorrectionRoll = -atanf(MainHandOffset.Z / MainHandForwardDistanceFromCamera);

	SwayRotator = FRotator(
		MainHandRotation.Pitch + MouseMovingSwayRotPitch + AimCorrectionPitch,
		MainHandRotation.Yaw,
		MainHandRotation.Roll + MouseMovingSwayRotRoll + AimCorrectionRoll
	);
}