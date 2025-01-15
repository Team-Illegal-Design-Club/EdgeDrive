#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EDHeroComponent.generated.h"

UCLASS()
class EDGEDRIVE_API UEDHeroComponent : public UActorComponent
{
    GENERATED_BODY()

protected:
    UPROPERTY(VisibleAnywhere)
    class USpringArmComponent* SpringArm;

    UPROPERTY(VisibleAnywhere)
    class UCameraComponent* Camera;

    UPROPERTY(EditAnywhere, Category = "EnhancedInput")
    class UInputMappingContext* InputMapping;

    UPROPERTY(EditAnywhere, Category = "EnhancedInput")
    class UInputAction* LookAction;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool _2DModeEnabled = false;
    UEDHeroComponent();
    UFUNCTION()
    void LookInput(const FInputActionValue& Value);

    // 기존 InitializeComponent를 다른 이름으로 변경
    void SetupComponent(USpringArmComponent* InSpringArm, UCameraComponent* InCamera);



    void SetupInput(class UEnhancedInputComponent* PlayerInputComponent);

};
