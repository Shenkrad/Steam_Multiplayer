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

void UMainMenu::SetServerList(TArray<FString> ServerNames)
{
    ServerList->ClearChildren();

    for (const FString& ServerName : ServerNames)
    {
        UServerRow* Row = CreateWidget<UServerRow>(this, ServerRowClass);
        if (!ensure(Row != nullptr)) return;

        Row->ServerName->SetText(FText::FromString(ServerName));

        ServerList->AddChild(Row);
    }
}

bool UMainMenu::Initialize()
{
    bool Success = Super::Initialize();
    if (!Success) return false;

    // TODO: setup

    if (!ensure(HostButton != nullptr)) return false;
    HostButton->OnClicked.AddDynamic(this, &UMainMenu::HostServer);

    if (!ensure(JoinButton != nullptr)) return false;
    JoinButton->OnClicked.AddDynamic(this, &UMainMenu::OpenJoinMenu);

    if (!ensure(BackButton != nullptr)) return false;
    BackButton->OnClicked.AddDynamic(this, &UMainMenu::OpenMainMenu);

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
    if (MenuInterface != nullptr)
    {
        MenuInterface->Host();
    }
}

void UMainMenu::JoinServer()
{
    if (MenuInterface != nullptr)
    {
        // if (!ensure(IPAddressField != nullptr)) return;
        // const FString& Address = IPAddressField->GetText().ToString();
        MenuInterface->Join("");
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

void UMainMenu::ExitGame()
{
    if (MenuInterface != nullptr)
    {
        MenuInterface->ExitGame();
    }
}
