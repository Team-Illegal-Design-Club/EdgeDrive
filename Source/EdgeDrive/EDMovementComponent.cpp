#include "EDMovementComponent.h"
#include "EDCombatComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EDAbilityComponent.h"

UAnimMontage* UEDMovementComponent::SelectBestDodgeAnimation()
{
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        FVector MovementDirection;
        if (!AbilityComponent || !AbilityComponent->bIsLockingOn)
        {
            return Cast<UAnimMontage>(DodgeForwardAnim);
        }
        if (CurrentMovementInput.IsZero())
        {
            return Cast<UAnimMontage>(DodgeBackwardAnim);
        }

        const FRotator Rotation = Character->Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        MovementDirection = (ForwardDir * CurrentMovementInput.Y +
            RightDir * CurrentMovementInput.X).GetSafeNormal();

        float ForwardDot = FVector::DotProduct(MovementDirection, ForwardDir);
        float RightDot = FVector::DotProduct(MovementDirection, RightDir);

        if (FMath::Abs(ForwardDot) > FMath::Abs(RightDot))
        {
            return Cast<UAnimMontage>((ForwardDot > 0) ? DodgeForwardAnim : DodgeBackwardAnim);
        }
        else
        {
            return Cast<UAnimMontage>((RightDot > 0) ? DodgeRightAnim : DodgeLeftAnim);
        }
    }
    return nullptr;
}


UEDMovementComponent::UEDMovementComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    // Load Assets
    DodgeTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DodgeTimeline"));
    DodgeTimelineProgress.BindUFunction(this, FName("OnDodgeTimelineProgress"));

    MovementBufferSettings.BufferDuration = 0.5f;
    MovementBufferSettings.MaxBufferedInputs = 3;
    MovementBufferSettings.bUseSekiroPenalty = true;
    MovementBufferSettings.PenaltyDecayTime = 0.5;
    MovementBufferSettings.MaxPenaltyStacks = 4;
    MovementBufferSettings.bDebugBuffer = false;

}
void UEDMovementComponent::InitializeMovementComponent()
{
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        Character->bUseControllerRotationYaw = false;
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            MovementComp->bOrientRotationToMovement = true;
            MovementComp->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
            MovementComp->MaxWalkSpeed = WalkSpeed;
            MovementComp->MinAnalogWalkSpeed = 20.f;
            MovementComp->BrakingDecelerationWalking = 2000.f;
        }
    }
}
void UEDMovementComponent::OnDodgeTimelineProgress(float Value)
{
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        FVector CurrentVelocity = Character->GetVelocity();
        Character->GetCharacterMovement()->Velocity =
            FMath::Lerp(CurrentVelocity, FVector::ZeroVector, Value);
    }
}
void UEDMovementComponent::BeginPlay()
{
    Super::BeginPlay();
    if (DodgeCurve)
    {
        // ProgressFunction 대신 이미 바인딩된 DodgeTimelineProgress 사용
        DodgeTimeline->AddInterpFloat(DodgeCurve, DodgeTimelineProgress);

        FOnTimelineEvent FinishedFunction;
        FinishedFunction.BindUFunction(this, FName("OnDodgeEnd"));
        DodgeTimeline->SetTimelineFinishedFunc(FinishedFunction);

        DodgeTimeline->SetTimelineLength(1.0f);
        DodgeTimeline->SetPlayRate(1.5f);
        DodgeTimeline->SetLooping(false);
    }
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        AbilityComponent = Character->FindComponentByClass<UEDAbilityComponent>();
        CombatComponent = Character->FindComponentByClass<UEDCombatComponent>();
    }

}
void UEDMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Process Movement Input Buffer every frame
    ProcessMovementBuffer();
}
void UEDMovementComponent::SetDodgeTimelineSpeed(float Speed)
{
    if (DodgeTimeline)
    {
        DodgeTimeline->SetPlayRate(Speed);
    }
}
void UEDMovementComponent::SetupInput(UEnhancedInputComponent* PlayerInputComponent)
{
    if (!PlayerInputComponent) return;

    if (MoveAction)
    {
        FInputActionValue::Axis2D DeadZone;
        DeadZone.X = 0.2f;  
        DeadZone.Y = 0.2f;

        PlayerInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &UEDMovementComponent::MoveInput);
    }

    if (SprintAction)
    {
        PlayerInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &UEDMovementComponent::StartSprint);
        PlayerInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &UEDMovementComponent::EndSprint);
    }

    if (DodgeAction)
    {
        PlayerInputComponent->BindAction(DodgeAction, ETriggerEvent::Completed, this, &UEDMovementComponent::Dodge);
    }
}

