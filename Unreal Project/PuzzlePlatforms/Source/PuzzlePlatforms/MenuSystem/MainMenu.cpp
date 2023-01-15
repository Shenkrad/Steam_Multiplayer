// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenu.h"

#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"

#include "ServerRow.h"


UMainMenu::UMainMenu(const FObjectInitializer & ObjectInitializer)
{
    static ConstructorHelpers::FClassFinder<UServerRow> ServerRowBPClass(TEXT("/Game/MenuSystem/WBP_ServerRow"));
    if(!ensure(ServerRowBPClass.Class != nullptr)) return;

    ServerRowClass = ServerRowBPClass.Class;
}

void UMainMenu::SetServerList(TArray<FServerData> ServerData)
{
    ServerList->ClearChildren();

    uint32 i=0;
    for (const FServerData& Server : ServerData)
    {
        UServerRow* Row = CreateWidget<UServerRow>(this, ServerRowClass);
        if (!ensure(Row != nullptr)) return;

        Row->ServerName->SetText(FText::FromString(Server.Name));
        Row->HostUsername->SetText(FText::FromString(Server.HostUsername));
        FString FractionText = FString::Printf(TEXT("%d/%d"), Server.CurrentPlayers, Server.MaxPlayers);
        Row->PlayerInfo->SetText(FText::FromString(FractionText));
        Row->Setup(this, i);
        i++;

        ServerList->AddChild(Row);
    }
}

void UMainMenu::SetSelectedIndex(uint32 Index)
{
    SelectedIndex = Index;
    UpdateChildren();
}

bool UMainMenu::Initialize()
{
    bool Success = Super::Initialize();
    if (!Success) return false;

    // TODO: setup

    if (!ensure(HostButton != nullptr)) return false;
    HostButton->OnClicked.AddDynamic(this, &UMainMenu::OpenHostMenu);

    if (!ensure(JoinButton != nullptr)) return false;
    JoinButton->OnClicked.AddDynamic(this, &UMainMenu::OpenJoinMenu);

    if (!ensure(BackButton != nullptr)) return false;
    BackButton->OnClicked.AddDynamic(this, &UMainMenu::OpenMainMenu);

    if (!ensure(BackButton2 != nullptr)) return false;
    BackButton2->OnClicked.AddDynamic(this, &UMainMenu::OpenMainMenu);

    if (!ensure(ConfirmHostButton != nullptr)) return false;
    ConfirmHostButton->OnClicked.AddDynamic(this, &UMainMenu::HostServer);

    if (!ensure(JoinGameButton != nullptr)) return false;
    JoinGameButton->OnClicked.AddDynamic(this, &UMainMenu::JoinServer);

    if (!ensure(ExitButton != nullptr)) return false;
    ExitButton->OnClicked.AddDynamic(this, &UMainMenu::ExitGame);

    return true; 
}

void UMainMenu::OnLevelRemovedFromWorld(ULevel* Level, UWorld* World)
{
    Super::OnLevelRemovedFromWorld(Level, World);

    this->RemoveFromViewport();

    if (!ensure(World != nullptr)) return;

    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (!ensure(PlayerController != nullptr)) return;

    FInputModeGameOnly InputModeData;
    PlayerController->SetInputMode(InputModeData);

    PlayerController->bShowMouseCursor = false;

}

void UMainMenu::HostServer()
{
    if (MenuInterface != nullptr && HostName != nullptr)
    {
        FString ServerHostName = HostName->GetText().ToString();
        MenuInterface->Host(ServerHostName);
    }
}

void UMainMenu::JoinServer()
{   
    if (SelectedIndex.IsSet() && MenuInterface != nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("SelectedIndex %d"), SelectedIndex.GetValue());
         MenuInterface->Join(SelectedIndex.GetValue());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("SelectedIndex not set"));
    }   
}

void UMainMenu::OpenJoinMenu()
{   
    if (!ensure(MenuSwitcher != nullptr)) return;
    if (!ensure(JoinMenu != nullptr)) return;
    MenuSwitcher->SetActiveWidget(JoinMenu);

    if (MenuInterface != nullptr)
    {
        MenuInterface->RefreshServerList();
    }
}

void UMainMenu::OpenMainMenu()
{
    if (!ensure(MenuSwitcher != nullptr)) return;
    if (!ensure(MainMenu != nullptr)) return;
    MenuSwitcher->SetActiveWidget(MainMenu);
}

void UMainMenu::OpenHostMenu()
{
    if (!ensure(MenuSwitcher != nullptr)) return;
    if (!ensure(HostMenu != nullptr)) return;
    MenuSwitcher->SetActiveWidget(HostMenu);
}

void UMainMenu::ExitGame()
{
    if (MenuInterface != nullptr)
    {
        MenuInterface->ExitGame();
    }
}

void UMainMenu::UpdateChildren()
{
    for (int32 i = 0; i < ServerList->GetChildrenCount(); i++)
    {
        UServerRow* Row = Cast<UServerRow>(ServerList->GetChildAt(i));
        if (Row != nullptr)
        {
            Row->Selected = (SelectedIndex.IsSet() && SelectedIndex.GetValue() == i);
            Row->OnSelectedChanged();
        }
        
    }
}
