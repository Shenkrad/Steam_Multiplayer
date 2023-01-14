// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ServerRow.generated.h"

class UTextBlock;
class UMainMenu;
class UButton;

/**
 * 
 */
UCLASS()
class PUZZLEPLATFORMS_API UServerRow : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ServerName;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* HostUsername;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerInfo;

	UPROPERTY(BlueprintReadOnly)
	bool Selected = false;

	void Setup(UMainMenu* Parent, uint32 Index);

	UFUNCTION(BlueprintImplementableEvent)
	void OnSelectedChanged();

private:

	UPROPERTY(meta = (BindWidget))
	UButton* RowButton;

	UPROPERTY()
	UMainMenu* ParentComponent;

	uint32 OwnIndex;

	UFUNCTION()
	void OnClicked();

};
