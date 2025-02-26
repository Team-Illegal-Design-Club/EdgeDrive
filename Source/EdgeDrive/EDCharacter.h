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




 
public:
    UPROPERTY(EditAnywhere, Category = "EnhancedInput")
    class UInputMappingContext* InputMapping;
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    class UEDCombatComponent* CombatComponent;
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    class UEDMovementComponent* MovementComponent;
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    class UEDAbilityComponent* AbilityComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    class UEDHeroComponent* HeroComponent;
    UFUNCTION(BlueprintCallable)
    UEDMovementComponent* GetEDMovementComponent();

    UFUNCTION(BlueprintCallable)
    UEDCombatComponent* GetEDCombatComponent();


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    class USpringArmComponent* SpringArm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    class UCameraComponent* Camera;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    UStaticMeshComponent* LeftHandMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    UStaticMeshComponent* RightHandMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    UStaticMeshComponent* LeftFootMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    UStaticMeshComponent* RightFootMesh;

public:
    AEDCharacter();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};