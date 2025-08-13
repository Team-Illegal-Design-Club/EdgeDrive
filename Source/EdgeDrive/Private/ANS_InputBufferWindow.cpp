// Fill out your copyright notice in the Description page of Project Settings.


#include "ANS_InputBufferWindow.h"
#include "../EDCombatComponent.h"
#include "Components/SkeletalMeshComponent.h"

UANS_InputBufferWindow::UANS_InputBufferWindow()
{
    BufferWindowTag = FName("DefaultBuffer");
    AllowedInputs.Add(FName("Attack"));
}

void UANS_InputBufferWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
    if (MeshComp && MeshComp->GetOwner())
    {
        if (UEDCombatComponent* CombatComp = MeshComp->GetOwner()->FindComponentByClass<UEDCombatComponent>())
        {
            CombatComp->OpenInputBufferWindow(BufferWindowTag, AllowedInputs);
        }
    }
}

void UANS_InputBufferWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (MeshComp && MeshComp->GetOwner())
    {
        if (UEDCombatComponent* CombatComp = MeshComp->GetOwner()->FindComponentByClass<UEDCombatComponent>())
        {
            CombatComp->CloseInputBufferWindow();
        }
    }
}