void UEDMovementComponent::MoveInput(const FInputActionValue& Value)
{
    const FVector2D MovementVector = Value.Get<FVector2D>();
    CurrentMovementInput = MovementVector;

    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        const FRotator Rotation = Character->Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        // 입력이 없을 때 방향 초기화
        if (MovementVector.IsZero())
        {
            if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
            {
                // 기본 회전 설정으로 복구
                MovementComp->bOrientRotationToMovement = true;
                Character->bUseControllerRotationYaw = false;

                // 속도를 0으로 설정하여 부드럽게 정지
                MovementComp->Velocity = FVector::ZeroVector;
            }
            return;
        }

        Character->AddMovementInput(RightDirection, MovementVector.X);

        if (!b2DModeEnabled)
        {
            Character->AddMovementInput(ForwardDirection, MovementVector.Y);
        }
    }
}
void UEDMovementComponent::StartSprint()
{
    bIsSprint = true;
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            MovementComp->bOrientRotationToMovement = true;
            Character->bUseControllerRotationYaw = false;  // 달리는 동안 카메라 회전 따라가기 비활성화
            MovementComp->MaxWalkSpeed = SprintSpeed;
        }
    }
}

void UEDMovementComponent::EndSprint()
{
    bIsSprint = false;
    bCanDodgeAfterSprint = false; // 달리기 종료 시 대쉬 불가능하게 설정

    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            MovementComp->MaxWalkSpeed = WalkSpeed;

            // 락온 중이었다면 원래 회전 설정으로 복구
            if (AbilityComponent && AbilityComponent->bIsLockingOn)
            {
                Character->bUseControllerRotationYaw = true;
                MovementComp->bOrientRotationToMovement = false;
            }
        }
    }
    GetWorld()->GetTimerManager().SetTimer(
        SprintEndCooldownTimer,
        [this]() { bCanDodgeAfterSprint = true; },
        SprintEndCooldown,
        false
    );
}
FVector UEDMovementComponent::CalculateDodgeDirection(ACharacter* Character)
{
    const FRotator Rotation = Character->Controller->GetControlRotation();
    const FRotator YawRotation(0, Rotation.Yaw, 0);
    const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
    const FVector RightDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

    if (CurrentMovementInput.IsZero())
    {
        return -Character->GetActorForwardVector();
    }

    return (ForwardDir * CurrentMovementInput.Y +
        RightDir * CurrentMovementInput.X).GetSafeNormal();
}

void UEDMovementComponent::Dodge(const FInputActionValue& Value)
{
    if (bIsSprint || !bCanDodge || !bCanDodgeAfterSprint) return;

    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        // 닷지 방향 계산
        FVector DodgeDirection = CalculateDodgeDirection(Character);

        // 애니메이션 재생
        if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
        {
            if (UAnimMontage* BestMatchMontage = SelectBestDodgeAnimation())
            {
                AnimInstance->Montage_Play(BestMatchMontage);
            }
        }

        // 닷지 이동 적용
        Character->LaunchCharacter(DodgeDirection * DodgeDistance, true, true);

        bIsDodge = true;
        bCanDodge = false;

        // 타임라인 처리
        if (DodgeTimeline)
        {
            DodgeTimeline->Stop();
            DodgeTimeline->PlayFromStart();
        }

        // 쿨다운 타이머 설정
        GetWorld()->GetTimerManager().SetTimer(
            DodgeCooldownTimer,
            this,
            &UEDMovementComponent::OnDodgeEnd,
            DodgeCooldown,
            false
        );
    }
}



void UEDMovementComponent::OnDodgeEnd()
{
    bCanDodge = true;
    bIsDodge = false;  // 애니메이션 완료 후 상태 리셋
    CurrentMovementInput = {0,0};
    EndSprint();
}

bool UEDMovementComponent::IsSprint() const
{
    return bIsSprint;
}
bool UEDMovementComponent::IsWalk() const
{
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        // 이동 중이고, 달리기나 회피 상태가 아닐 때를 걷기로 판단
        return !CurrentMovementInput.IsZero() && !bIsSprint && !bIsDodge;
    }
    return false;
}
bool UEDMovementComponent::IsFalling() const
{
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        return Character->GetCharacterMovement()->IsFalling();
    }
    return false;
}

