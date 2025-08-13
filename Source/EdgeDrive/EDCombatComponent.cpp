#include "EDCombatComponent.h"
#include "GameFramework/Character.h"
#include "EnhancedInputComponent.h"
#include "EDCharacter.h"

#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"

UEDCombatComponent::UEDCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    // 메시 컴포넌트들을 CreateDefaultSubobject로 생성

    AttackTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("AttackTimeline"));
    AttackTimeline->SetIgnoreTimeDilation(true);
    AttackTimeline->SetTickGroup(TG_PostUpdateWork);

   
}
void UEDCombatComponent::BeginPlay()
{
    Super::BeginPlay();



    // 소켓 이름 설정
    StartSocketNames.Add(EAttackLimb::LeftHand, FName("Start"));
    StartSocketNames.Add(EAttackLimb::RightHand, FName("Start"));
    StartSocketNames.Add(EAttackLimb::LeftFoot, FName("Start"));
    StartSocketNames.Add(EAttackLimb::RightFoot, FName("Start"));

    EndSocketNames.Add(EAttackLimb::LeftHand, FName("End"));
    EndSocketNames.Add(EAttackLimb::RightHand, FName("End"));
    EndSocketNames.Add(EAttackLimb::LeftFoot, FName("End"));
    EndSocketNames.Add(EAttackLimb::RightFoot, FName("End"));
    for (const auto& Pair : LimbMeshes)
    {
        UE_LOG(LogTemp, Warning, TEXT("Initialized Limb: %s, Mesh: %s"),
            *UEnum::GetValueAsString(Pair.Key),
            Pair.Value ? *Pair.Value->GetName() : TEXT("None"));
    }

    // 타임라인 설정
    if (AttackCurve)
    {
        FOnTimelineFloat ProgressFunction;
        AttackTimeline->AddInterpFloat(AttackCurve, ProgressFunction);

        FOnTimelineEvent FinishedFunction;
        FinishedFunction.BindUFunction(this, FName("ResetCombo"));
        AttackTimeline->SetTimelineFinishedFunc(FinishedFunction);

        AttackTimeline->SetTimelineLength(1.0f);
        AttackTimeline->SetPlayRate(1 / ComboResetTime);
        AttackTimeline->SetLooping(false);
    }
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        MovementComponent = Character->FindComponentByClass<UEDMovementComponent>();
    }
}
void UEDCombatComponent::SetupInput(UEnhancedInputComponent* PlayerInputComponent)
{
    if (!PlayerInputComponent) return;

    if (AttackAction)
    {
        PlayerInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &UEDCombatComponent::StartAttack);
    }
}

void UEDCombatComponent::SetAttackTimelineSpeed(float Speed)
{
    if (AttackTimeline)
    {
        AttackTimeline->SetPlayRate(Speed);
    }
}
void UEDCombatComponent::StartAttack()
{
    if (MovementComponent->IsDodge())
        return;

    // Use advanced input buffer system
    if (bIsAttacking && !bCanCombo)
    {
        // In Elden Ring style, buffer with high priority
        AddAdvancedInputToBuffer(FName("Attack"), 2.0f, false);
        return;
    }

    // Commit to attack (Elden Ring style commitment)
    if (bUseEldenRingCommitSystem)
    {
        float AttackDuration = 0.5f; // Adjust based on animation
        if (CurrentComboIndex < ComboAttacks.Num())
        {
            // Some attacks can be interrupted by dodge
            bool bCanBeInterrupted = (CurrentComboIndex == 0); // Only first attack can be interrupted
            CommitToAction(FName("Attack"), AttackDuration, bCanBeInterrupted);
        }
    }

    // Rest of your existing attack logic...
    ACharacter* Character = Cast<ACharacter>(GetOwner());

    if (!Character || !Character->GetMesh() || !Character->GetMesh()->GetAnimInstance())
        return;

    if (ComboAttacks.Num() == 0)
        return;

    // Your existing attack rotation and direction logic...
    if (bCanCombo || !bIsAttacking)
    {
        FVector AttackDirection = CalculateAttackDirection(Character);
        FRotator TargetRotation = AttackDirection.Rotation();
        FRotator NewRotation = FMath::RInterpTo(
            Character->GetActorRotation(),
            FRotator(0.0f, TargetRotation.Yaw, 0.0f),
            GetWorld()->GetDeltaSeconds(),
            10.0f
        );
        Character->SetActorRotation(NewRotation);
    }

    if (CurrentComboIndex >= ComboAttacks.Num())
    {
        ResetCombo();
    }

    FComboAttackData& CurrentAttackData = ComboAttacks[CurrentComboIndex];

    bHasHitThisAttack = false;
    bIsAttacking = true;
    bCanCombo = false;

    Character->PlayAnimMontage(CurrentAttackData.Montage);

    if (ComboResetTime > 0.0f && AttackTimeline)
    {
        AttackTimeline->Stop();
        AttackTimeline->PlayFromStart();
    }

    CurrentComboIndex++;
}

