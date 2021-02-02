// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

//Engine
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Core.h"
#include "Net/UnrealNetwork.h"

#include "PawnStatusComp.generated.h"

// STATUS SYSTEM

UENUM(BlueprintType)
enum EStatus
{
	//Delta Describes the Increase or Decrease of a Value.
	Health        	UMETA(DisplayName = "Health"),
	Blood        	UMETA(DisplayName = "Blood"), 
	Water         	UMETA(DisplayName = "Water"),    
	Food          	UMETA(DisplayName = "Food"),   
    Heat          	UMETA(DisplayName = "Heat"),
	Stamina         UMETA(DisplayName = "Stamina"),
    //Keep this the last entry so we know the lenght.
    LastEntry      	UMETA(DisplayName = "LastEntry")
};    

UENUM(BlueprintType)
enum EStatusInfo
{
	Base         	UMETA(DisplayName = "Base"),
	Max          	UMETA(DisplayName = "Max"),
    Min          	UMETA(DisplayName = "Min")
};  

UENUM(BlueprintType)
enum EStatusMeta
{
	StatusBase     	UMETA(DisplayName = "StatusBase"),  
    StatusDelta    	UMETA(DisplayName = "StatusDelta")
}; 

USTRUCT(BlueprintType)
struct FStatusBase
{
	GENERATED_USTRUCT_BODY()
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float Base;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float Min;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float Max;
};

USTRUCT(BlueprintType)
struct FStatusEffect
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	FString StatusName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	TEnumAsByte<EStatus> StatusToEffect;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float Duration;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float Amount;

	FStatusEffect()
	{
		StatusName = "none";
		StatusToEffect = LastEntry;
		Duration = 0.0f;
		Amount = 0.0f;	
	};

	FStatusEffect(EStatus NewStatus, float NewDuration, float NewAmount)
	{
		StatusName = "none";
		StatusToEffect = NewStatus;
		Duration = NewDuration;
		Amount = NewAmount;	
	};
};

USTRUCT(BlueprintType)
struct FStatusFull
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	TEnumAsByte<EStatus> Status;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	FStatusBase StatusBase;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	FStatusBase StatusDelta;
};

USTRUCT(BlueprintType)
struct FEffectValues
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float Duration;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float Amount;

	FEffectValues()
	{
		Duration = 1.0f;
		Amount = 1.0f;
	};

	FEffectValues(float NewDuration, float NewAmount)
	{
		Duration = NewDuration;
		Amount = NewAmount;
	};
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatusChanged, EStatus, Name);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
class EPIDEMIC_API UPawnStatusComp : public UActorComponent
{
	GENERATED_UCLASS_BODY()

public:	
	// Sets default values for this component's properties
	UPawnStatusComp();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	   FOutputDeviceNull ar;


    UFUNCTION(BlueprintCallable, Category = "Status|Functions")
    float& GetStatus(EStatus Name, EStatusMeta Meta, EStatusInfo Info);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Status|Functions")
    float& GetStatusPure(EStatus Name, EStatusMeta Meta, EStatusInfo Info);
    
    UFUNCTION(BlueprintCallable, Category = "Status|Functions")
    float& SetStatus(float Value, EStatus Name, EStatusMeta Meta, EStatusInfo Info); // set a status (not increasing or decrease current value)
	float& SetStatusSave(float Value, EStatus Name, EStatusMeta Meta, EStatusInfo Info);

	UFUNCTION(BlueprintCallable, Category = "Status|Functions")
    float& AddStatus(float Value, EStatus Name, EStatusMeta Meta, EStatusInfo Info); // increase or decrease status

    UFUNCTION(BlueprintCallable, Category = "Status|Functions")
    float& AddStatusSave(float Value, EStatus Name, EStatusMeta Meta); // increase or decrease value clamped to its min/max
	float& AddStatusSaveNoDelta(float Value, EStatus Name, EStatusMeta Meta);

	//Directly Accesses the Base value of a Status.
    UFUNCTION()
    float& GetStatusBase(const EStatus Name, EStatusMeta Meta);

    //Directly Accesses the Min value of a Status.
    UFUNCTION()
    float& GetStatusMin(const EStatus Name, EStatusMeta Meta);

    //Directly Accesses the Max value of a Status.
    UFUNCTION()
    float& GetStatusMax(const EStatus Name, EStatusMeta Meta);

	UFUNCTION(BlueprintCallable, Category = "iStatus|Functions")
	float GetStatusPercent(const EStatus Name, EStatusMeta Meta);

	UFUNCTION(BlueprintCallable, Category = "iStatus|Functions")
	void AddEffect(UPARAM(ref)FStatusEffect& EffectToAdd);

	UFUNCTION()
	void HandleHeat();
	
    UFUNCTION()
    void ReciveOnStat();

	UFUNCTION()
	void CheckStatus();

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	TMap<TEnumAsByte<EStatus>, FEffectValues> ActiveEffects;
	
	UPROPERTY(BlueprintAssignable, Category = "Chat")
	FOnStatusChanged OnStatusChanged;

	UPROPERTY()
	TArray<FStatusEffect> StatusEffects;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Status")
	TArray<FStatusFull> StatusType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status|Health")
    bool UseHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status|Blood")
    bool UseBlood;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status|Water")
    bool UseWater;

    UPROPERTY(BlueprintReadWrite, Category = "Status")
    bool bUnderEffect;

    UPROPERTY(Replicated ,BlueprintReadOnly, Category = "Status")
    bool bIsLosingHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status|Food")
    bool UseFood;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status|Heat")
    bool UseHeat;

	UPROPERTY()
	bool IsSprinting;
	
    // Heat code, @todo move sunlight detection to a different component.
    UFUNCTION(BlueprintCallable, Category = "Status|Functions")
    bool InShade();
    
    UPROPERTY()
    AActor* SunLight;

    FCollisionQueryParams SunHitParams;
    FHitResult SunHitResult;

	UPROPERTY(EditAnywhere ,BlueprintReadWrite)
	float BiomeHeat;
	
    UPROPERTY(BlueprintReadWrite, Category = "Status|Heat")
    float HotAlpha;
    
    UPROPERTY(BlueprintReadWrite, Category = "Status|Heat")
    float ColdAlpha;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FonLosingHealth);

    UPROPERTY(BlueprintAssignable, Category = "inventory")
    FonLosingHealth SendLosingHealthUpdate;

    
    void SetLosingLife();


    UPROPERTY(BlueprintReadWrite, Category = "Status")
    bool Death = false;
    UPROPERTY(BlueprintReadWrite, Category = "Status")
    bool Suffocate = false;
    UPROPERTY(BlueprintReadWrite, Category = "Status")
    bool Thirsty = false;
    UPROPERTY(BlueprintReadWrite, Category = "Status")
    bool Hungry = false;
    UPROPERTY(BlueprintReadWrite, Category = "Status")
    bool OutOfEnergy = false;

private:

    float StatusNone = 0.0f;
		
};
