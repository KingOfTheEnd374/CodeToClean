// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GunHandlingComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WARTORN_API UGunHandlingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGunHandlingComponent();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UpdateOnTick")
	bool HasValidGun;

	// In world space
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UpdateOnTick")
	FTransform CameraTransform;

	// In world space
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UpdateOnTick")
	FVector OffHandGripLocation;

	// How far from camera should sight be
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Config")
	float SightDistanceToCamera = 20.0f;

	// Used instead of SightDistanceToCamera
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Config")
	float MainHandForwardDistanceFromCamera = 5.0f;

	// In world space
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UpdateOnTick")
	FTransform SightTransform;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector MainHandOffset;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FRotator MainHandRotation;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool DisrubtedHandling;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool RecoilRecovering;

	UPROPERTY(BlueprintReadOnly)
	FTransform MainHandTransform;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MouseLookRight;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MouseLookUp;

	UPROPERTY(BlueprintReadOnly)
	FRotator SwayRotator;


	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Config")
	float MoveSpeedToDisruptHandling = 100.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Config")
	float MouseMovingMaxSway = 10.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Config")
	float MouseMovingSwaySpeed = 5.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Config")
	float HandMaxOffset = 0.2f;


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY()
	FVector RandomMainHandOffset;

	UPROPERTY()
	FRotator RandomMainHandRotation;

	UPROPERTY()
	ACharacter* OwnerCharacter;


public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
	void MainHandRandomOffset();

	UFUNCTION()
	void InterpMainHandOffset(float DeltaTime);

	UFUNCTION()
	void CalculateMainHandTransform();

	UFUNCTION()
	void MouseMovingInputs(float DeltaTime);

	UFUNCTION()
	void MainHandRandomRotation();

	UFUNCTION()
	void InterpMainHandRotation(float DeltaTime);
};
