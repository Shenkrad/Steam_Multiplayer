// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PuzzlePlatformsGameMode.h"
#include "LobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class PUZZLEPLATFORMS_API ALobbyGameMode : public APuzzlePlatformsGameMode
{
	GENERATED_BODY()

protected:

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void Logout(AController* Exiting) override;


private:

	uint16 PlayerQuantity = 0;
	uint16 PlayersToStart = 2;

	FTimerHandle StartTimerHandle;
	float StartTimerDelay = 10.f;

	UFUNCTION()
	void StartGame();
	
};
