// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularCharacter.h"
#include "InputActionValue.h"
#include "EDCharacter.generated.h"

/**
 * 
 */
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
protected:
	UPROPERTY(BlueprintReadOnly, Category = "State")
	ECharacterState CurrentState;
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
public:
	AEDCharacter();
protected:
	virtual void BeginPlay() override;
public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	
	void MoveInput(const FInputActionValue& Value);
	void LookInput(const FInputActionValue& Value);

	void StartSprint();
	void EndSprint();
	UPROPERTY(EditAnywhere, Category = "Movement")
	float WalkSpeed =500.f;
	UPROPERTY(EditAnywhere, Category = "Movement")
	float SprintSpeed= 1000.f;
	//virtual void Jump() override;
	UPROPERTY(BlueprintReadWrite)
	bool bIsAttacking;
	UFUNCTION(BlueprintCallable)
	void LineTrace();
protected:
	int Damage;
	void StartAttack();


};