//void UEDCombatComponent::StartAttack()
//{
//    if (MovementComponent->IsDodge())
//        return;
//    if (bIsAttacking && !bCanCombo) return;
//
//    ACharacter* Character = Cast<ACharacter>(GetOwner());
//    
//    if (!Character || !Character->GetMesh() || !Character->GetMesh()->GetAnimInstance()) return;
//
//    if (ComboAttacks.Num() == 0) return;
//    if (bCanCombo || !bIsAttacking)
//    {
//        FVector AttackDirection = CalculateAttackDirection(Character);
//        FRotator TargetRotation = AttackDirection.Rotation();
//        FRotator NewRotation = FMath::RInterpTo(
//            Character->GetActorRotation(),
//            FRotator(0.0f, TargetRotation.Yaw, 0.0f),
//            GetWorld()->GetDeltaSeconds(),
//            10.0f
//        );
//        Character->SetActorRotation(NewRotation);
//    }
//    if (CurrentComboIndex >= ComboAttacks.Num())
//    {
//        ResetCombo();
//    }
//
//    FComboAttackData& CurrentAttackData = ComboAttacks[CurrentComboIndex];
//    //if (!CurrentAttackData.Montage) return;
//    //if (Character->GetMesh()->GetAnimInstance()->Montage_IsPlaying(CurrentAttackData.Montage))
//    //{
//    //    return;
//    //}
//
//    bHasHitThisAttack = false;
//    bIsAttacking = true;
//    bCanCombo = false;
//
//    Character->PlayAnimMontage(CurrentAttackData.Montage);
//
//    if (ComboResetTime > 0.0f && AttackTimeline)
//    {
//        AttackTimeline->Stop();
//        AttackTimeline->PlayFromStart();
//    }
//
//    //// 현재 공격에 사용되는 부위의 메시로 라인트레이스 실행
//
//    CurrentComboIndex++;
//}


void UEDCombatComponent::LineTrace()
{
    // 이미 히트했다면 리턴
   // if (bHasHitThisAttack) return;
    if (!ComboAttacks.IsValidIndex(CurrentComboIndex - 1))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid combo index"));
        return;
    }

    const FComboAttackData& CurrentAttackData = ComboAttacks[CurrentComboIndex - 1];
    UStaticMeshComponent* CurrentLimbMesh = LimbMeshes[CurrentAttackData.AttackLimb];
    if (!CurrentLimbMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("Missing limb mesh for: %s"),
            *UEnum::GetValueAsString(CurrentAttackData.AttackLimb));
        return;
    }

    // 소켓 이름 가져오기
    FName StartSocket = StartSocketNames[CurrentAttackData.AttackLimb];
    FName EndSocket = EndSocketNames[CurrentAttackData.AttackLimb];

    // 소켓 존재 여부 확인
    if (!CurrentLimbMesh->DoesSocketExist(StartSocket) ||
        !CurrentLimbMesh->DoesSocketExist(EndSocket))
    {
        UE_LOG(LogTemp, Warning, TEXT("Socket does not exist on mesh"));
        return;
    }

    FVector StartLocation = CurrentLimbMesh->GetSocketLocation(StartSocket);
    FVector EndLocation = CurrentLimbMesh->GetSocketLocation(EndSocket);

    float CurrentRadius = GetCurrentComboRadius();
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    TArray<FHitResult> HitResults;

    DrawDebugCapsule(
        GetWorld(),
        (StartLocation + EndLocation) / 2.0f,
        FVector::Distance(StartLocation, EndLocation) / 2.0f,
        CurrentRadius,
        FQuat::FindBetweenNormals(FVector::UpVector, (EndLocation - StartLocation).GetSafeNormal()),
        FColor::Red,
        false,
        0.2f,
        0,
        2.0f
    );

    bool bHit = GetWorld()->SweepMultiByChannel(
        HitResults,
        StartLocation,
        EndLocation,
        FQuat::FindBetweenNormals(FVector::UpVector, (EndLocation - StartLocation).GetSafeNormal()),
        ECC_Visibility,
        FCollisionShape::MakeCapsule(CurrentRadius, FVector::Distance(StartLocation, EndLocation) / 2.0f),
        QueryParams
    );

    for (const FHitResult& Hit : HitResults)
    {
        if (AActor* ActorHit = Hit.GetActor())
        {
            DrawDebugPoint(
                GetWorld(),
                Hit.Location,
                10.0f,
                FColor::Green,
                false,
                0.2f,
                0
            );
            float CurrentDamage = GetCurrentComboDamage();
            UGameplayStatics::ApplyDamage(ActorHit, CurrentDamage, GetOwner()->GetInstigatorController(), GetOwner(), nullptr);
            if (Cast<APawn>(ActorHit)) {
                if (CurrentComboIndex == ComboAttacks.Num())
                    PlayHitEffect(Hit.Location);
               // ApplyHitStop();
            }
            // 히트 발생 시 플래그 설정
            bHasHitThisAttack = true;

        }
    }
}




