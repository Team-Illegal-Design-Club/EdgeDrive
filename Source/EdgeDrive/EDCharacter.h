// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularCharacter.h"
#include "InputActionValue.h"
#include "EDCharacter.generated.h"

/**
 * 
 */
class UNiagaraSystem;
class UNiagaraComponent;
class UCamaraComponent;
class UAnimSequence;
UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	Idle,
	Walking,
	Dodging,
	Sprinting,
	Parring,
	Falling
};
UINTERFACE(MinimalAPI)
class ULockOnInterface : public UInterface
{
	GENERATED_BODY()
};

class ILockOnInterface
{
	GENERATED_BODY()
public:
	// 필요한 인터페이스 함수 선언
};
UCLASS()
class EDGEDRIVE_API AEDCharacter : public AModularCharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* SpringArm;
	UPROPERTY(VisibleAnywhere , meta = (AllowPrivateAccess = "ture"))
	class UCameraComponent* Camera;
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* GloveMesh;
	UPROPERTY(EditAnywhere)
	class UAnimSequence* AttackAnim;
	FVector2D CurrentMovementInput;
protected:
	UPROPERTY(BlueprintReadOnly, Category = "State")
	ECharacterState CurrentState;




	UPROPERTY(EditAnywhere, Category = "Animation")
	class UAnimMontage* ForwardDodgeMontage;

	UPROPERTY(EditAnywhere, Category = "Animation")
	class UAnimMontage* BackwardDodgeMontage;

	UPROPERTY(EditAnywhere, Category = "Animation")
	class UAnimMontage* LeftDodgeMontage;

	UPROPERTY(EditAnywhere, Category = "Animation")
	class UAnimMontage* RightDodgeMontage;



	UPROPERTY(EditAnywhere, Category ="Lock On")
	float LockOnRadius = 10000.f;
	UPROPERTY(EditAnywhere, Category = "Lock On")
	float LockOnRotationSpeed = 10.f;
	UPROPERTY(EditAnywhere, Category = "Lock On")
	class UInputAction* LockOnAction;



	UPROPERTY()
	AActor* LockedTarget;

	UPROPERTY()
	bool bIsLockingOn;



	UPROPERTY(EditAnywhere, Category = "Perfect Dodge")
	float PerfectDodgeTimeWindow = 0.2f;

	UPROPERTY(EditAnywhere, Category = "Perfect Dodge")
	float SlowMotionScale = 0.2f;

	UPROPERTY()
	bool bIsPerfectDodge;

	UPROPERTY()
	class UNiagaraComponent* AfterImageEffect;

	UPROPERTY(EditAnywhere, Category = "Perfect Dodge")
	class UNiagaraSystem* AfterImageTemplate;

	UPROPERTY()
	FTimerHandle PerfectDodgeTimer;


	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
	class UInputMappingContext* InputMapping;
	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
	class UInputAction* MoveAction;
	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
	class UInputAction* AttackAction;
	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
	class UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
	class UInputAction* DodgeAction;
	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
	class UInputAction* SprintAction;
	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
	class UInputAction* ParryAction; 
	UPROPERTY(EditAnywhere, Category = "EnhancedInput")
	class UInputAction* AbilityAction;
	// Combo System
	UPROPERTY(EditAnywhere, Category = "Combat")
	TArray<UAnimMontage*> ComboAttackMontages;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ComboTimeWindow = 1.5f;

	int32 CurrentComboIndex = 0;
	FTimerHandle ComboResetTimer;
	bool bCanCombo = false;

	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void ResetCombo();

public:
	AEDCharacter();

	// Function
	void ToggleLockOn(const FInputActionValue& Value);
	void UpdateLockOnCamera(float DeltaTime);
	AActor* FindNearestTarget();
protected:
	virtual void BeginPlay() override;
public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	void PlayHitEffect(const FVector& HitLocation);

	void MoveInput(const FInputActionValue& Value);
	void LookInput(const FInputActionValue& Value);

	void StartSprint();
	void EndSprint();
	void Dodge(const FInputActionValue& Value);
	void OnDodgeEnd();
	UPROPERTY(EditAnywhere, Category = "Movement")
	float WalkSpeed =500.f;
	UPROPERTY(EditAnywhere, Category = "Movement")
	float SprintSpeed= 1000.f;



	UPROPERTY(EditAnywhere, Category = "Movement")
	float DodgeDistance = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float DodgeCooldown = 0.5f;

	
	bool bCanDodge = true;
	FTimerHandle DodgeCooldownTimer;


	//virtual void Jump() override;
	UPROPERTY(BlueprintReadWrite)
	bool bIsAttacking;
	UFUNCTION(BlueprintCallable)
	void LineTrace();
protected:
	UPROPERTY(EditAnywhere)
	float Damage;
	void StartAttack();

		void CheckPerfectDodge();
		void EnablePerfectDodgeEffects();
		void DisablePerfectDodgeEffects();


};
