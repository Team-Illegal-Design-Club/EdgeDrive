#include "EDAbilityComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"

UEDAbilityComponent::UEDAbilityComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UEDAbilityComponent::SetupInput(UEnhancedInputComponent* PlayerInputComponent)
{
    if (!PlayerInputComponent) return;

    if (LockOnAction)
    {
        PlayerInputComponent->BindAction(LockOnAction, ETriggerEvent::Triggered, this, &UEDAbilityComponent::ToggleLockOn);
    }

    if (AbilityAction)
    {
        // 추가 어빌리티 입력 바인딩
    }
}

void UEDAbilityComponent::ToggleLockOn(const FInputActionValue& Value)
{
    if (!bIsLockingOn)
    {
        LockedTarget = FindNearestTarget();
        if (LockedTarget)
        {
            bIsLockingOn = true;
            if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
            {
                Character->bUseControllerRotationYaw = true;
                if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
                {
                    MovementComp->bOrientRotationToMovement = false;
                }
            }
        }
    }
    else
    {
        bIsLockingOn = false;
        LockedTarget = nullptr;
        if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
        {
            Character->bUseControllerRotationYaw = false;
            if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
            {
                MovementComp->bOrientRotationToMovement = true;
            }
        }
    }
}

void UEDAbilityComponent::UpdateLockOnCamera(float DeltaTime)
{
    if (!LockedTarget || !GetOwner()) return;

    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character || !Character->Controller) return;

    FVector Direction = LockedTarget->GetActorLocation() - GetOwner()->GetActorLocation();
    FRotator TargetRotation = Direction.Rotation();
    FRotator NewRotation = FMath::RInterpTo(
        Character->Controller->GetControlRotation(),
        TargetRotation,
        DeltaTime,
        LockOnRotationSpeed
    );

    Character->Controller->SetControlRotation(NewRotation);
}

AActor* UEDAbilityComponent::FindNearestTarget()
{
    TArray<AActor*> OverlappingActors;
    FVector Location = GetOwner()->GetActorLocation();

    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        Location,
        LockOnRadius,
        TArray<TEnumAsByte<EObjectTypeQuery>>(),
        AActor::StaticClass(),
        TArray<AActor*>(),
        OverlappingActors
    );

    AActor* BestTarget = nullptr;
    float BestScore = FLT_MAX;

    for (AActor* Actor : OverlappingActors)
    {
        if (Actor == GetOwner()) continue;
        if (!Actor->Implements<ULockOnInterface>()) continue;

        float Distance = FVector::Distance(Location, Actor->GetActorLocation());
        if (Distance < BestScore)
        {
            BestScore = Distance;
            BestTarget = Actor;
        }
    }

    return BestTarget;
}

void UEDAbilityComponent::CheckPerfectDodge()
{
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("ActiveAttack"), NearbyActors);

    if (NearbyActors.Num() > 0)
    {
        bIsPerfectDodge = true;
        EnablePerfectDodgeEffects();

        GetWorld()->GetTimerManager().SetTimer(
            PerfectDodgeTimer,
            this,
            &UEDAbilityComponent::DisablePerfectDodgeEffects,
            PerfectDodgeTimeWindow,
            false
        );
    }
}

void UEDAbilityComponent::EnablePerfectDodgeEffects()
{
    UGameplayStatics::SetGlobalTimeDilation(GetWorld(), SlowMotionScale);

    if (AfterImageTemplate)
    {
        AfterImageEffect = UNiagaraFunctionLibrary::SpawnSystemAttached(
            AfterImageTemplate,
            Cast<ACharacter>(GetOwner())->GetMesh(),
            NAME_None,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::SnapToTarget,
            true
        );
    }
}

void UEDAbilityComponent::DisablePerfectDodgeEffects()
{
    UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);

    if (AfterImageEffect)
    {
        AfterImageEffect->DeactivateImmediate();
    }

    bIsPerfectDodge = false;
}
