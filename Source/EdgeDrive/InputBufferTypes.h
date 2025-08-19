#pragma once
#include "CoreMinimal.h"
#include "Engine/Engine.h"  // "Endgine" → "Engine" 오타 수정
#include "InputBufferTypes.generated.h"

USTRUCT(BlueprintType)
struct EDGEDRIVE_API FBaseBufferedInput
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName InputName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeStamp = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Priority = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsConsumed = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D MovementData = FVector2D::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, FString> ExtraData;

    FBaseBufferedInput()
    {
        InputName = NAME_None;
        TimeStamp = 0.0f;
        Priority = 1;
        bIsConsumed = false;
        MovementData = FVector2D::ZeroVector;
    }

    FBaseBufferedInput(FName InInputName, float InTimeStamp, int32 InPriority)
    {
        InputName = InInputName;
        TimeStamp = InTimeStamp;
        Priority = InPriority;
        bIsConsumed = false;
        MovementData = FVector2D::ZeroVector;
    }
};

USTRUCT(BlueprintType)
struct EDGEDRIVE_API FInputBufferSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buffer Settings")
    float BufferDuration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buffer Settings")
    int32 MaxBufferedInputs = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sekiro Style")
    bool bUseSekiroPenalty = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sekiro Style")
    float PenaltyDecayTime = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sekiro Style")
    int32 MaxPenaltyStacks = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bDebugBuffer = false;

    FInputBufferSettings()
    {
        BufferDuration = 0.5f;
        MaxBufferedInputs = 3;
        bUseSekiroPenalty = false;
        PenaltyDecayTime = 0.5f;
        MaxPenaltyStacks = 4;
        bDebugBuffer = false;
    }
};

USTRUCT(BlueprintType)
struct EDGEDRIVE_API FCommitState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsCommitted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName CommittedAction = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CommitEndTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanBeInterrupted = false;
};

USTRUCT(BlueprintType)
struct EDGEDRIVE_API FAdvancedInputSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float BaseBufferDuration = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float MinBufferDuration = 0.067f; // 4 frames at 60fps

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Penalty")
    float PenaltyDecayTime = 0.5f; // 30 frames at 60fps

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Penalty")
    int32 MaxPenaltyStacks = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Penalty")
    float PenaltyMultiplier = 0.5f;
};

USTRUCT(BlueprintType)
struct EDGEDRIVE_API FInputWithTiming
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName InputName = NAME_None;  // 기본값 추가

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeStamp = 0.0f;  // 기본값 추가

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Priority = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanOverrideCommit = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsConsumed = false;
};

USTRUCT(BlueprintType)
struct EDGEDRIVE_API FBufferedInput
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName InputName = NAME_None;  // 기본값 추가

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeStamp = 0.0f;  // 기본값 추가

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsConsumed = false;

    FBufferedInput()
        : InputName(NAME_None), TimeStamp(0.0f), bIsConsumed(false) {
    }

    FBufferedInput(FName InInputName, float InTimeStamp)
        : InputName(InInputName), TimeStamp(InTimeStamp), bIsConsumed(false) {
    }
};

USTRUCT(BlueprintType)
struct EDGEDRIVE_API FInputBufferWindow
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsOpen = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName BufferTag = NAME_None;  // 기본값 추가

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> AllowedInputs;

    FInputBufferWindow()
        : bIsOpen(false), BufferTag(NAME_None) {
    }
};



// 추가적인 유틸리티 구조체들
UENUM(BlueprintType)
enum class EInputType : uint8
{
    None        UMETA(DisplayName = "None"),
    Dodge       UMETA(DisplayName = "Dodge"),         // Priority: 10
    Parry       UMETA(DisplayName = "Parry"),         // Priority: 9  
    GuardCounter UMETA(DisplayName = "Guard Counter"), // Priority: 8
    Attack      UMETA(DisplayName = "Attack"),        // Priority: 7
    Ability     UMETA(DisplayName = "Ability"),       // Priority: 5
    Sprint      UMETA(DisplayName = "Sprint"),        // Priority: 3
    LockOn      UMETA(DisplayName = "Lock On")        // Priority: 1
};

USTRUCT(BlueprintType)
struct EDGEDRIVE_API FInputPriorityData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EInputType InputType = EInputType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Priority = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BufferDuration = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanInterruptAttack = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanInterruptMovement = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bUseSekiroPenalty = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bUseEldenRingCommit = false;

    FInputPriorityData()
    {
        InputType = EInputType::None;
        Priority = 0;
        BufferDuration = 0.3f;
        bCanInterruptAttack = false;
        bCanInterruptMovement = false;
        bUseSekiroPenalty = false;
        bUseEldenRingCommit = false;
    }
};
