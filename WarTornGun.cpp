// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Weapons/WarTornGun.h"
#include "Components/SceneComponent.h"

void AWarTornGun::StartGunBuilding()
{
	// Remove existing gun building widgets and slots actors
	CleanBeforeGunUI();

	// Create slots and data for gun building UI
	CreateComponentSlots(Mesh, {});

	// Create the gun building UI
	CreateGunUI();
}

void AWarTornGun::EndGunBuilding(bool bError = false)
{
	//DataTable = nullptr;
}

void AWarTornGun::CreateComponentSlots(UStaticMeshComponent* ParentMeshComponent, const TArray<int32>& Slots)
{
	FString MeshName = ParentMeshComponent->GetStaticMesh()->GetName().RightChop(12);
	FComponentAttachments* Data = DataTable->FindRow<FComponentAttachments>(FName(*MeshName), "", false);

	if (!Data)
	{
		UE_LOG(LogTemp, Error, TEXT("Gun building error: Didn't find %s from Data Table"), *MeshName);
		return;
	}

	TArray<FName> Sockets = ParentMeshComponent->GetAllSocketNames();
	for (FName SocketName : Sockets)
	{
		// Check if socket is numeric
		if (!SocketName.ToString().IsNumeric())
			continue;

		// FName to Integer
		int32 Socket = FCString::Atoi(*SocketName.ToString());

		TArray<int32> NewSlots = Slots;
		NewSlots.Add(Socket);
		AActor* SlotActor = SpawnSlot(NewSlots, ParentMeshComponent);

		UStaticMeshComponent* ChildComponent = GetComponentAttachedToSocket(ParentMeshComponent, SocketName);

		TArray<FComponentAttachmentStats> CompatibleComponents = FindCompatibleComponents(Data->Slots[Socket].Components);
		UStaticMesh* ChildComponentMesh = ChildComponent ? ChildComponent->GetStaticMesh() : nullptr;

		AddSlotUIData(CompatibleComponents, NewSlots, SlotActor, ChildComponentMesh);

		if (ChildComponent)
		{
			CreateComponentSlots(ChildComponent, NewSlots);
		}
	}
}

UStaticMeshComponent* AWarTornGun::GetComponentAttachedToSocket(UStaticMeshComponent* ParentMeshComponent, FName Socket)
{
	TArray<USceneComponent*> ChildComponents;

	ParentMeshComponent->GetChildrenComponents(false, ChildComponents);

	for (USceneComponent* ChildComponent : ChildComponents)
	{
		UStaticMeshComponent* ChildMeshComponent = Cast<UStaticMeshComponent>(ChildComponent);
		if (!ChildMeshComponent)
			continue;

		if (ChildMeshComponent->GetAttachSocketName() == Socket)
		{
			return ChildMeshComponent;
		}
	}
	return nullptr;
}

