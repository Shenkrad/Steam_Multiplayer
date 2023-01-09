// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MenuSystem/MenuInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "PuzzlePlatformsGameInstance.generated.h"

class UUserWidget;
class UMainMenu;
class FOnlineSessionSearch;


/**
 * 
 */
UCLASS()
class PUZZLEPLATFORMS_API UPuzzlePlatformsGameInstance : public UGameInstance, public IMenuInterface
{
	GENERATED_BODY()

public:
	UPuzzlePlatformsGameInstance(const FObjectInitializer & ObjectInitializer);

	virtual void Init() override;

	UFUNCTION(BlueprintCallable)
	void LoadMenu();

	UFUNCTION(BlueprintCallable)
	void InGameLoadMenu();

	UFUNCTION(exec)
	void Host() override;

	UFUNCTION(exec)
	void Join(const FString& Address) override;

	void RefreshServerList() override;

	virtual void LoadMainMenu() override;

	virtual void ExitGame() override;

private:

	TSubclassOf<UUserWidget> MenuClass;

	TSubclassOf<UUserWidget> InGameMenuClass;

	UMainMenu* Menu;

	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	void OnCreateSessionComplete(FName SessionName, bool Success);
	void OnDestroySessionComplete(FName SessionName, bool Success);
	void OnFindSessionsComplete(bool Success);

	void CreateSession();
};
