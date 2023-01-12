// Fill out your copyright notice in the Description page of Project Settings.


#include "ServerRow.h"

#include "Components/Button.h"

#include "MainMenu.h"


void UServerRow::Setup(UMainMenu* Parent, uint32 Index)
{
    if (!ensure(RowButton != nullptr)) return;
    RowButton->OnClicked.AddDynamic(this, &UServerRow::OnClicked);

    ParentComponent = Parent;
    OwnIndex = Index;
}

void UServerRow::OnClicked()
{
    if (!ensure(ParentComponent != nullptr)) return;
    ParentComponent->SetSelectedIndex(OwnIndex);
}