TArray<FComponentAttachmentStats> AWarTornGun::FindCompatibleComponents(const TArray<FComponentAttachmentStats>& Components)
{
	TArray<FComponentAttachmentStats> CompatibleComponents = {};

	for (FComponentAttachmentStats Component : Components)
	{
		bool NotCompatible = false;

		TArray<FComponentIncompatibles> IncompatibleComponents;
		if (Component.StatsChanged)
		{
			IncompatibleComponents = Component.Stats.Incompatible;
		}
		else
		{
			FString MeshName = Component.Attachment->GetName().RightChop(12);
			FString ContextString;
			FComponentAttachments* Data = DataTable->FindRow<FComponentAttachments>(FName(*MeshName), ContextString);

			if (Data == nullptr)
			{
				UE_LOG(LogTemp, Error, TEXT("Gun building error: Didn't find %s from Data Table"), *MeshName);
				EndGunBuilding(true);
				return {};
			}
			IncompatibleComponents = Data->BaseStats.Incompatible;
		}

		// Loop through all incompatibilities
		for (FComponentIncompatibles IncompatibleComponent : IncompatibleComponents)
		{
			// Search if something is in obstructing slot
			if (IncompatibleComponent.Slots.Num() > 0)
			{
				USceneComponent* ParentComp = Mesh;
				TArray<USceneComponent*> ChildrenComp = GetSameActorChildrenComponents(ParentComp, false);

				for (int32 k = 0; k < IncompatibleComponent.Slots.Num(); k++)
				{
					for (int32 l = 0; l < ChildrenComp.Num(); l++)
					{
						FString Soc = ChildrenComp[l]->GetAttachSocketName().ToString();
						if (Soc.IsNumeric() && Cast<UStaticMeshComponent>(ChildrenComp[l]))
						{
							if (FCString::Atoi(*Soc) == IncompatibleComponent.Slots[k])
							{
								if (k == IncompatibleComponent.Slots.Num() - 1)
								{
									if (IncompatibleComponent.SpecificAttachment)
									{
										if (Cast<UStaticMeshComponent>(ChildrenComp[l])->GetStaticMesh() == IncompatibleComponent.SpecificAttachment)
										{
											NotCompatible = true;
											goto CheckCompatible;
										}
									}
									else
									{
										NotCompatible = true;
										goto CheckCompatible;
									}
								}
								else
								{
									ParentComp = ChildrenComp[l];
									break;
								}
							}
						}
					}
				}
			}
			// Search for a specific incompatible component anywhere on gun
			else if (IncompatibleComponent.SpecificAttachment)
			{
				// Is the root mesh incompatible
				if (Mesh->GetStaticMesh() == IncompatibleComponent.SpecificAttachment)
				{
					NotCompatible = true;
					break;
				}

				// Find the incompatible component anywhere on gun
				if (FindIncompatibleComponents(Mesh, IncompatibleComponent.SpecificAttachment))
				{
					NotCompatible = true;
					break;
				}
			}
		}
	CheckCompatible:
		if (!NotCompatible)
		{
			CompatibleComponents.Add(Component);
		}
	}
	return CompatibleComponents;
}

bool AWarTornGun::FindIncompatibleComponents(USceneComponent* ParentComp, UStaticMesh* IncompatibleComponent)
{
	// Loop through all ChildrenComponenst of ParentComp
	TArray<USceneComponent*> ChildrenComponents;
	ChildrenComponents = GetSameActorChildrenComponents(ParentComp, false);
	for (int32 i = 0; i < ChildrenComponents.Num(); i++)
	{
		FString Socket = ChildrenComponents[i]->GetAttachSocketName().ToString();
		UStaticMeshComponent* ChildComponent = Cast<UStaticMeshComponent>(ChildrenComponents[i]);
		if (Socket.IsNumeric() && ChildComponent)
		{
			if (ChildComponent->GetStaticMesh() == IncompatibleComponent)
			{
				return true;
			}
			else
			{
				if (FindIncompatibleComponents(ChildrenComponents[i], IncompatibleComponent))
				{
					return true;
				}
			}
		}
	}

	return false;
}

void AWarTornGun::AddToGunStats(const FComponentStats& Stats)
{
	if (Stats.HasTrigger) GunStats.HasTrigger = true;
	if (Stats.HasHammer) GunStats.HasHammer = true;
	if (Stats.HasFiringPin) GunStats.HasFiringPin = true;
	if (Stats.HasGasPiston) GunStats.HasGasPiston = true;
	if (Stats.HasBolt) GunStats.HasBolt = true;
	if (Stats.HasRecoilSpring) GunStats.HasRecoilSpring = true;
	if (Stats.CanCarryBolt) GunStats.CanCarryBolt = true;
	if (Stats.HasGasBlowback) GunStats.HasGasBlowback = true;
	if (Stats.ManualChamberPull) GunStats.ManualChamberPull = true;
	if (Stats.ManualChamberPush) GunStats.ManualChamberPush = true;
	if (Stats.HasFiremodeSelector) GunStats.HasFiremodeSelector = true;
	if (Stats.HoldMagazine) GunStats.HoldMagazine = true;
	if (Stats.RemoveMagazine) GunStats.RemoveMagazine = true;
	if (Stats.CanTakeAmmoFromMagazine) GunStats.CanTakeAmmoFromMagazine = true;

	/*GunStats.HasTrigger += Stats.HasTrigger;
	GunStats.HasHammer += Stats.HasHammer;
	GunStats.HasFiringPin += Stats.HasFiringPin;
	GunStats.HasGasPiston += Stats.HasGasPiston;
	GunStats.HasBolt += Stats.HasBolt;
	GunStats.HasRecoilSpring += Stats.HasRecoilSpring;
	GunStats.CanCarryBolt += Stats.CanCarryBolt;
	GunStats.HasGasBlowback += Stats.HasGasBlowback;
	GunStats.ManualChamberPull += Stats.ManualChamberPull;
	GunStats.ManualChamberPush += Stats.ManualChamberPush;
	GunStats.HasFiremodeSelector += Stats.HasFiremodeSelector;
	GunStats.HoldMagazine += Stats.HoldMagazine;
	GunStats.RemoveMagazine += Stats.RemoveMagazine;
	GunStats.CanTakeAmmoFromMagazine += Stats.CanTakeAmmoFromMagazine;*/

	GunStats.TriggerWeight += Stats.TriggerWeight;
	GunStats.TriggerReset += Stats.TriggerReset;
	GunStats.GasBlockPressureLetThrough += Stats.GasBlockPressureLetThrough;
}

