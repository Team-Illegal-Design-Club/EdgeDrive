// Fill out your copyright notice in the Description page of Project Settings.


#include "EDCharacter.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h" 
#include "NiagaraSystem.h"
#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h"

#include "EDHealthComponent.h"

AEDCharacter::AEDCharacter()
{
	// Enable tick by default for smooth movement
	PrimaryActorTick.bCanEverTick = true;

	// Set default movement values
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	GetCharacterMovement()->bOrientRotationToMovement = false;

	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 300.0f;
	SpringArm->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(SpringArm);
	Camera->bUsePawnControlRotation = false;
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->bOrientRotationToMovement = true;
		MovementComponent->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
		MovementComponent->JumpZVelocity = 500.f;
		MovementComponent->AirControl = 0.35f;
		MovementComponent->MaxWalkSpeed = 500.f;
		MovementComponent->MinAnalogWalkSpeed = 20.f;
		MovementComponent->BrakingDecelerationWalking = 2000.f;
	}
	// Bind the montage end delegate
	if (USkeletalMeshComponent* SkeletalMesh = GetMesh())
	{
		if (UAnimInstance* AnimInstance = SkeletalMesh->GetAnimInstance())
		{
			AnimInstance->OnMontageEnded.AddDynamic(this, &AEDCharacter::OnMontageEnded);
		}
	}
	GloveMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Sword Mesh"));
	GloveMesh->SetupAttachment(GetMesh(), FName("GloveSocket"));
}

void AEDCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Verify input mapping and actions are set
	if (!InputMapping || !MoveAction)
	{
		UE_LOG(LogTemp, Warning, TEXT("Input mapping or actions not set in EDCharacter"));
	}
	if (USkeletalMeshComponent* SkeletalMesh = GetMesh())
	{
		if (UAnimInstance* AnimInstance = SkeletalMesh->GetAnimInstance())
		{
			AnimInstance->OnMontageEnded.AddDynamic(this, &AEDCharacter::OnMontageEnded);
		}
	}

}

void AEDCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsLockingOn && LockedTarget)
	{
		UpdateLockOnCamera(DeltaTime);
	}
	// Update character state
	if (GetCharacterMovement()->IsFalling())
	{
		CurrentState = ECharacterState::Falling;
	}
	else if (GetCharacterMovement()->Velocity.SizeSquared() > 0)
	{
		CurrentState = ECharacterState::Walking;
	}
	else
	{
		CurrentState = ECharacterState::Idle;
	}
}

void AEDCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{

		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputMapping, 0);

		}
	}

	if (UEnhancedInputComponent* Input = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AEDCharacter::MoveInput);
		Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &AEDCharacter::LookInput);
		Input->BindAction(SprintAction, ETriggerEvent::Triggered, this, &AEDCharacter::StartSprint);
		Input->BindAction(SprintAction, ETriggerEvent::Completed, this, &AEDCharacter::EndSprint);
		Input->BindAction(AttackAction, ETriggerEvent::Triggered, this, &AEDCharacter::StartAttack);
		Input->BindAction(DodgeAction, ETriggerEvent::Triggered, this, &AEDCharacter::Dodge);
		Input->BindAction(LockOnAction, ETriggerEvent::Triggered, this, &AEDCharacter::ToggleLockOn);
		//	Input->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AEDCharacter::Jump);

	}
}
void AEDCharacter::LookInput(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(-LookAxisVector.Y);
}
void AEDCharacter::StartSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}
void AEDCharacter::EndSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

}

void AEDCharacter::StartAttack()
{
	if (bIsAttacking && !bCanCombo) return;

	if (CurrentComboIndex >= ComboAttackMontages.Num())
	{
		CurrentComboIndex = 0;
	}

	if (UAnimMontage* CurrentAttackMontage = ComboAttackMontages[CurrentComboIndex])
	{
		bIsAttacking = true;
		bCanCombo = false;
		PlayAnimMontage(CurrentAttackMontage);

		GetWorldTimerManager().ClearTimer(ComboResetTimer);
		GetWorldTimerManager().SetTimer(ComboResetTimer, this, &AEDCharacter::ResetCombo, ComboTimeWindow, false);

		CurrentComboIndex++;
	}
}
void AEDCharacter::LineTrace()
{
	FVector StartLocation = GloveMesh->GetSocketLocation(FName("Start"));
	FVector EndLocation = GloveMesh->GetSocketLocation(FName("End"));

	FHitResult HitResult;
	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(this);

	// Debug Lines
	DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 1, 0, 1);

	// Linetrace
	if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, TraceParams))
	{
		if (AActor* ActorHit = HitResult.GetActor())
		{
			// ApplyDamage ���
			UGameplayStatics::ApplyDamage(
				ActorHit,           // �������� ���� ����
				Damage,             // ������ ��
				GetController(),    // �������� �� ��Ʈ�ѷ�
				this,              // �������� �� ����
				nullptr            // ������ Ÿ��
			);

			// ��Ʈ ����Ʈ ��� (���û���)
			PlayHitEffect(HitResult.Location);
		}
	}
}

