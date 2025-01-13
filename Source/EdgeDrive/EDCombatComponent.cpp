#include "EDCombatComponent.h"
#include "GameFramework/Character.h"
#include "EnhancedInputComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
UEDCombatComponent::UEDCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UEDCombatComponent::SetupInput(UEnhancedInputComponent* PlayerInputComponent)
{
    if (!PlayerInputComponent) return;

    if (AttackAction)
    {
        PlayerInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &UEDCombatComponent::StartAttack);
    }
}

void UEDCombatComponent::StartAttack()
{
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character || !Character->GetMesh() || !Character->GetMesh()->GetAnimInstance()) return;
    if (bIsAttacking && !bCanCombo) return;
    if (ComboAttackMontages.Num() == 0) return;

    if (CurrentComboIndex >= ComboAttackMontages.Num())
    {
        CurrentComboIndex = 0;
    }

    UAnimMontage* CurrentAttackMontage = ComboAttackMontages[CurrentComboIndex];
    if (!CurrentAttackMontage) return;

    if (Character->GetMesh()->GetAnimInstance()->Montage_IsPlaying(CurrentAttackMontage))
    {
        return;
    }

    bIsAttacking = true;
    bCanCombo = false;

    Character->PlayAnimMontage(CurrentAttackMontage);

    if (ComboResetTime > 0.0f)
    {
        GetWorld()->GetTimerManager().ClearTimer(ComboResetTimer);
        GetWorld()->GetTimerManager().SetTimer(
            ComboResetTimer,
            this,
            &UEDCombatComponent::ResetCombo,
            ComboResetTime,
            false
        );
    }

    if (GloveMesh &&
        GloveMesh->DoesSocketExist(FName("Start")) &&
        GloveMesh->DoesSocketExist(FName("End")))
    {
        LineTrace();
    }

    CurrentComboIndex++;
}

void UEDCombatComponent::LineTrace()
{
    FVector StartLocation = GloveMesh->GetSocketLocation(FName("Start"));
    FVector EndLocation = GloveMesh->GetSocketLocation(FName("End"));
    float CurrentRadius = GetCurrentComboRadius();

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    TArray<FHitResult> HitResults;
    DrawDebugSphere(
        GetWorld(),
        StartLocation,
        CurrentRadius,
        32,  // 세그먼트 수
        FColor::Red,
        false,  // 지속적
        0.2f,   // 지속 시간
        0,      // 깊이 우선순위
        1.0f    // 두께
    );

    DrawDebugSphere(
        GetWorld(),
        EndLocation,
        CurrentRadius,
        32,
        FColor::Blue,
        false,
        0.2f,
        0,
        1.0f
    );

    // 두 구체 사이의 선 그리기
    DrawDebugLine(
        GetWorld(),
        StartLocation,
        EndLocation,
        FColor::Green,
        false,
        0.2f,
        0,
        2.0f
    );

    bool bHit = GetWorld()->SweepMultiByChannel(
        HitResults,
        StartLocation,
        EndLocation,
        FQuat::Identity,
        ECC_Visibility,
        FCollisionShape::MakeSphere(CurrentRadius),
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
                FColor::Yellow,
                false,
                0.2f,
                0
            );
            float CurrentDamage = GetCurrentComboDamage();
            UGameplayStatics::ApplyDamage(ActorHit, CurrentDamage, GetOwner()->GetInstigatorController(), GetOwner(), nullptr);
            PlayHitEffect(Hit.Location);
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

void UEDCombatComponent::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (bIsAttacking)
    {
        bIsAttacking = false;
        bCanCombo = false;
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
    GetWorld()->GetTimerManager().ClearTimer(ComboResetTimer);
}

float UEDCombatComponent::GetCurrentComboDamage() const
{
    if (CurrentComboIndex > 0 && CurrentComboIndex <= ComboDamages.Num())
    {
        return ComboDamages[CurrentComboIndex - 1];
    }
    return Damage;
}

float UEDCombatComponent::GetCurrentComboRadius() const
{
    if (CurrentComboIndex > 0 && CurrentComboIndex <= ComboRadiuses.Num())
    {
        return ComboRadiuses[CurrentComboIndex - 1];
    }
    return 20.0f;
}
