// Fill out your copyright notice in the Description page of Project Settings.

#include "PawnStatusComp.h"

#include "Epidemic/Actors/Pawns/EpiCharacter.h"


// Sets default values for this component's properties
UPawnStatusComp::UPawnStatusComp(const FObjectInitializer& ObjectInitializer)
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = true;

    SetIsReplicatedByDefault(true);
    StatusType.AddDefaulted(LastEntry);

    //Health
    StatusType[Health].Status = Health;
    StatusType[Health].StatusBase.Base = 100.0f;
    StatusType[Health].StatusBase.Max = 100.0f;
    StatusType[Health].StatusBase.Min = 0.0f;
    
    StatusType[Health].StatusDelta.Base = 0.0f;
    StatusType[Health].StatusDelta.Max = 10.0f;
    StatusType[Health].StatusDelta.Min = 0.0f;

    //Blood
    StatusType[Blood].Status = Blood;
    StatusType[Blood].StatusBase.Base = 5000.0f;
    StatusType[Blood].StatusBase.Max = 5000.0f;
    StatusType[Blood].StatusBase.Min = 0.0f;
    
    StatusType[Blood].StatusDelta.Base = 0.0f;
    StatusType[Blood].StatusDelta.Max = 100.0f;
    StatusType[Blood].StatusDelta.Min = -100.0f;

    //Water
    StatusType[Water].Status = Water;
    StatusType[Water].StatusBase.Base = 3700.0f;
    StatusType[Water].StatusBase.Max = 3700.0f;
    StatusType[Water].StatusBase.Min = 0.0f;
    
    StatusType[Water].StatusDelta.Base = 0.0f;
    StatusType[Water].StatusDelta.Max = 100.0f;
    StatusType[Water].StatusDelta.Min = -100.0f;

    //Food
    StatusType[Food].Status = Food;
    StatusType[Food].StatusBase.Base = 2700.0f;
    StatusType[Food].StatusBase.Max = 2700.0f;
    StatusType[Food].StatusBase.Min = 0.0f;
    
    StatusType[Food].StatusDelta.Base = 0.0f;
    StatusType[Food].StatusDelta.Max = 100.0f;
    StatusType[Food].StatusDelta.Min = -100.0f;

    //Heat
    StatusType[Heat].Status = Heat;
    StatusType[Heat].StatusBase.Base = 20.0f;
    StatusType[Heat].StatusBase.Max = 50.0f;
    StatusType[Heat].StatusBase.Min = -50.0f;
    
    StatusType[Heat].StatusDelta.Base = 0.0f;
    StatusType[Heat].StatusDelta.Max = 10.0f;
    StatusType[Heat].StatusDelta.Min = -10.0f;

    //Stamina
    StatusType[Stamina].Status = Stamina;
    StatusType[Stamina].StatusBase.Base = 100.0f;
    StatusType[Stamina].StatusBase.Max = 100.0f;
    StatusType[Stamina].StatusBase.Min = 0.0f;
    
    StatusType[Stamina].StatusDelta.Base = 0.0f;
    StatusType[Stamina].StatusDelta.Max = 1.0f;
    StatusType[Stamina].StatusDelta.Min = -1.0f;

    BiomeHeat = -10.0f;
    // ...
}


// Called when the game starts
void UPawnStatusComp::BeginPlay()
{
    Super::BeginPlay();

    // ...
}

// Called every frame
void UPawnStatusComp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    for (auto& Effect : ActiveEffects)
    {
        FEffectValues& Values = Effect.Value;

        const float DeltaValue = (Values.Amount / Values.Duration) * DeltaTime;

        AddStatusSaveNoDelta(DeltaValue, Effect.Key, StatusBase);
        
        Values.Duration -= DeltaTime;
        Values.Amount -= DeltaValue;
        //UE_LOG(LogTemp, Error, TEXT("Duration %f Amount %f DeltaValue %f"), Values.Duration, Values.Amount, DeltaValue);
        
        if (abs(Values.Amount) <= 0 || Values.Duration <= 0)
        {
            ActiveEffects.Remove(Effect.Key);
            break;
        }
    }
    
    for (auto& Status : StatusType)
    {
        AddStatusSave(GetStatusBase(Status.Status, StatusDelta), Status.Status, StatusBase);
    }
    SetLosingLife();
    HandleHeat();
}

