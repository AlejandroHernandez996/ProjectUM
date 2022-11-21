// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectUMMainMenuGameState.h"
#include "ProjectUMAssetCache.h"
#include "Engine/ObjectLibrary.h"
#include "ProjectUMInventoryComponent.h"
#include "Json.h"
#include "ProjectUMItem.h"
#include "JsonObjectConverter.h"

AProjectUMMainMenuGameState::AProjectUMMainMenuGameState() {
}

void AProjectUMMainMenuGameState::RequestInventory() {
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AProjectUMMainMenuGameState::OnInventoryResponseReceived);
	Request->SetURL("https://eqsntvit1c.execute-api.us-east-2.amazonaws.com/inventory/JANDRO");
	Request->SetVerb("GET");
	Request->ProcessRequest();
}

void AProjectUMMainMenuGameState::OnInventoryResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	TSharedPtr<FJsonObject> ResponseObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	FJsonSerializer::Deserialize(Reader, ResponseObj);

	FString ITEM = FString(TEXT("Item"));

	FInventoryJsonStruct InventoryStruct;
	FJsonObjectConverter::JsonObjectToUStruct(ResponseObj.Get()->GetObjectField(ITEM).ToSharedRef(), &InventoryStruct);
	Inventory.Reset();
	for (auto& ParsedJsonItem : InventoryStruct.items) {
		for (auto& Item : AssetCache->ItemCache) {
			if (Item && Item->ItemId == ParsedJsonItem.item_id) {
				UProjectUMItem* NewItem = DuplicateObject(Item, nullptr);
				NewItem->ItemRarity = UProjectUMItem::RarityStringToEnum(ParsedJsonItem.rarity);
				for (auto& kvp : ParsedJsonItem.stats) {
					NewItem->Stats.Add(UProjectUMItem::CharacterStatStringToEnum(kvp.Key), kvp.Value);
				}
				NewItem->StackSize = ParsedJsonItem.quantity;
				Inventory.Add(NewItem);
				break;
			}
		}
	}

	OnGetInventory.Broadcast();
}

void AProjectUMMainMenuGameState::RequestStash() {
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AProjectUMMainMenuGameState::OnStashResponseReceived);
	Request->SetURL("https://eqsntvit1c.execute-api.us-east-2.amazonaws.com/stash/JANDRO");
	Request->SetVerb("GET");
	Request->ProcessRequest();
}

void AProjectUMMainMenuGameState::OnStashResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	TSharedPtr<FJsonObject> ResponseObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	FJsonSerializer::Deserialize(Reader, ResponseObj);

	FString ITEM = FString(TEXT("Item"));

	FInventoryJsonStruct InventoryStruct;
	FJsonObjectConverter::JsonObjectToUStruct(ResponseObj.Get()->GetObjectField(ITEM).ToSharedRef(), &InventoryStruct);
	Stash.Reset();
	for (auto& ParsedJsonItem : InventoryStruct.items) {
		for (auto& Item : AssetCache->ItemCache) {
			if (Item && Item->ItemId == ParsedJsonItem.item_id) {
				UProjectUMItem* NewItem = DuplicateObject(Item, nullptr);
				NewItem->ItemRarity = UProjectUMItem::RarityStringToEnum(ParsedJsonItem.rarity);
				for (auto& kvp : ParsedJsonItem.stats) {
					NewItem->Stats.Add(UProjectUMItem::CharacterStatStringToEnum(kvp.Key), kvp.Value);
				}
				NewItem->StackSize = ParsedJsonItem.quantity;
				Stash.Add(NewItem);
				break;
			}
		}
	}

	OnGetStash.Broadcast();
}

UProjectUMItem* AProjectUMMainMenuGameState::GetItemById(int32  ItemId) {

	for (auto& Item : AssetCache->ItemCache) {
		if (Item && Item->ItemId == ItemId) {
			return Item;
		}
	}
	return nullptr;
}

void AProjectUMMainMenuGameState::MoveItemInventoryToStash(int32 ItemIndex) {
	FString PutString;
	FMoveItemStruct MoveItemStruct;
	MoveItemStruct.itemIndex = ItemIndex;
	MoveItemStruct.owningPlayerId = TEXT("JANDRO");
	FJsonObjectConverter::UStructToJsonObjectString(MoveItemStruct, PutString);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, PutString);

	FHttpRequestRef RequestPut = FHttpModule::Get().CreateRequest();
	RequestPut->OnProcessRequestComplete().BindUObject(this, &AProjectUMMainMenuGameState::OnMoveItemReceived);
	RequestPut->SetURL("https://eqsntvit1c.execute-api.us-east-2.amazonaws.com/move/i2s");
	RequestPut->SetVerb("POST");
	RequestPut->SetHeader("Content-Type", "application/json");
	RequestPut->SetContentAsString(PutString);
	RequestPut->ProcessRequest();
}


void AProjectUMMainMenuGameState::MoveItemStashToInventory(int32 ItemIndex) {
	FString PutString;
	FMoveItemStruct MoveItemStruct;
	MoveItemStruct.itemIndex = ItemIndex;
	MoveItemStruct.owningPlayerId = "JANDRO";
	FJsonObjectConverter::UStructToJsonObjectString(MoveItemStruct, PutString);

	FHttpRequestRef RequestPut = FHttpModule::Get().CreateRequest();
	RequestPut->OnProcessRequestComplete().BindUObject(this, &AProjectUMMainMenuGameState::OnMoveItemReceived);
	RequestPut->SetURL("https://eqsntvit1c.execute-api.us-east-2.amazonaws.com/move/s2i");
	RequestPut->SetVerb("POST");
	RequestPut->SetHeader("Content-Type", "application/json");
	RequestPut->SetContentAsString(PutString);
	RequestPut->ProcessRequest();
}

void AProjectUMMainMenuGameState::OnMoveItemReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	RequestStash();
	RequestInventory();
}