void UEDCombatComponent::PlayHitEffect(const FVector& HitLocation)
{
    if (AfterImageTemplate)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            AfterImageTemplate,
            HitLocation,
            GetOwner()->GetActorRotation()
        );
    }
}


void UEDCombatComponent::EnableComboWindow()
{
    bCanCombo = true;
}

void UEDCombatComponent::DisableComboWindow()
{
    bCanCombo = false;
}

void UEDCombatComponent::ResetCombo()
{
    CurrentComboIndex = 0;
    bCanCombo = false;
    bIsAttacking = false;
    bHasHitThisAttack = false;
    // 디버그 메시지 출력
   /* UKismetSystemLibrary::PrintString(GetWorld(),
        FString::Printf(TEXT("Combo Reset Timer Triggered at: %.2f"), GetWorld()->GetTimeSeconds()),
        true, true, FColor::Red, 2.0f);*/

}

float UEDCombatComponent::GetCurrentComboDamage() const
{
    if (CurrentComboIndex > 0 && CurrentComboIndex <= ComboAttacks.Num())
    {
        return ComboAttacks[CurrentComboIndex - 1].Damage;
    }
    return Damage;
}

float UEDCombatComponent::GetCurrentComboRadius() const
{
    if (CurrentComboIndex > 0 && CurrentComboIndex <= ComboAttacks.Num())
    {
        return ComboAttacks[CurrentComboIndex - 1].Radius;
    }
    return 20.0f;
}

FVector UEDCombatComponent::CalculateAttackDirection(ACharacter* Character)
{
    const FRotator Rotation = Character->Controller->GetControlRotation();
    const FRotator YawRotation(0, Rotation.Yaw, 0);
    const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
    const FVector RightDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

    if (MovementComponent && !MovementComponent->CurrentMovementInput.IsZero())
    {
        FVector2D Input = MovementComponent->CurrentMovementInput;
        return (ForwardDir * Input.Y + RightDir * Input.X).GetSafeNormal();
    }

    // 입력이 없을 경우 캐릭터가 바라보는 방향 사용
    return Character->GetActorForwardVector();
}

void UEDCombatComponent::ApplyHitStop()
{
    OriginalTimeDilation = UGameplayStatics::GetGlobalTimeDilation(GetWorld());

    // 월드의 시간 배율 조정
    UGameplayStatics::SetGlobalTimeDilation(GetWorld(), HitStopTimeDilation);

    // 타이머로 히트스톱 해제 예약
    GetWorld()->GetTimerManager().SetTimer(
        HitStopTimerHandle,
        this,
        &UEDCombatComponent::ResetHitStop,
        HitStopDuration * HitStopTimeDilation,  // 실제 시간으로 조정
        false
    );
}

void UEDCombatComponent::ResetHitStop()
{
    UGameplayStatics::SetGlobalTimeDilation(GetWorld(), OriginalTimeDilation);
}

void UEDCombatComponent::ExecuteBufferedAction(FName ActionName)
{
    if (ActionName == FName("Attack"))
    {
        StartAttack();
    }
    else if (ActionName == FName("Parry"))
    {
        // If you have a Parry function, call it here:
        // StartParry();
    }
}

