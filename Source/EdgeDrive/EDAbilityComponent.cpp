#include "EDAbilityComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
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

void UEDAbilityComponent::ToggleLockOn()
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

    // 타겟의 위치를 가져옴
    FVector TargetLocation = LockedTarget->GetActorLocation();

    // 타겟의 캡슐 컴포넌트를 가져와서 높이 조절
    if (UCapsuleComponent* TargetCapsule = Cast<UCapsuleComponent>(LockedTarget->GetRootComponent()))
    {
        // 타겟의 높이를 절반으로 조절
        TargetLocation.Z -= TargetCapsule->GetScaledCapsuleHalfHeight();
    }

    FVector Direction = TargetLocation - GetOwner()->GetActorLocation();
    FRotator TargetRotation = Direction.Rotation();

    // 피치(상하) 회전 각도 제한
    float ClampedPitch = FMath::ClampAngle(TargetRotation.Pitch, -30.0f, 30.0f);
    TargetRotation.Pitch = ClampedPitch;

    FRotator NewRotation = FMath::RInterpTo(
        Character->Controller->GetControlRotation(),
        TargetRotation,
        DeltaTime,
        LockOnRotationSpeed
    );

    Character->Controller->SetControlRotation(NewRotation);
}


//AActor* UEDAbilityComponent::FindNearestTarget()
//{
//    TArray<AActor*> OverlappingActors;
//    FVector Location = GetOwner()->GetActorLocation();
//
//    UKismetSystemLibrary::SphereOverlapActors(
//        GetWorld(),
//        Location,
//        LockOnRadius,
//        TArray<TEnumAsByte<EObjectTypeQuery>>(),
//        AActor::StaticClass(),
//        TArray<AActor*>(),
//        OverlappingActors
//    );
//
//    AActor* BestTarget = nullptr;
//    float BestScore = FLT_MAX;
//
//    for (AActor* Actor : OverlappingActors)
//    {
//        if (Actor == GetOwner()) continue;
//        if (!Actor->Implements<ULockOnInterface>()) continue;
//
//        float Distance = FVector::Distance(Location, Actor->GetActorLocation());
//        if (Distance < BestScore)
//        {
//            BestScore = Distance;
//            BestTarget = Actor;
//        }
//    }
//
//    return BestTarget;
//}
AActor* UEDAbilityComponent::FindNearestTarget()
{
    TArray<AActor*> OverlappingActors;
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character || !Character->Controller) return nullptr;

    APlayerController* PC = Cast<APlayerController>(Character->Controller);
    if (!PC) return nullptr;

    // 화면 중앙 좌표 구하기
    int32 ViewportSizeX, ViewportSizeY;
    PC->GetViewportSize(ViewportSizeX, ViewportSizeY);
    FVector2D ScreenCenter(ViewportSizeX * 0.5f, ViewportSizeY * 0.5f);

    // 캐릭터 주변의 액터들 찾기
    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        Character->GetActorLocation(),
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

        // 화면상의 위치 계산
        FVector2D ScreenLocation;
        if (PC->ProjectWorldLocationToScreen(Actor->GetActorLocation(), ScreenLocation))
        {
            // 화면 중앙과의 거리 계산
            float ScreenDistance = FVector2D::Distance(ScreenCenter, ScreenLocation);

            // 가장 가까운 타겟 선택
            if (ScreenDistance < BestScore)
            {
                // 시야 체크
                FHitResult HitResult;
                FCollisionQueryParams QueryParams;
                QueryParams.AddIgnoredActor(GetOwner());

                if (!GetWorld()->LineTraceSingleByChannel(
                    HitResult,
                    PC->PlayerCameraManager->GetCameraLocation(),
                    Actor->GetActorLocation(),
                    ECC_Visibility,
                    QueryParams))
                {
                    BestScore = ScreenDistance;
                    BestTarget = Actor;
                }
            }
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