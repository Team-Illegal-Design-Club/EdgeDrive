#pragma once

#include "CoreMinimal.h"
#include "ModularCharacter.h"
#include "EDCharacter.generated.h"

//class UEDHealthComponent;
class UEDCameraComponent;

UCLASS(Config = Game)
class EDGEDRIVE_API AEDCharacter : public AModularCharacter
{
    GENERATED_BODY()

public:
    AEDCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    // 기본 액션
    void ToggleCrouch();

protected:
    virtual void BeginPlay() override;

    // 핵심 컴포넌트들
    //UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ED|Character")
    //TObjectPtr<UEDHealthComponent> HealthComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ED|Character")
    TObjectPtr<UEDCameraComponent> CameraComponent;
};
