#include "EDMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"



UEDMovementComponent::UEDMovementComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    // Load Assets
    DodgeTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DodgeTimeline"));
    DodgeTimelineProgress.BindUFunction(this, FName("OnDodgeTimelineProgress"));

    //static ConstructorHelpers::FObjectFinder<UCurveFloat> CurveAssets(TEXT(""));


}
void UEDMovementComponent::InitializeMovementComponent()
{
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
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
            MovementComp->MaxWalkSpeed = SprintSpeed;
        }
    }
}

void UEDMovementComponent::EndSprint()
{
    bIsSprint = false;
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            MovementComp->MaxWalkSpeed = WalkSpeed;
        }
    }
}

void UEDMovementComponent::Dodge(const FInputActionValue& Value)
{
    if (bIsSprint || !bCanDodge) return;
    bIsDodge = true;


    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        FVector DodgeDirection;

        if (CurrentMovementInput.IsZero())
        {
            // If no movement input, dodge backwards
            DodgeDirection = -Character->GetActorForwardVector();
        }
        else
        {
            // Dodge in the direction of movement input
            const FRotator Rotation = Character->Controller->GetControlRotation();
            const FRotator YawRotation(0, Rotation.Yaw, 0);

            const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
            const FVector RightDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

            DodgeDirection = (ForwardDir * CurrentMovementInput.Y + RightDir * CurrentMovementInput.X).GetSafeNormal();
        }

        Character->LaunchCharacter(DodgeDirection * DodgeDistance, true, true);
        bIsDodge = true;
        bCanDodge = false;
        if (DodgeTimeline)
        {
            DodgeTimeline->Stop();  // 기존 타임라인 정지

            DodgeTimeline->PlayFromStart();


        }
        //GetWorld()->GetTimerManager().SetTimer(
        //    DodgeCooldownTimer,
        //    FTimerDelegate::CreateUObject(this, &UEDMovementComponent::OnDodgeEnd),
        //    DodgeCooldown,
        //    false
        //
        //);
    }
}

void UEDMovementComponent::OnDodgeEnd()
{
    bCanDodge = true;
    bIsDodge = false;  // 애니메이션 완료 후 상태 리셋
    EndSprint();
}

bool UEDMovementComponent::IsSprint() const
{
    return bIsSprint;
}

bool UEDMovementComponent::IsFalling() const
{
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        return Character->GetCharacterMovement()->IsFalling();
    }
    return false;
}
