#include "EDHeroComponent.h"
#include "../Camera/EDCameraMode.h" 
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UEDHeroComponent::UEDHeroComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    AbilityCameraMode = nullptr;
    bReadyToBindInputs = false;
}

void UEDHeroComponent::OnRegister()
{
    Super::OnRegister();

    if (!GetPawn<APawn>())
    {
        UE_LOG(LogTemp, Error, TEXT("[UEDHeroComponent::OnRegister] This component must be added to a Pawn."));
    }
}

void UEDHeroComponent::BeginPlay()
{
    Super::BeginPlay();

    // Enhanced Input √ ±‚»≠
    if (APawn* Pawn = GetPawn<APawn>())
    {
        if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
        {
            if (ULocalPlayer* LP = PC->GetLocalPlayer())
            {
                if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
                {
                    Subsystem->ClearAllMappings();

                    FModifyContextOptions Options = {};
                    Options.bIgnoreAllPressedKeysUntilRelease = false;
                    Subsystem->AddMappingContext(DefaultMappingContext, 0, Options);
                }
            }
        }
    }
}

void UEDHeroComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

void UEDHeroComponent::InitializePlayerInput(UInputComponent* PlayerInputComponent)
{
    if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &UEDHeroComponent::Input_Move);
        EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &UEDHeroComponent::Input_LookMouse);
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &UEDHeroComponent::Input_Jump);
        EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &UEDHeroComponent::Input_Crouch);
        EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &UEDHeroComponent::Input_Sprint);
    }
}

void UEDHeroComponent::Input_Move(const FInputActionValue& InputActionValue)
{
    APawn* Pawn = GetPawn<APawn>();
    if (!Pawn) return;

    const FVector2D Value = InputActionValue.Get<FVector2D>();
    const FRotator Rotation = Pawn->GetControlRotation();
    const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

    if (Value.X != 0.0f)
    {
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        Pawn->AddMovementInput(Direction, Value.X);
    }

    if (Value.Y != 0.0f)
    {
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        Pawn->AddMovementInput(Direction, Value.Y);
    }
}

void UEDHeroComponent::Input_LookMouse(const FInputActionValue& InputActionValue)
{
    APawn* Pawn = GetPawn<APawn>();
    if (!Pawn) return;

    const FVector2D Value = InputActionValue.Get<FVector2D>();

    if (Value.X != 0.0f)
    {
        Pawn->AddControllerYawInput(Value.X);
    }
    if (Value.Y != 0.0f)
    {
        Pawn->AddControllerPitchInput(Value.Y);
    }
}

void UEDHeroComponent::Input_LookStick(const FInputActionValue& InputActionValue)
{
    APawn* Pawn = GetPawn<APawn>();
    if (!Pawn) return;

    const FVector2D Value = InputActionValue.Get<FVector2D>();
    const float DeltaTime = GetWorld()->GetDeltaSeconds();

    if (Value.X != 0.0f)
    {
        Pawn->AddControllerYawInput(Value.X * 300.0f * DeltaTime);
    }
    if (Value.Y != 0.0f)
    {
        Pawn->AddControllerPitchInput(Value.Y * 165.0f * DeltaTime);
    }
}

void UEDHeroComponent::Input_Crouch(const FInputActionValue& InputActionValue)
{
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (Character->CanCrouch())
        {
            if (Character->bIsCrouched)
            {
                Character->UnCrouch();
            }
            else
            {
                Character->Crouch();
            }
        }
    }
}

void UEDHeroComponent::Input_Jump(const FInputActionValue& InputActionValue)
{
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        Character->Jump();
    }
}

void UEDHeroComponent::Input_Sprint(const FInputActionValue& InputActionValue)
{
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
        {
            MovementComponent->MaxWalkSpeed = InputActionValue.Get<bool>() ? 600.0f : 400.0f;
        }
    }
}

void UEDHeroComponent::SetAbilityCameraMode(TSubclassOf<UEDCameraMode> CameraMode, const FGameplayAbilitySpecHandle& OwningSpecHandle)
{
    if (CameraMode)
    {
        AbilityCameraMode = CameraMode;
        AbilityCameraModeOwningSpecHandle = OwningSpecHandle;
    }
}

void UEDHeroComponent::ClearAbilityCameraMode(const FGameplayAbilitySpecHandle& OwningSpecHandle)
{
    if (AbilityCameraModeOwningSpecHandle == OwningSpecHandle)
    {
        AbilityCameraMode = nullptr;
        AbilityCameraModeOwningSpecHandle = FGameplayAbilitySpecHandle();
    }
}

TSubclassOf<UEDCameraMode> UEDHeroComponent::DetermineCameraMode() const
{
    return AbilityCameraMode;
}
