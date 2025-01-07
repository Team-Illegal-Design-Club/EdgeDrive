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

    // �⺻ �׼�
    void ToggleCrouch();

protected:
    virtual void BeginPlay() override;

    // �ٽ� ������Ʈ��
    //UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ED|Character")
    //TObjectPtr<UEDHealthComponent> HealthComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ED|Character")
    TObjectPtr<UEDCameraComponent> CameraComponent;
};