void AWarTornGun::RemoveFromGunStats(const FComponentStats& Stats)
{
	if (Stats.HasTrigger) GunStats.HasTrigger = false;
	if (Stats.HasHammer) GunStats.HasHammer = false;
	if (Stats.HasFiringPin) GunStats.HasFiringPin = false;
	if (Stats.HasGasPiston) GunStats.HasGasPiston = false;
	if (Stats.HasBolt) GunStats.HasBolt = false;
	if (Stats.HasRecoilSpring) GunStats.HasRecoilSpring = false;
	if (Stats.CanCarryBolt) GunStats.CanCarryBolt = false;
	if (Stats.HasGasBlowback) GunStats.HasGasBlowback = false;
	if (Stats.ManualChamberPull) GunStats.ManualChamberPull = false;
	if (Stats.ManualChamberPush) GunStats.ManualChamberPush = false;
	if (Stats.HasFiremodeSelector) GunStats.HasFiremodeSelector = false;
	if (Stats.HoldMagazine) GunStats.HoldMagazine = false;
	if (Stats.RemoveMagazine) GunStats.RemoveMagazine = false;
	if (Stats.CanTakeAmmoFromMagazine) GunStats.CanTakeAmmoFromMagazine = false;

	GunStats.TriggerWeight -= Stats.TriggerWeight;
	GunStats.TriggerReset -= Stats.TriggerReset;
	GunStats.GasBlockPressureLetThrough -= Stats.GasBlockPressureLetThrough;
}

