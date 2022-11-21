// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "Http.h"
#include "ProjectUMMainMenuGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGetInventory);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGetStash);

UCLASS()
class PROJECTUM_API AProjectUMMainMenuGameState : public AGameStateBase
{
	GENERATED_BODY()

	AProjectUMMainMenuGameState();

	void OnInventoryResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);

	void OnStashResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);

	void OnMoveItemReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
		FOnGetInventory OnGetInventory;

	UPROPERTY(BlueprintAssignable, Category = "Stash")
		FOnGetStash OnGetStash;
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Instanced, Category = "Cache")
		class UProjectUMAssetCache* AssetCache;

	UFUNCTION(BlueprintCallable, Category = "Game State")
		class UProjectUMItem* GetItemById(int32 ItemId);

	UPROPERTY(BlueprintReadOnly)
		TArray<class UProjectUMItem*> Inventory;
	UPROPERTY(BlueprintReadOnly)
		TArray<class UProjectUMItem*> Stash;

	UFUNCTION(BlueprintCallable)
		void RequestInventory();

	UFUNCTION(BlueprintCallable)
		void RequestStash();

	UFUNCTION(BlueprintCallable)
		void MoveItemInventoryToStash(int32 ItemIndex);

	UFUNCTION(BlueprintCallable)
		void MoveItemStashToInventory(int32 ItemIndex);

};

USTRUCT()
struct FMoveItemStruct {

	GENERATED_USTRUCT_BODY()
	
	UPROPERTY()
		FString owningPlayerId;
	UPROPERTY()
		int32 itemIndex;
};