// ������: ��Ʈ ����Ʈ�� ���� �Լ�
void AEDCharacter::PlayHitEffect(const FVector& HitLocation)
{
	// ��Ʈ ����Ʈ ����
	// ��ƼŬ, ���� ���� ���
}

void AEDCharacter::MoveInput(const FInputActionValue& Value)
{
	/*GEngine->AddOnScreenDebugMessage(-1,1.f, FColor::Red, "Pressed input action ");*/
	CurrentMovementInput = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, CurrentMovementInput.Y);
		AddMovementInput(RightDirection, CurrentMovementInput.X);
	}
	CurrentMovementInput = Value.Get<FVector2D>();
}

void AEDCharacter::ToggleLockOn(const FInputActionValue& Value)
{
	if (!bIsLockingOn)
	{
		LockedTarget = FindNearestTarget();
		if (LockedTarget)
		{
			bIsLockingOn = true;
			// ���� �� ī�޶� ��Ʈ�� ����
			bUseControllerRotationYaw = true;
			GetCharacterMovement()->bOrientRotationToMovement = false;
		}
	}
	else
	{
		bIsLockingOn = false;
		LockedTarget = nullptr;
		// ���� ���� �� ���� �������� ����
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
	}
}

void AEDCharacter::UpdateLockOnCamera(float DeltaTime)
{
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, "Pressed input action ");
	if (!LockedTarget) return;

	// Ÿ�� �������� ȸ��
	FVector Direction = LockedTarget->GetActorLocation() - GetActorLocation();
	FRotator TargetRotation = Direction.Rotation();
	FRotator NewRotation = FMath::RInterpTo(
		GetController()->GetControlRotation(),
		TargetRotation,
		DeltaTime,
		LockOnRotationSpeed
	);

	GetController()->SetControlRotation(NewRotation);
}
AActor* AEDCharacter::FindNearestTarget()
{
	TArray<AActor*> OverlappingActors;
	FVector Location = GetActorLocation();

	// �ֺ� ���� �˻�
	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		Location,
		LockOnRadius,
		TArray<TEnumAsByte<EObjectTypeQuery>>(),
		AActor::StaticClass(),
		TArray<AActor*>(),
		OverlappingActors
	);

	// ī�޶� �þ߰� ���
	FVector CameraForward = Camera->GetForwardVector();
	AActor* BestTarget = nullptr;
	float BestScore = FLT_MAX;

	for (AActor* Actor : OverlappingActors)
	{
		// �ڽ� ����
		if (Actor == this) continue;

		// �������̽� ���� ���� Ȯ��
		if (!Actor->Implements<ULockOnInterface>()) continue;

		FVector DirectionToTarget = (Actor->GetActorLocation() - Location).GetSafeNormal();

		// �þ߰� ���
		float ViewDotProduct = FVector::DotProduct(CameraForward, DirectionToTarget);

		// �þ߰��� �ʹ� ū ��� ���� (�� 90�� �̻�)
		if (ViewDotProduct < 0.0f) continue;

		float Distance = FVector::Distance(Location, Actor->GetActorLocation());
		float Score = Distance * (1.0f - ViewDotProduct);

		if (Score < BestScore)
		{
			BestScore = Score;
			BestTarget = Actor;
		}
	}

	return BestTarget;
}
void AEDCharacter::Dodge(const FInputActionValue& Value)
{
	if (!bCanDodge || GetCharacterMovement()->IsFalling() || CurrentState == ECharacterState::Dodging)
		return;
	CheckPerfectDodge();
	float ForwardInput = CurrentMovementInput.Y;
	float RightInput = CurrentMovementInput.X;

	UAnimMontage* SelectedMontage = nullptr;
	FVector DodgeDirection = FVector::ZeroVector;

	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->bOrientRotationToMovement = false;
		MovementComp->bAllowPhysicsRotationDuringAnimRootMotion = false;
	}

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	if (bIsLockingOn)
	{

		// 8���� ȸ�� ���� ���
		DodgeDirection = ForwardDirection * ForwardInput + RightDirection * RightInput;

		if (!DodgeDirection.IsNearlyZero())
		{
			DodgeDirection.Normalize();

			// ���⿡ ���� ��Ÿ�� ����
			float DotForward = FVector::DotProduct(DodgeDirection, ForwardDirection);
			float DotRight = FVector::DotProduct(DodgeDirection, RightDirection);

			if (FMath::Abs(DotForward) > FMath::Abs(DotRight))
			{
				SelectedMontage = (DotForward > 0) ? ForwardDodgeMontage : BackwardDodgeMontage;
			}
			else
			{
				SelectedMontage = (DotRight > 0) ? RightDodgeMontage : LeftDodgeMontage;
			}
		}
		else
		{
			DodgeDirection = -GetActorForwardVector();
			SelectedMontage = BackwardDodgeMontage;
		}
	}
	else
	{
		// ������ �ƴ� ���� ��/�ڷθ� ������
		if (!CurrentMovementInput.IsZero())
		{
			DodgeDirection = GetActorForwardVector();
			SelectedMontage = ForwardDodgeMontage;
		}
		else
		{
			DodgeDirection = -GetActorForwardVector();
			SelectedMontage = BackwardDodgeMontage;
		}
	}
	if (SelectedMontage)
	{
		float MontageLength = SelectedMontage->GetPlayLength();
		float DodgeSpeed = DodgeDistance / MontageLength;

		PlayAnimMontage(SelectedMontage);
		LaunchCharacter(DodgeDirection * DodgeSpeed, true, false);
		CurrentState = ECharacterState::Dodging;

		bCanDodge = false;
		GetWorldTimerManager().SetTimer(
			DodgeCooldownTimer,
			[this]() {
			bCanDodge = true;
			OnDodgeEnd();
		},
			DodgeCooldown,
			false
		);
	}
}