void UEDMovementComponent::AddMovementInputToBuffer(FName InputName, int32 Priority, FVector2D InputData)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (MovementBufferSettings.bUseSekiroPenalty)
    {
		UpdateInputPenalty(InputName);
        
		int32 CurrentPenalty = GetInputPriority(InputName);
        if (CurrentPenalty >= MovementBufferSettings.MaxPenaltyStacks)
        {
            if (MovementBufferSettings.bDebugBuffer)
            {
                UE_LOG(LogTemp, Warning, TEXT("Movement input %s ignored due to spam penalty: %d"),
                    *InputName.ToString(), CurrentPenalty);
            }
            return;
        }
    }
    // Check if can execute immediately
    if (CanExecuteMovementInput(InputName))
    {
        FBaseBufferedInput ImmediateInput(InputName, CurrentTime, Priority);
        ImmediateInput.MovementData = InputData;
        ExecuteMovementInput(ImmediateInput);
        return;
    }
    // Add to buffer
    CleanupMovementBuffer();

    if (MovementInputBuffer.Num() >= MovementBufferSettings.MaxBufferedInputs)
    {
        // Remove lowest priority input
        MovementInputBuffer.Sort([](const FBaseBufferedInput& A, const FBaseBufferedInput& B)
            {
                return A.Priority > B.Priority;
            });
        MovementInputBuffer.RemoveAt(MovementInputBuffer.Num() - 1);
    }

    FBaseBufferedInput BufferedInput(InputName, CurrentTime, Priority);
    BufferedInput.MovementData = InputData;
    MovementInputBuffer.Add(BufferedInput);

    LastInputTimes.Add(InputName, CurrentTime);

    if (MovementBufferSettings.bDebugBuffer)
    {
        UE_LOG(LogTemp, Log, TEXT("Added %s to movement buffer (Priority: %d, Buffer Size: %d)"),
            *InputName.ToString(), Priority, MovementInputBuffer.Num());
    }
}

bool UEDMovementComponent::ProcessMovementBuffer()
{
    CleanupMovementBuffer();

    if (MovementInputBuffer.Num() == 0) return false;
    MovementInputBuffer.Sort([](const FBaseBufferedInput& A, const FBaseBufferedInput& B)
        {
            if (A.Priority != B.Priority)
                return A.Priority > B.Priority;
			return A.TimeStamp < B.TimeStamp;
        });
    // Execute highest priority executable input
    for (FBaseBufferedInput& Input : MovementInputBuffer)
    {
        if (!Input.bIsConsumed && CanExecuteMovementInput(Input.InputName))
        {
            ExecuteMovementInput(Input);
            Input.bIsConsumed = true;
            return true;
        }
    }
    return false;
}
bool UEDMovementComponent::CanExecuteMovementInput(FName InputName) const
{
    if (InputName == FName("Dodge"))
    {
        bool bBasicCondition = bCanDodge && !bIsSprint && bCanDodgeAfterSprint;
        // Combat Component interaction check (Dodge has Highest Priority)
        // Combat Component interaction check (Dodge has highest priority)
        if (CombatComponent && CombatComponent->bIsAttacking)
        {
            // Dodge allowed even during attacks (FromSoftware style)
            return bBasicCondition;
        }

        return bBasicCondition;
    }
    else if (InputName == FName("EndSprint"))
    {
        return bIsSprint;
    }
    return true;
}

int32 UEDMovementComponent::GetInputPriority(FName InputName) const
{
    // FromSoftware style priorities
    if (InputName == FName("Dodge")) return 10;      // Highest priority (survival)
    if (InputName == FName("Sprint")) return 3;      // Low priority
    if (InputName == FName("EndSprint")) return 2;   // Lowest priority

    return 1; // Default
}

void UEDMovementComponent::UpdateInputPenalty(FName InputName)
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
    float LastTime = LastInputTimes.FindRef(InputName);
	//Check penalty decay
    if (CurrentTime - LastTime > MovementBufferSettings.PenaltyDecayTime)
    {
        InputPenalties.Remove(InputName);
        return;
    }
    // Check rapid successive input (Sekiro style)
    if(CurrentTime - LastTime < 0.1f)//Within 100ms
    {
		int32 CurrentPenalty = InputPenalties.FindRef(InputName);  
        InputPenalties.Add(InputName, FMath::Min(CurrentPenalty + 1, MovementBufferSettings.MaxPenaltyStacks));

        if (MovementBufferSettings.bDebugBuffer)
        {
            UE_LOG(LogTemp, Log, TEXT("Input penalty for %s: %d"),
                *InputName.ToString(), InputPenalties.FindRef(InputName));
        }
    }
}

void UEDMovementComponent::CleanupMovementBuffer()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    MovementInputBuffer.RemoveAll([CurrentTime, this](const FBaseBufferedInput& Input) {
        return (CurrentTime - Input.TimeStamp) > MovementBufferSettings.BufferDuration || Input.bIsConsumed; 
        });
}
void UEDMovementComponent::ClearMovementBuffer()
{
}



void UEDMovementComponent::SetMovementBufferSettings(const FInputBufferSettings& NewSettings)
{
}
