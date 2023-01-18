// Fill out your copyright notice in the Description page of Project Settings.


#include "PuzzlePlatformsGameInstance.h"

#include "UObject/ConstructorHelpers.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

#include "MenuSystem/MainMenu.h"
#include "MenuSystem/MenuWidget.h"


const static FName SESSION_NAME = NAME_GameSession;
const static FName SERVER_NAME_SETTINGS_KEY = TEXT("ServerName");

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
            SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UPuzzlePlatformsGameInstance::OnJoinSessionComplete);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Found no subsystem"));
    }

    if (GEngine != nullptr)
    {
        GEngine->OnNetworkFailure().AddUObject(this, &UPuzzlePlatformsGameInstance::OnNetworkFailure);
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

void UPuzzlePlatformsGameInstance::Host(FString ServerName)
{
    DesiredServerName = ServerName;

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

void UPuzzlePlatformsGameInstance::Join(const uint32 Index)
{

    if (SessionInterface == nullptr) return;
    if (SessionSearch == nullptr) return;

    SessionInterface->JoinSession(0, SESSION_NAME, SessionSearch->SearchResults[Index]);

}

void UPuzzlePlatformsGameInstance::StartSession()
{
    if (SessionInterface.IsValid())
    {
        // Mark Session as "In progress" and is not reachable
        SessionInterface->StartSession(SESSION_NAME);
    }
}

void UPuzzlePlatformsGameInstance::RefreshServerList()
{
    SessionSearch = MakeShareable(new FOnlineSessionSearch());

    if (SessionSearch.IsValid())
    {
        SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
        SessionSearch->MaxSearchResults = 100;
        SessionSearch->QuerySettings.Set(SEARCH_KEYWORDS, FString("PuzzlePlatformsShenkrad"), EOnlineComparisonOp::In);
        // SessionSearch->bIsLanQuery = true;
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

    World->ServerTravel("/Game/PuzzlePlatforms/Maps/Lobby?listen");
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
        TArray<FServerData> ServerData;
        for (const FOnlineSessionSearchResult& SearchResult: SessionSearch->SearchResults)
        {
            FServerData Data;

            Data.Name = SearchResult.GetSessionIdStr();
            Data.MaxPlayers = SearchResult.Session.SessionSettings.NumPublicConnections;
            Data.CurrentPlayers = Data.MaxPlayers - SearchResult.Session.NumOpenPublicConnections;
            Data.HostUsername = SearchResult.Session.OwningUserName;

            FString ServerName;
            if (SearchResult.Session.SessionSettings.Get(SERVER_NAME_SETTINGS_KEY, ServerName))
            {
                Data.Name =  ServerName;
            }
            else
            {
                Data.Name = "Could not find name";
            }

            ServerData.Add(Data);
        }
        Menu->SetServerList(ServerData);
        
        UE_LOG(LogTemp, Warning, TEXT("Finished finding game sessions"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No game sessions found"));
    }
}

void UPuzzlePlatformsGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{   
    if (SessionInterface == nullptr) return;
    FString ConnectString;
    if(!SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
    {
        UE_LOG(LogTemp, Error, TEXT("Could not get connect string"));
    }

    UEngine* Engine = GetEngine();
    if (!ensure(Engine != nullptr)) return;

    Engine->AddOnScreenDebugMessage(0, 5, FColor::Green, FString::Printf(TEXT("Joining %s"), *ConnectString));

    APlayerController* PlayerController = GetFirstLocalPlayerController();
    if (!ensure(PlayerController != nullptr)) return;

    PlayerController->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
}

void UPuzzlePlatformsGameInstance::OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
    LoadMainMenu();
}

void UPuzzlePlatformsGameInstance::CreateSession()
{
    if (SessionInterface.IsValid())
    {
        FOnlineSessionSettings SessionSettings;

        if (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL")
        {
           SessionSettings.bIsLANMatch = true;
        }
        else
        {
            SessionSettings.bIsLANMatch = false;
        }

        SessionSettings.NumPublicConnections = 5;
        SessionSettings.bShouldAdvertise = true;
        SessionSettings.bUsesPresence = true;
        SessionSettings.bUseLobbiesIfAvailable = true;
        SessionSettings.Set(SERVER_NAME_SETTINGS_KEY, DesiredServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

        SessionInterface->CreateSession(0, SESSION_NAME, SessionSettings);
    }
}