void UEDCombatComponent::AddInputToBuffer(FName InputName)
{
    if (!IsInputAllowedInCurrentWindow(InputName))
    {
        return;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();

    // Clean up expired inputs first
    CleanupExpiredInputs();

    // Check if we have room for more inputs
    if (InputBuffer.Num() >= MaxBufferedInputs)
    {
        // Remove oldest input to make room
        InputBuffer.RemoveAt(0);
    }

    // Add new input to buffer
    InputBuffer.Add(FBufferedInput(InputName, CurrentTime));

    UE_LOG(LogTemp, Log, TEXT("Added input %s to buffer at time %.2f"),
        *InputName.ToString(), CurrentTime);
}

bool UEDCombatComponent::HasBufferedInput(FName InputName)
{
    CleanupExpiredInputs();

    for (const FBufferedInput& BufferedInput : InputBuffer)
    {
        if (BufferedInput.InputName == InputName && !BufferedInput.bIsConsumed)
        {
            return true;
        }
    }
    return false;
}

FBufferedInput UEDCombatComponent::ConsumeBufferedInput(FName InputName)
{
    CleanupExpiredInputs();

    for (FBufferedInput& BufferedInput : InputBuffer)
    {
        if (BufferedInput.InputName == InputName && !BufferedInput.bIsConsumed)
        {
            BufferedInput.bIsConsumed = true;
            UE_LOG(LogTemp, Log, TEXT("Consumed buffered input: %s"), *InputName.ToString());
            return BufferedInput;
        }
    }

    return FBufferedInput(); // Return empty input if not found
}

void UEDCombatComponent::OpenInputBufferWindow(FName BufferTag, const TArray<FName>& AllowedInputs)
{
    CurrentBufferWindow.bIsOpen = true;
    CurrentBufferWindow.BufferTag = BufferTag;
    CurrentBufferWindow.AllowedInputs = AllowedInputs;

    UE_LOG(LogTemp, Log, TEXT("Opened input buffer window: %s"), *BufferTag.ToString());
}

void UEDCombatComponent::CloseInputBufferWindow()
{
    CurrentBufferWindow.bIsOpen = false;
    ProcessBufferedInputs();

    UE_LOG(LogTemp, Log, TEXT("Closed input buffer window: %s"),
        *CurrentBufferWindow.BufferTag.ToString());
}

void UEDCombatComponent::ProcessBufferedInputs()
{
    if (InputBuffer.Num() == 0)
    {
        return;
    }

    // Process the most recent unconsumed input
    for (int32 i = InputBuffer.Num() - 1; i >= 0; i--)
    {
        FBufferedInput& BufferedInput = InputBuffer[i];
        if (!BufferedInput.bIsConsumed)
        {
            // Execute the buffered input
            if (BufferedInput.InputName == FName("Attack"))
            {
                StartAttack();
            }
            // Add more input types as needed

            BufferedInput.bIsConsumed = true;
            break; // Only process one input at a time
        }
    }
}

void UEDCombatComponent::ClearInputBuffer()
{
    InputBuffer.Empty();
    UE_LOG(LogTemp, Log, TEXT("Cleared input buffer"));
}

void UEDCombatComponent::CleanupExpiredInputs()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();

    InputBuffer.RemoveAll([CurrentTime, this](const FBufferedInput& Input)
        {
            return (CurrentTime - Input.TimeStamp) > InputBufferDuration;
        });
}

bool UEDCombatComponent::IsInputAllowedInCurrentWindow(FName InputName) const
{
    if (!CurrentBufferWindow.bIsOpen)
    {
        return false;
    }

    if (CurrentBufferWindow.AllowedInputs.Num() == 0)
    {
        return true; // If no specific inputs defined, allow all
    }

    return CurrentBufferWindow.AllowedInputs.Contains(InputName);
}
void UEDCombatComponent::AddAdvancedInputToBuffer(FName InputName, float Priority, bool bCanOverrideCommit)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();

    // Sekiro-style penalty system
    if (bUseSekiroPenaltySystem)
    {
        UpdateInputPenalty();

        // If spamming too much, ignore the input
        if (InputSpamPenalty >= InputSettings.MaxPenaltyStacks)
        {
            UE_LOG(LogTemp, Warning, TEXT("Input ignored due to spam penalty"));
            return;
        }
    }

    // Check if we're committed to an action and can't override
    if (bUseEldenRingCommitSystem && IsActionCommitted() && !bCanOverrideCommit)
    {
        // Buffer the input for after commit ends
        if (CurrentCommitState.CommitEndTime - CurrentTime > 0)
        {
            FInputWithTiming BufferedInput;
            BufferedInput.InputName = InputName;
            BufferedInput.TimeStamp = CurrentTime;
            BufferedInput.Priority = Priority;
            BufferedInput.bCanOverrideCommit = bCanOverrideCommit;

            AdvancedInputBuffer.Add(BufferedInput);

            UE_LOG(LogTemp, Log, TEXT("Buffered input %s during commit"), *InputName.ToString());
        }
        return;
    }

    // Process input immediately if possible
    if (!IsActionCommitted() || bCanOverrideCommit)
    {
        ExecuteBufferedAction(InputName);
    }

    LastInputTime = CurrentTime;
}

