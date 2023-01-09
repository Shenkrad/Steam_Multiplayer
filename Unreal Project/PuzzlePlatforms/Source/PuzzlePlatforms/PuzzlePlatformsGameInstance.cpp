// Fill out your copyright notice in the Description page of Project Settings.


#include "PuzzlePlatformsGameInstance.h"

#include "UObject/ConstructorHelpers.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

#include "MenuSystem/MainMenu.h"
#include "MenuSystem/MenuWidget.h"


const static FName SESSION_NAME = TEXT("My Session Game");

UPuzzlePlatformsGameInstance::UPuzzlePlatformsGameInstance(const FObjectInitializer & ObjectInitializer)
{
    static ConstructorHelpers::FClassFinder<UUserWidget> MainMenuBPClass(TEXT("/Game/MenuSystem/WBP_MainMenu"));
    if(!ensure(MainMenuBPClass.Class != nullptr)) return;

    MenuClass = MainMenuBPClass.Class;

    static ConstructorHelpers::FClassFinder<UUserWidget> InGameMenuBPClass(TEXT("/Game/MenuSystem/WBP_InGameMenu"));
    if(!ensure(InGameMenuBPClass.Class != nullptr)) return;

    InGameMenuClass = InGameMenuBPClass.Class;
}

void UPuzzlePlatformsGameInstance::Init()
{
    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
    if (OnlineSubsystem != nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("Found subsystem: %s"), *OnlineSubsystem->GetSubsystemName().ToString());
        SessionInterface = OnlineSubsystem->GetSessionInterface();

        if (SessionInterface.IsValid())
        {
            SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UPuzzlePlatformsGameInstance::OnCreateSessionComplete);
            SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UPuzzlePlatformsGameInstance::OnDestroySessionComplete);
            SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UPuzzlePlatformsGameInstance::OnFindSessionsComplete);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Found no subsystem"));
    }
}

void UPuzzlePlatformsGameInstance::LoadMenu()
{
    if (!ensure(MenuClass != nullptr)) return;

    Menu = CreateWidget<UMainMenu>(this, MenuClass);
    if (!ensure(Menu != nullptr)) return;

    Menu->Setup();
    Menu->SetMenuInterface(this);

}

void UPuzzlePlatformsGameInstance::InGameLoadMenu()
{
    if (!ensure(InGameMenuClass != nullptr)) return;

    UMenuWidget* InGameMenu = CreateWidget<UMenuWidget>(this, InGameMenuClass);
    if (!ensure(InGameMenu != nullptr)) return;

    InGameMenu->Setup();
    InGameMenu->SetMenuInterface(this);
}

void UPuzzlePlatformsGameInstance::Host()
{
    if (SessionInterface.IsValid())
    {
        FNamedOnlineSession* ExistingSession = SessionInterface->GetNamedSession(SESSION_NAME);
        if (ExistingSession != nullptr)
        {
            SessionInterface->DestroySession(SESSION_NAME);
        }
        else
        {
            CreateSession();
        }  
    }
}

void UPuzzlePlatformsGameInstance::Join(const FString& Address)
{

    if (Menu != nullptr)
    {
        Menu->SetServerList({ "Test1", "Test2" });
    }

    // UEngine* Engine = GetEngine();
    // if (!ensure(Engine != nullptr)) return;

    // Engine->AddOnScreenDebugMessage(0, 5, FColor::Green, FString::Printf(TEXT("Joining %s..."), *Address));

    // APlayerController* PlayerController = GetFirstLocalPlayerController();
    // if (!ensure(PlayerController != nullptr)) return;

    // PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);

}

void UPuzzlePlatformsGameInstance::RefreshServerList()
{
    SessionSearch = MakeShareable(new FOnlineSessionSearch());

    if (SessionSearch.IsValid())
    {
        SessionSearch->bIsLanQuery = true;
        UE_LOG(LogTemp, Warning, TEXT("Starting finding game sessions"));
        SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());                
    }
}

void UPuzzlePlatformsGameInstance::LoadMainMenu()
{
    APlayerController* PlayerController = GetFirstLocalPlayerController();
    if (!ensure(PlayerController != nullptr)) return;

    PlayerController->ClientTravel("/Game/MenuSystem/MainMenu", ETravelType::TRAVEL_Absolute);
}

void UPuzzlePlatformsGameInstance::ExitGame()
{
    APlayerController* PlayerController = GetFirstLocalPlayerController();
    if (!ensure(PlayerController != nullptr)) return;

    PlayerController->ConsoleCommand("Quit");
}

void UPuzzlePlatformsGameInstance::OnCreateSessionComplete(FName SessionName, bool Success)
{
    if (!Success)
    {
        UE_LOG(LogTemp, Warning, TEXT("Could not create session"));
        return;
    }
    
    UEngine* Engine = GetEngine();
    if (!ensure(Engine != nullptr)) return;

    Engine->AddOnScreenDebugMessage(0, 2, FColor::Green, TEXT("Hosting"));

    UWorld* World = GetWorld();
    if (!ensure(World != nullptr)) return;

    World->ServerTravel("/Game/ThirdPerson/Maps/ThirdPersonMap?listen");
}

void UPuzzlePlatformsGameInstance::OnDestroySessionComplete(FName SessionName, bool Success)
{
    if (Success)
    {
        CreateSession();
    } 
}

void UPuzzlePlatformsGameInstance::OnFindSessionsComplete(bool Success)
{
    if (Success && SessionSearch.IsValid() && Menu != nullptr)
    {   
        TArray<FString> ServerNames;
        for (const FOnlineSessionSearchResult& SearchResult: SessionSearch->SearchResults)
        {
           ServerNames.Add(SearchResult.GetSessionIdStr());
        }
        Menu->SetServerList(ServerNames);
        
        UE_LOG(LogTemp, Warning, TEXT("Finished finding game sessions"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No game sessions found"));
    }
}

void UPuzzlePlatformsGameInstance::CreateSession()
{
    if (SessionInterface.IsValid())
    {
        FOnlineSessionSettings SessionSettings;
        SessionSettings.bIsLANMatch = true;
        SessionSettings.NumPublicConnections = 2;
        SessionSettings.bShouldAdvertise = true;

        SessionInterface->CreateSession(0, SESSION_NAME, SessionSettings);
    }
}
