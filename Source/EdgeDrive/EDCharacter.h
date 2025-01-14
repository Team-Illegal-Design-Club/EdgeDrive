#pragma once

#include "CoreMinimal.h"
#include "ModularCharacter.h"
#include "InputActionValue.h"
#include "EDCharacter.generated.h"
class USpringArmComponent;
class UCameraComponent;
class UCharacterMovementComponent;
class UStaticMeshComponent;
UCLASS()
class EDGEDRIVE_API AEDCharacter : public AModularCharacter
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere)
    class UEDHeroComponent* HeroComponent;
    UPROPERTY(VisibleAnywhere)
    class UEDMovementComponent* MovementComponent;


    UPROPERTY(VisibleAnywhere)
    class UEDCombatComponent* CombatComponent;

    UPROPERTY(VisibleAnywhere)
    class UEDAbilityComponent* AbilityComponent;

    UPROPERTY(EditAnywhere, Category = "EnhancedInput")
    class UInputMappingContext* InputMapping;
public:

    UFUNCTION(BlueprintCallable)
    UEDMovementComponent* GetEDMovementComponent();

    UFUNCTION(BlueprintCallable)
    UEDCombatComponent* GetEDCombatComponent();
    UPROPERTY(EditAnywhere, Category = "Movement")
    float WalkSpeed = 500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    class USpringArmComponent* SpringArm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    class UCameraComponent* Camera;

    UPROPERTY(VisibleAnywhere)
    class UStaticMeshComponent* GloveMesh;
public:
    AEDCharacter();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