void AWarTornGun::AddAttachment(const FComponentAttachmentStats& Component, const TArray<int32>& Slots)
{
	USceneComponent* ParentComponent = Mesh;
	for (int32 i = 0; i < Slots.Num(); i++)
	{
		if (i == Slots.Num() - 1)
		{
			// Create new attachment and attach it
			FString CompName = Component.Attachment->GetName() + FString::FromInt(MeshNum);
			USceneComponent* NewComponent = NewObject<UStaticMeshComponent>(this, FName(*CompName));

			NewComponent->RegisterComponent();

			FAttachmentTransformRules AttachRules = FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true);

			FString SlotIndexString = FString::FromInt(Slots[i]);
			NewComponent->AttachToComponent(ParentComponent, AttachRules, FName(*SlotIndexString));

			Cast<UStaticMeshComponent>(NewComponent)->SetStaticMesh(Component.Attachment);
			Cast<UStaticMeshComponent>(NewComponent)->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			Cast<UStaticMeshComponent>(NewComponent)->SetCollisionProfileName("PhysicsActor");
			Cast<UStaticMeshComponent>(NewComponent)->SetGenerateOverlapEvents(false);
			Cast<UStaticMeshComponent>(NewComponent)->SetCollisionResponseToAllChannels(ECR_Ignore);	//CHANGE THIS TO WHAT SUITS THE BEST

			MeshNum++;

			FAttachmentStats NewStats;

			NewStats.Attachment = Component.Attachment;
			NewStats.Slots = Slots;

			if (Component.StatsChanged)
			{
				NewStats.Stats = Component.Stats;
			}
			else
			{
				FString MeshName = Component.Attachment->GetName().RightChop(12);
				FString ContextString;
				FComponentAttachments* Data = DataTable->FindRow<FComponentAttachments>(FName(*MeshName), ContextString);

				if (Data == nullptr)
				{
					UE_LOG(LogTemp, Error, TEXT("Gun building error: Didn't find %s from Data Table"), *MeshName);
					EndGunBuilding(true);
					return;
				}

				NewStats.Stats = Data->BaseStats;
			}

			// FOR TESTING ONLY, MOVE TO ADD TO GUN STATS
			if (NewComponent->DoesSocketExist("Grip"))
			{
				GripLocation.Add(NewComponent->GetSocketTransform("Grip", ERelativeTransformSpace::RTS_Actor).GetLocation());//NewComponent->GetSocketLocation("Grip") - GetActorLocation());
			}

			if (NewComponent->DoesSocketExist("Sight"))
			{
				SightLocation = NewComponent->GetSocketTransform("Sight", ERelativeTransformSpace::RTS_Actor);//NewComponent->GetSocketLocation("Sight") - GetActorLocation();
			}

			//MODIFY GUN STATS
			AddToGunStats(NewStats.Stats);
			Attachments.Add(NewStats);

			/* Not really performance good, probably needs rework. */
			// Reattach the attachment to another point (only if needed).
			TArray<UStaticMesh*> Reattachments = NewStats.Stats.AttachmentsToReattach;
			if (Reattachments.Num() > 0)
			{
				if (!Reattachments.Contains(Cast<UStaticMeshComponent>(ParentComponent)->GetStaticMesh()))
				{
					for (int32 k = 0; k < Reattachments.Num(); k++)
					{
						for (int32 m = 0; m < Attachments.Num(); m++)
						{
							if (Reattachments[k] == Attachments[m].Attachment)
							{
								RemoveAttachment(Slots);

								TArray<int32> SlotToReattach = Attachments[m].Slots;
								SlotToReattach.Add(NewStats.Stats.ReattachSlot);
								FComponentAttachmentStats ReattachStats;
								ReattachStats.Attachment = NewStats.Attachment;
								ReattachStats.StatsChanged = false;

								FString MeshName = Cast<UStaticMeshComponent>(Attachments[m].Attachment)->GetStaticMesh()->GetName().RightChop(12);
								FString ContextString;
								FComponentAttachments* Data = DataTable->FindRow<FComponentAttachments>(FName(*MeshName), ContextString);

								if (Data == nullptr)
								{
									UE_LOG(LogTemp, Error, TEXT("Gun building error: Didn't find %s from Data Table"), *MeshName);
									EndGunBuilding(true);
									return;
								}

								int32 ReattachSlot = NewStats.Stats.ReattachSlot;
								for (int32 n = 0; n < Data->Slots[ReattachSlot].Components.Num(); n++)
								{
									if (NewStats.Attachment == Data->Slots[ReattachSlot].Components[n].Attachment)
									{
										if (Data->Slots[ReattachSlot].Components[n].StatsChanged)
										{
											ReattachStats.StatsChanged = true;
											ReattachStats.Stats = Data->Slots[ReattachSlot].Components[n].Stats;
											break;
										}
									}
								}
								AddAttachment(ReattachStats, SlotToReattach);
							}
						}
					}
				}
			}
		}
		else
		{
			TArray<USceneComponent*> ChildrenComponents;
			ChildrenComponents = GetSameActorChildrenComponents(ParentComponent, false);
			for (int32 j = 0; j < ChildrenComponents.Num(); j++)
			{
				FString Socket = ChildrenComponents[j]->GetAttachSocketName().ToString();
				if (Socket.IsNumeric() && Cast<UStaticMeshComponent>(ChildrenComponents[j]))
				{
					if (FCString::Atoi(*Socket) == Slots[i])
					{
						ParentComponent = ChildrenComponents[j];
						break;
					}
				}
			}
		}
	}
}

