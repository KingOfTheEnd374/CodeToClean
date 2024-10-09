// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/WarTornItem.h"
#include "Engine/DataTable.h"
#include "WarTornGun.generated.h"


USTRUCT(BlueprintType)
struct FComponentIncompatibles
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<int32> Slots;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UStaticMesh* SpecificAttachment;
};

USTRUCT(BlueprintType)
struct FComponentStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool HasTrigger = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float TriggerWeight = 0.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float TriggerReset = 0.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool HasHammer = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool HasFiringPin = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool HasGasPiston = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool HasBolt = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool HasRecoilSpring = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool CanCarryBolt = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool HasGasBlowback = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float GasBlockPressureLetThrough = 0.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool ManualChamberPull = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool ManualChamberPush = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool HasChamber = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool HasFiremodeSelector = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool HoldMagazine = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool RemoveMagazine = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool CanTakeAmmoFromMagazine = false;



	/*
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool FoldStock = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool Disconnector = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool OutOfBatterySafety = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float FireRate = 0.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float VerticalRecoil = 0.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float HorizontalRecoil = 0.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float RecoilSpeed = 0.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float VerticalSway = 0.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float HorizontalSway = 0.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float SwaySpeed = 0.0f;
	*/

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FComponentIncompatibles> Incompatible;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<UStaticMesh*> AttachmentsToReattach;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 ReattachSlot;
};

USTRUCT(BlueprintType)
struct FAttachmentStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UStaticMesh* Attachment;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FComponentStats Stats;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<int32> Slots;
};

USTRUCT(BlueprintType)
struct FComponentAttachmentStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UStaticMesh* Attachment;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool StatsChanged;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FComponentStats Stats;
};

USTRUCT(BlueprintType)
struct FComponent2DArray
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FComponentAttachmentStats> Components;
};

USTRUCT(BlueprintType)
struct FComponentAttachments : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UStaticMesh* Component;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FComponentStats BaseStats;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FComponent2DArray> Slots;
};

UCLASS()
class WARTORN_API AWarTornGun : public AWarTornItem
{
	GENERATED_BODY()
	

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FAttachmentStats> Attachments;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FComponentStats GunStats;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UDataTable* DataTable;

	UPROPERTY()
	int32 MeshNum = 0;

	UPROPERTY(BlueprintReadWrite)
	TArray<FVector> GripLocation;

	UPROPERTY(BlueprintReadWrite)
	FTransform SightLocation = FTransform();
	
	UFUNCTION(BlueprintCallable)
	void StartGunBuilding();

	UFUNCTION(BlueprintCallable)
	void EndGunBuilding(bool bError);

	/**
	* Spawns slot actor in the slot location.
	*
	* @param Slot - Slots for the slot actor.
	* @param Comp - Parent component of the slot actor.
	*
	* @return Slot actor.
	*/
	UFUNCTION(BlueprintImplementableEvent)
	AActor* SpawnSlot(const TArray<int32>& Slot, USceneComponent* Comp);

	/** TEMP FUNCTION, WILL DO IN C++ LATER */
	UFUNCTION(BlueprintImplementableEvent)
	void AddSlotUIData(const TArray<FComponentAttachmentStats>& Components, const TArray<int32>& Slots, AActor* Socket, UStaticMesh* Comp);

	UFUNCTION(BlueprintCallable)
	void AddToGunStats(const FComponentStats& Stats);

	UFUNCTION(BlueprintCallable)
	void RemoveFromGunStats(const FComponentStats& Stats);
	
	/**
	* Attach an attachment to the gun in the specified slot.
	*
	* @param Component - Stats for to be added attachment.
	* @param Slots - Slots where the attachment will be added.
	*/
	UFUNCTION(BlueprintCallable)
	void AddAttachment(const FComponentAttachmentStats& Component, const TArray<int32>& Slots);

	/**
	* Find attachment in Slots and pass it on to be destroyed.
	*
	* @param Slots - Attachment to be removed slots.
	*/
	UFUNCTION(BlueprintCallable)
	void RemoveAttachment(const TArray<int32>& Slots);

	/**
	* Remove all attachments and leave only the base component.
	*/
	UFUNCTION(BlueprintCallable)
	void RemoveAllAttachments();

	UFUNCTION(BlueprintImplementableEvent)
	void CleanBeforeGunUI();

	UFUNCTION(BlueprintImplementableEvent)
	void Print(const FString& Text);

	virtual void PrimaryUse(AWarTornCharacter* User);

protected:


	/**
	* Create attachment slots data for all gun components. It is recursive and works through all children. This data is compatible components, current slot and mesh that is currently attached to it.
	* 
	* @param ParentMeshComponent - Current iteration parent component
	* @param Slots - Current iteration slots tree
	*/
	UFUNCTION()
	void CreateComponentSlots(UStaticMeshComponent* ParentMeshComponent, const TArray<int32>& Slots);

	// Gets a StaticMeshComponent that is attached to the ParentMeshComponent at Socket. Returns nullptr if none found.
	UFUNCTION()
	UStaticMeshComponent* GetComponentAttachedToSocket(UStaticMeshComponent* ParentMeshComponent, FName Socket);

	/**
	* Destroys the attachment and its children. Updates weapon stats.
	*
	* @param ParentAttachment - Attachment to be destroyed.
	* @param Slots - Attachment to be destroyed slots.
	*/
	UFUNCTION()
	void DestroyAttachment(USceneComponent* ParentAttachment, const TArray<int32>& Slots);

	UFUNCTION()
	TArray<FComponentAttachmentStats> FindCompatibleComponents(const TArray<FComponentAttachmentStats>& Components);

	UFUNCTION()
	bool FindIncompatibleComponents(USceneComponent* ParentComp, UStaticMesh* IncompatibleComponent);

	/** Spawn gun building UI widgets*/
	UFUNCTION(BlueprintImplementableEvent)
	void CreateGunUI();

	/**
	* Gives an array of ParentComponent children components that are the same actor.
	*
	* @param ParentComponent - Parent component for the children components.
	* @param AllDescendants - Should it get all descendants of the children.
	*
	* @return Only children components of that actor.
	*/
	UFUNCTION()
	TArray<USceneComponent*> GetSameActorChildrenComponents(USceneComponent* ParentComp, bool AllDescendants);

	UFUNCTION(BlueprintCallable)
	void LoadBuild(const TArray<FAttachmentStats>& AttachmentsList);
};