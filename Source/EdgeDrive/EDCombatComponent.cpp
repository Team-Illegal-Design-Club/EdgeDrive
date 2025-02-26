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
    if (bIsAttacking && !bCanCombo) return;

    ACharacter* Character = Cast<ACharacter>(GetOwner());
    
    if (!Character || !Character->GetMesh() || !Character->GetMesh()->GetAnimInstance()) return;

    if (ComboAttacks.Num() == 0) return;
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
    //if (!CurrentAttackData.Montage) return;
    //if (Character->GetMesh()->GetAnimInstance()->Montage_IsPlaying(CurrentAttackData.Montage))
    //{
    //    return;
    //}

    bHasHitThisAttack = false;
    bIsAttacking = true;
    bCanCombo = false;

    Character->PlayAnimMontage(CurrentAttackData.Montage);

    if (ComboResetTime > 0.0f && AttackTimeline)
    {
        AttackTimeline->Stop();
        AttackTimeline->PlayFromStart();
    }

    //// 현재 공격에 사용되는 부위의 메시로 라인트레이스 실행

    CurrentComboIndex++;
}


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