void AWarTornGun::RemoveAttachment(const TArray<int32>& Slots)
{
	USceneComponent* ParentComponent = Mesh;
	for (int32 i = 0; i < Slots.Num(); i++)
	{
		TArray<USceneComponent*> ChildrenComponents;
		ChildrenComponents = GetSameActorChildrenComponents(ParentComponent, false);

		for (int32 j = 0; j < ChildrenComponents.Num(); j++)
		{
			FString Socket = ChildrenComponents[j]->GetAttachSocketName().ToString();
			if (Socket.IsNumeric() && Cast<UStaticMeshComponent>(ChildrenComponents[j]))
			{
				if (FCString::Atoi(*Socket) == Slots[i])
				{
					if (i == Slots.Num() - 1)
					{
						DestroyAttachment(ChildrenComponents[j], Slots);
					}
					else
					{
						ParentComponent = ChildrenComponents[j];
						break;
					}
				}
			}
		}
	}
}

void AWarTornGun::RemoveAllAttachments()
{
	TArray<USceneComponent*> ChildrenComponents;
	ChildrenComponents = GetSameActorChildrenComponents(Mesh, false);

	for (int32 i = 0; i < ChildrenComponents.Num(); i++)
	{
		FString Socket = ChildrenComponents[i]->GetAttachSocketName().ToString();
		if (Socket.IsNumeric() && Cast<UStaticMeshComponent>(ChildrenComponents[i]))
		{
			TArray<int32> Slots = { FCString::Atoi(*Socket) };
			DestroyAttachment(ChildrenComponents[i], Slots);
		}
	}
}

void AWarTornGun::DestroyAttachment(USceneComponent* ParentAttachment, const TArray<int32>& Slots)
{
	// Find all children attachments and destroy them
	TArray<USceneComponent*> ChildrenComponents;
	ChildrenComponents = GetSameActorChildrenComponents(ParentAttachment, false);
	for (int32 i = 0; i < ChildrenComponents.Num(); i++)
	{
		FString Socket = ChildrenComponents[i]->GetAttachSocketName().ToString();
		if (Socket.IsNumeric() && Cast<UStaticMeshComponent>(ChildrenComponents[i]))
		{			
			TArray<int32> NewSlots = Slots;
			NewSlots.Add(FCString::Atoi(*Socket));
			DestroyAttachment(ChildrenComponents[i], NewSlots);
			
		}
	}

	// Find attachment stats and update weapon stats
	for (int32 i = 0; i < Attachments.Num(); i++)
	{
		if (Slots == Attachments[i].Slots)
		{
			//Attachments[i].Stats;
			//MODIFY WEAPON STATS
			RemoveFromGunStats(Attachments[i].Stats);
			Attachments.RemoveAt(i);
			break;
		}
	}

	ParentAttachment->DestroyComponent();
}

TArray<USceneComponent*> AWarTornGun::GetSameActorChildrenComponents(USceneComponent* ParentComp, bool AllDescendants)
{
	TArray<USceneComponent*> SameActorChildren;
	TArray<USceneComponent*> AllChildren;

	ParentComp->GetChildrenComponents(AllDescendants, AllChildren);
	
	for (int i = 0; i < AllChildren.Num(); i++)
	{
		// Not sure why this check is here, maybe for magazines?
		if (AllChildren[i]->GetOwner() == this)
		{
			SameActorChildren.Add(AllChildren[i]);
		}
	}
	return SameActorChildren;
}

void AWarTornGun::PrimaryUse(AWarTornCharacter* User)
{
	Super::PrimaryUse(User);

	if (GunStats.HasTrigger)
	{
		if (GunStats.HasHammer)
		{
			if (GunStats.HasFiringPin)
			{
				/*Fire bullet in chamber*/
			}
		}
	}
}

void AWarTornGun::LoadBuild(const TArray<FAttachmentStats>& AttachmentsList)
{
	GripLocation.Empty();
	SightLocation = FTransform();
	RemoveAllAttachments();

	for (int32 i = 0; i < AttachmentsList.Num(); i++)
	{
		FComponentAttachmentStats Stats;
		Stats.Attachment = AttachmentsList[i].Attachment;
		Stats.StatsChanged = true;
		Stats.Stats = AttachmentsList[i].Stats;

		AddAttachment(Stats, AttachmentsList[i].Slots);
	}
}