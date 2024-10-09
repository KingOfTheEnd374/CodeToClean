#include "Components/Health/HealthComponent.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	FPartBaseStats BasePart;

	InitBodyPart(Body_Head, BasePart);

	InitAlteration(UWound::StaticClass(), EBodyAlteration::Wound, Body_Head, 0.0f, true, { 2.0f, 1.0f });
}


// Called every frame
void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	DeltaT += DeltaTime;

	if (DeltaT >= 1.0f)
	{
		AlterationsTickEffect();
		DeltaT -= 1.0f;
	}
}


void UHealthComponent::InitBodyPart(EBodyPart Part, FPartBaseStats BaseStats)
{
	FBodyPart NewPart;
	NewPart.AssignBaseStats(BaseStats);
	BodyParts.Add(Part, NewPart);
}

UAlteration* UHealthComponent::InitAlteration(TSubclassOf<UAlteration> Class, EBodyAlteration Alteration, EBodyPart Part, float Time, bool Ticks, TArray<float> params)
{
	UAlteration* NewAlteration = NewObject<UAlteration>(GetTransientPackage(), Class);
	NewAlteration->AlterationType = Alteration;
	NewAlteration->Part = Part;
	NewAlteration->Time = Time;
	NewAlteration->UsesTick = Ticks;
	NewAlteration->Initialize(params);

	FBodyPart* BodyPart = BodyParts.Find(Body_Head);
	BodyPart->Alterations.Add(NewAlteration);
	Alterations.Add(NewAlteration);
	if (Ticks)
		AlterationsWithTick.Add(NewAlteration);

	NewAlteration->InitialEffect(BodyPart);

	return NewAlteration;
}


void UHealthComponent::AlterationsTickEffect()
{
	for (UAlteration* Alteration : AlterationsWithTick)
	{
		Alteration->TickEffect(BodyParts.Find(Alteration->Part));
	}
}