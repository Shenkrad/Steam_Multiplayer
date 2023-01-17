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

	uint32 PlayerQuantity = 0;
	
};