float& UPawnStatusComp::GetStatus(EStatus Name, EStatusMeta Meta, EStatusInfo Info)
{
    switch (Meta)
    {
    case StatusBase:
        {
            FStatusBase& Type = StatusType[Name].StatusBase;
            switch (Info)
            {
            case Base:
                return Type.Base;
            case Min:
                return Type.Min;
            case Max:
                return Type.Max;
            }
        }
        break;
    case StatusDelta:
        {
            FStatusBase& Type = StatusType[Name].StatusDelta;
            switch (Info)
            {
            case Base:
                return Type.Base;
            case Min:
                return Type.Min;
            case Max:
                return Type.Max;
            }
        }
        break;
    }
    return StatusNone;
}

float& UPawnStatusComp::GetStatusPure(EStatus Name, EStatusMeta Meta, EStatusInfo Info)
{
    return GetStatus(Name, Meta, Info);
}

/** set a status (not increasing or decrease current value)  */
float& UPawnStatusComp::SetStatus(float Value, EStatus Name, EStatusMeta Meta, EStatusInfo Info)
{
    float& ChangedStatus = GetStatus(Name, Meta, Info) = Value;
    OnStatusChanged.Broadcast(Name);
    CheckStatus();
    return ChangedStatus;
}

/** set a status (not increasing or decrease current value)  */
float& UPawnStatusComp::SetStatusSave(float Value, EStatus Name, EStatusMeta Meta, EStatusInfo Info)
{
    const float Max = GetStatusMax(Name, Meta);
    const float Min = GetStatusMin(Name, Meta);
    return SetStatus(FMath::Clamp(Value, Min, Max), Name, Meta, Info);
}

/** increase or decrease status  */
float& UPawnStatusComp::AddStatus(float Value, EStatus Name, EStatusMeta Meta, EStatusInfo Info)
{
    return GetStatus(Name, Meta, Info) += Value;
}

/** add or reduce a status clamped to it's min/max  */
float& UPawnStatusComp::AddStatusSave(float Value, EStatus Name, EStatusMeta Meta)
{
    const float Base = GetStatusBase(Name, Meta);
    const float Max = GetStatusMax(Name, Meta);
    const float Min = GetStatusMin(Name, Meta);
    const float Result = Base + Value * GetWorld()->GetDeltaSeconds();

    return SetStatus(FMath::Clamp(Result, Min, Max), Name, Meta, EStatusInfo::Base);
}

/** add or reduce a status clamped to it's min/max  */
float& UPawnStatusComp::AddStatusSaveNoDelta(float Value, EStatus Name, EStatusMeta Meta)
{
    const float Base = GetStatusBase(Name, Meta);
    const float Max = GetStatusMax(Name, Meta);
    const float Min = GetStatusMin(Name, Meta);
    const float Result = Base + Value;
    return SetStatus(FMath::Clamp(Result, Min, Max), Name, Meta, EStatusInfo::Base);
}

float& UPawnStatusComp::GetStatusBase(const EStatus Name, EStatusMeta Meta)
{
    return GetStatus(Name, Meta, Base);
}

float& UPawnStatusComp::GetStatusMin(const EStatus Name, EStatusMeta Meta)
{
    return GetStatus(Name, Meta, Min);
}

float& UPawnStatusComp::GetStatusMax(const EStatus Name, EStatusMeta Meta)
{
    return GetStatus(Name, Meta, Max);
}

float UPawnStatusComp::GetStatusPercent(const EStatus Name, EStatusMeta Meta)
{
    float Base = GetStatusBase(Name, Meta);
    float Max = GetStatusMax(Name, Meta);
    const float Min = abs(GetStatusMin(Name, Meta));
    Max += Min;
    Base += Min;
    const float Percent = Base / Max;
    return Percent;
}

void UPawnStatusComp::AddEffect(UPARAM(ref)FStatusEffect& EffectToAdd)
{
    if (ActiveEffects.Contains(EffectToAdd.StatusToEffect))
    {
        ActiveEffects.Find(EffectToAdd.StatusToEffect)->Amount += EffectToAdd.Amount;
        ActiveEffects.Find(EffectToAdd.StatusToEffect)->Duration += EffectToAdd.Duration;
    }
    else
    {
        ActiveEffects.Add(EffectToAdd.StatusToEffect, FEffectValues(EffectToAdd.Duration, EffectToAdd.Amount * 1.0f));
    }
}

