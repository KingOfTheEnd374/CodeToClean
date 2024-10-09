#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"


UENUM()
enum EBodyPart
{
	Body_Head,
	Body_Leg,
	Body_Arm,
};

UENUM()
enum class EBodyAlteration : uint8
{
	None,
	Wound,
};

// Parent alteration, children should override their own logic
UCLASS(Abstract)
class UAlteration : public UObject
{
	GENERATED_BODY()

public:

	/*UAlteration() {}

	UAlteration(EBodyPart BodyPart, EBodyAlteration alterationType, float time)
	{
		this->Part = BodyPart;
		this->AlterationType = alterationType;
		this->Time = time;
	}*/

	UPROPERTY()
	TEnumAsByte<EBodyAlteration> AlterationType;	//Not sure if i want to use strings as identifiers

	UPROPERTY()
	TEnumAsByte<EBodyPart> Part;

	UPROPERTY()
	float Time;	//Not sure if i will use floats or ints for ticks

	UPROPERTY()
	bool UsesTick;

	virtual void InitialEffect(FBodyPart* BodyPart) {}

	virtual void TickEffect(FBodyPart* BodyPart) {}

	virtual void Initialize(TArray<float> params) {}
};

// Will get these from data table (stuff like heart, bionic heart, etc)
USTRUCT()
struct FPartBaseStats
{
	GENERATED_BODY()

	UPROPERTY()
	float Effectiveness = 1.0f;

	/*UPROPERTY()
	FCurve EffectivenessCurve*/

	UPROPERTY()
	float MaxHealth = 10.0f;
};

USTRUCT()
struct FBodyPart
{
	GENERATED_BODY()

	/*FBodyPart() {}

	FBodyPart(float maxHealth) : MaxHealth(maxHealth) {}*/

	UPROPERTY()
	FPartBaseStats BaseStats;

	UPROPERTY()
	float Effectiveness;

	/*UPROPERTY()
	FCurve EffectivenessCurve*/	// How effectiveness is affected based on precentage of part health

	UPROPERTY()
	float Health;

	UPROPERTY()
	float MaxHealth;	//Not sure if i will use floats or ints

	UPROPERTY()
	TArray<UAlteration*> Alterations;

	void AssignBaseStats(FPartBaseStats baseStats)
	{
		BaseStats = baseStats;
		Effectiveness = BaseStats.Effectiveness;
		//EffectivenessCurve = BaseStats.EffectivenessCurve;
		MaxHealth = BaseStats.MaxHealth;
		Health = MaxHealth;
	}
};

UCLASS()
class UWound : public UAlteration
{
	GENERATED_BODY()

public:

	/*UWound() {}

	UWound(EBodyPart BodyPart, EBodyAlteration alterationType, float time, float damage, float bleed) : UAlteration(BodyPart, alterationType, time)
	{
		this->Damage = damage;
		this->Bleed = bleed;
	}*/

	UPROPERTY()
	float Damage;

	UPROPERTY()
	float Bleed;

	virtual void InitialEffect(FBodyPart* BodyPart) override
	{
		BodyPart->Health -= Damage;
	}

	virtual void TickEffect(FBodyPart* BodyPart) override
	{
		Damage += Bleed;
		BodyPart->Health -= Bleed;
	}

	virtual void Initialize(TArray<float> params) override
	{
		if (params.Num() != 2)
		{
			FString Type = UEnum::GetDisplayValueAsText(AlterationType).ToString();
			UE_LOG(LogTemp, Fatal, TEXT("Fatal error! %s of type %s has incorrect amount of initialization parameters."),  *this->GetName(), *Type);
			return;
		}
		Damage = params[0];
		Bleed = params[1];
	}
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WARTORN_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();

	UPROPERTY()
	TMap<TEnumAsByte<EBodyPart>, FBodyPart> BodyParts;

	UPROPERTY()
	TArray<UAlteration*> Alterations;

	UPROPERTY()
	TArray<UAlteration*> AlterationsWithTick;

	UPROPERTY()
	float DeltaT = 0.0f;

	UFUNCTION()
	void InitBodyPart(EBodyPart Part, FPartBaseStats BaseStats);

	UFUNCTION()
	UAlteration* InitAlteration(TSubclassOf<UAlteration> Class, EBodyAlteration Alteration, EBodyPart Part, float Time, bool Ticks, TArray<float> params);

	UFUNCTION()
	void AlterationsTickEffect();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