void UEDCombatComponent::UpdateInputPenalty()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();

    // Check if enough time has passed to reduce penalty
    if (CurrentTime - LastInputTime > InputSettings.PenaltyDecayTime)
    {
        InputSpamPenalty = 0;
        CurrentDeflectWindow = InputSettings.BaseBufferDuration;
    }
    else if (CurrentTime - LastInputTime < 0.1f) // Quick succession
    {
        InputSpamPenalty = FMath::Min(InputSpamPenalty + 1, InputSettings.MaxPenaltyStacks);

        // Reduce deflect window based on penalty
        float PenaltyRatio = (float)InputSpamPenalty / (float)InputSettings.MaxPenaltyStacks;
        CurrentDeflectWindow = FMath::Lerp(
            InputSettings.BaseBufferDuration,
            InputSettings.MinBufferDuration,
            PenaltyRatio
        );

        UE_LOG(LogTemp, Log, TEXT("Input penalty: %d, Deflect window: %.3f"),
            InputSpamPenalty, CurrentDeflectWindow);
    }
}

bool UEDCombatComponent::ProcessAdvancedBuffer()
{
    if (IsActionCommitted() && !CurrentCommitState.bCanBeInterrupted)
    {
        return false;
    }

    CleanupAdvancedBuffer();

    if (AdvancedInputBuffer.Num() == 0)
    {
        return false;
    }

    // Sort by priority and timestamp
    AdvancedInputBuffer.Sort([](const FInputWithTiming& A, const FInputWithTiming& B) {
        if (A.Priority != B.Priority)
            return A.Priority > B.Priority;
        return A.TimeStamp > B.TimeStamp; // More recent first
        });

    // Execute highest priority input
    FInputWithTiming& HighestPriorityInput = AdvancedInputBuffer[0];
    if (!HighestPriorityInput.bIsConsumed)
    {
        ExecuteBufferedAction(HighestPriorityInput.InputName);
        HighestPriorityInput.bIsConsumed = true;
        return true;
    }

    return false;
}

void UEDCombatComponent::CommitToAction(FName ActionName, float Duration, bool bCanBeInterrupted)
{
    CurrentCommitState.bIsCommitted = true;
    CurrentCommitState.CommittedAction = ActionName;
    CurrentCommitState.CommitEndTime = GetWorld()->GetTimeSeconds() + Duration;
    CurrentCommitState.bCanBeInterrupted = bCanBeInterrupted;

    UE_LOG(LogTemp, Log, TEXT("Committed to action: %s for %.2f seconds"),
        *ActionName.ToString(), Duration);
}

bool UEDCombatComponent::IsActionCommitted() const
{
    if (!CurrentCommitState.bIsCommitted)
        return false;

    float CurrentTime = GetWorld()->GetTimeSeconds();
    return CurrentTime < CurrentCommitState.CommitEndTime;
}

void UEDCombatComponent::CleanupAdvancedBuffer()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float BufferDuration = CalculateCurrentBufferDuration();

    AdvancedInputBuffer.RemoveAll([CurrentTime, BufferDuration](const FInputWithTiming& Input)
        {
            return (CurrentTime - Input.TimeStamp) > BufferDuration || Input.bIsConsumed;
        });
}

float UEDCombatComponent::CalculateCurrentBufferDuration() const
{
    if (bUseSekiroPenaltySystem)
    {
        return CurrentDeflectWindow;
    }
    return InputSettings.BaseBufferDuration;
}
void UEDCombatComponent::StartDeflectWindow()
{
    bCanDeflect = true;
    CurrentDeflectWindow = CalculateCurrentBufferDuration();

    // Set timer to end deflect window
    GetWorld()->GetTimerManager().SetTimer(
        DeflectTimerHandle,
        this,
        &UEDCombatComponent::EndDeflectWindow,
        CurrentDeflectWindow,
        false
    );

    UE_LOG(LogTemp, Log, TEXT("Deflect window opened for %.3f seconds"), CurrentDeflectWindow);
}

void UEDCombatComponent::EndDeflectWindow()
{
    bCanDeflect = false;
    UE_LOG(LogTemp, Log, TEXT("Deflect window closed"));
}

bool UEDCombatComponent::AttemptDeflect()
{
    if (!bCanDeflect)
    {
        return false;
    }

    // Reset penalty on successful deflect
    InputSpamPenalty = 0;
    CurrentDeflectWindow = InputSettings.BaseBufferDuration;

    // Deflect successful - apply effects
    bCanDeflect = false;
    GetWorld()->GetTimerManager().ClearTimer(DeflectTimerHandle);

    UE_LOG(LogTemp, Log, TEXT("Successful deflect! Penalty reset."));
    return true;
}