void UPawnStatusComp::HandleHeat()
{
    if (UseHeat)
    {
        const float CurHeat = GetStatus(Heat, StatusBase, Base);

        const float HeatDif = BiomeHeat + IsSprinting * 15.0f - CurHeat;

        float HeatIncreaseDir;

        if (HeatDif >= 0.0f)
        {
            HeatIncreaseDir = 1.0f;
        }
        else
        {
            HeatIncreaseDir = -1.0f;
        }

        float HeatAlpha;

        if (HeatDif == 0.0f)
        {
            HeatAlpha = 0.0f;
        }
        else
        {
            HeatAlpha = abs(HeatDif) / 50.0f;
        }

        const float HeatIncrease = FMath::Lerp(0.0f, 5.0f, HeatAlpha);

        AddStatusSave(HeatIncrease * FMath::Clamp(HeatIncreaseDir, -1.0f, 1.0f), Heat, StatusBase);

        const float HotOverFlow = FMath::Clamp(CurHeat - 30.0f, 0.0f, 50.0f);
        HotAlpha = HotOverFlow / 30.0f;
        
        const float ColdOverFlow = FMath::Clamp(CurHeat + 10.0f, -50.0f, 0.0f);
        ColdAlpha = abs(ColdOverFlow) / 30.0f;

        //Food Values
        const float MaxFoodDelta = GetStatusMax(Food, StatusDelta);
        const float MinFoodDelta = GetStatusMin(Food, StatusDelta);
        const float FoodDeltaLerp = FMath::Lerp(MaxFoodDelta, MinFoodDelta, ColdAlpha);
        SetStatusSave(FoodDeltaLerp, Food, StatusDelta, Base);

        const float MaxWaterDelta = GetStatusMax(Water, StatusDelta);
        const float MinWaterDelta = GetStatusMin(Water, StatusDelta);
        const float WaterDeltaLerp = FMath::Lerp(MaxWaterDelta, MinWaterDelta, HotAlpha);
        SetStatusSave(WaterDeltaLerp, Water, StatusDelta, Base);

        //UE_LOG(LogTemp, Error, TEXT("FoodDeltaLerp %f WaterDeltaLerp %f"), FoodDeltaLerp, WaterDeltaLerp);
        //UE_LOG(LogTemp, Error, TEXT("Increase %f Dif %f Heat %f Alpha %f"), HeatIncrease, HeatDif, GetStatus(Heat, StatusBase, Base), HeatAlpha);
        //UE_LOG(LogTemp, Error, TEXT("OverFlow %f HotAlpha %f Heat %f"), HotOverFlow, HotAlpha, CurHeat);
        //UE_LOG(LogTemp, Error, TEXT("OverFlow %f ColdAlpha %f Heat %f"), ColdOverFlow, ColdAlpha, CurHeat);
        //UE_LOG(LogTemp, Error, TEXT("FoodUse %f WaterUse %f"), Status.FoodUsage, Status.WaterUsage);

        Suffocate = (Heat >= GetStatus(Heat, StatusBase, Max) || Heat <= GetStatus(Heat, StatusBase, Min));
    }
}

bool UPawnStatusComp::InShade()
{
    if (!SunLight)
    {
        TArray<AActor*> FoundActors;
        //UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            SunLight = FoundActors[0];
        }
        return false;
    }

    if (!SunHitParams.GetIgnoredActors().IsValidIndex(0))
    {
        SunHitParams.AddIgnoredActor(GetOwner());
    }

    const FVector Start = GetOwner()->GetActorLocation();
    const FVector End = Start + -SunLight->GetActorForwardVector() * (90000.0f * 1000.0f);

    GetWorld()->LineTraceSingleByChannel(SunHitResult, Start, End, ECC_Visibility, SunHitParams);

    //If we neither hit the sun or moon we are in shade.
    if (SunHitResult.Actor != nullptr)
    {
        const bool InShade = !(SunHitResult.Actor->ActorHasTag(TEXT("Sun")) || SunHitResult.Actor->ActorHasTag(
            TEXT("Moon")));
        //UE_LOG(LogTemp, Warning, TEXT("InShade %i"), InShade);
        return InShade;
    }
    return false;
}

void UPawnStatusComp::ReciveOnStat()
{
    //Parent->CallFunctionByNameWithArguments(TEXT("UpdateStatus"), ar, nullptr, true);
}

void UPawnStatusComp::CheckStatus()
{
    if (UseHealth)
    {
        if (GetStatusBase(Health, StatusBase) <= 0.5f)
        {
            Cast<AEpiCharacter>(GetOwner())->OnDeathEvent();
        }
    }
}

void UPawnStatusComp::SetLosingLife()
{
    const bool NewStat = Suffocate || Thirsty || Hungry || bUnderEffect;
    if (NewStat != bIsLosingHealth)
    {
        bIsLosingHealth = NewStat;
        SendLosingHealthUpdate.Broadcast();
    }
}

void UPawnStatusComp::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UPawnStatusComp, StatusType);
    DOREPLIFETIME(UPawnStatusComp, bIsLosingHealth);
}