void AEDCharacter::OnDodgeEnd()
{
	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		// Restore rotation settings based on lock-on state
		if (bIsLockingOn)
		{
			MovementComp->bOrientRotationToMovement = false;
			bUseControllerRotationYaw = true;
		}
		else
		{
			MovementComp->bOrientRotationToMovement = true;
			bUseControllerRotationYaw = false;
		}
		MovementComp->bAllowPhysicsRotationDuringAnimRootMotion = true;
	}

	CurrentState = ECharacterState::Idle;
}
void AEDCharacter::CheckPerfectDodge()
{
	// �ֺ� ���� üũ
	TArray<AActor*> NearbyActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("ActiveAttack"), NearbyActors);

	if (NearbyActors.Num() > 0)
	{
		bIsPerfectDodge = true;
		EnablePerfectDodgeEffects();

		GetWorldTimerManager().SetTimer(
			PerfectDodgeTimer,
			this,
			&AEDCharacter::DisablePerfectDodgeEffects,
			PerfectDodgeTimeWindow,
			false
		);
	}
}

void AEDCharacter::EnablePerfectDodgeEffects()
{
	// ���ο� ��� ȿ��
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), SlowMotionScale);

	// �ܻ� ȿ�� ����
	if (AfterImageTemplate)
	{
		AfterImageEffect = UNiagaraFunctionLibrary::SpawnSystemAttached(
			AfterImageTemplate,
			GetMesh(),
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			true
		);
	}

	// ī�޶� FOV ȿ��
	if (Camera)
	{
		Camera->FieldOfView = 110.0f;
	}
}

void AEDCharacter::DisablePerfectDodgeEffects()
{
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);

	if (AfterImageEffect)
	{
		AfterImageEffect->DeactivateImmediate();
	}

	if (Camera)
	{
		Camera->FieldOfView = 90.0f;
	}

	bIsPerfectDodge = false;
}
void AEDCharacter::ResetCombo()
{
	CurrentComboIndex = 0;
	bIsAttacking = false;
	bCanCombo = false;
}

void AEDCharacter::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!bInterrupted && CurrentComboIndex > 0)
	{
		bCanCombo = true;
	}
	else
	{
		ResetCombo();
	}
}
