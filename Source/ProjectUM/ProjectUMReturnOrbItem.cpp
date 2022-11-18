// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectUMReturnOrbItem.h"
#include "Json.h"
#include "ProjectUMInventoryComponent.h"
#include "ProjectUMCharacter.h"
#include "ProjectUMItem.h"
#include "JsonObjectConverter.h"

void UProjectUMReturnOrbItem::Use(AProjectUMCharacter* CharacterUser)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, "ORB");
	FHttpRequestRef RequestDelete = FHttpModule::Get().CreateRequest();
	RequestDelete->SetURL("https://eqsntvit1c.execute-api.us-east-2.amazonaws.com/inventory/JANDRO");
	RequestDelete->SetVerb("DELETE");
	RequestDelete->ProcessRequest();

	TArray<UProjectUMItem*> ItemsArray = CharacterUser->GetInventory()->Items;
	TArray<FItemJsonStruct> Items;
	int32 Index = 0;
	FInventoryJsonStruct Inventory;
	for (auto& Item : CharacterUser->GetInventory()->Items) {
		FItemJsonStruct JsonItemStruct;
		JsonItemStruct.is_stackable = Item->bIsStackable;
		JsonItemStruct.quantity = Item->StackSize;
		JsonItemStruct.item_id = Item->ItemId;
		JsonItemStruct.collection_index = Index;
		JsonItemStruct.rarity = UProjectUMItem::RarityEnumToString(Item->ItemRarity);
		JsonItemStruct.stats.Add("Str", Item->Stats.FindOrAdd(ECharacterStatEnum::STRENGTH));
		JsonItemStruct.stats.Add("Int", Item->Stats.FindOrAdd(ECharacterStatEnum::INTELLECT));
		JsonItemStruct.stats.Add("Agi", Item->Stats.FindOrAdd(ECharacterStatEnum::AGILITY));
		JsonItemStruct.stats.Add("Wis", Item->Stats.FindOrAdd(ECharacterStatEnum::WISDOM));
		JsonItemStruct.stats.Add("Health", Item->Stats.FindOrAdd(ECharacterStatEnum::HEALTH));
		JsonItemStruct.stats.Add("Mana", Item->Stats.FindOrAdd(ECharacterStatEnum::MANA));
		Items.Add(JsonItemStruct);
		Index++;
	}

	Inventory.owning_player_id = TEXT("JANDRO");
	Inventory.items = Items;
	FString PutString;
	FJsonObjectConverter::UStructToJsonObjectString(Inventory, PutString);

	FHttpRequestRef RequestPut = FHttpModule::Get().CreateRequest();
	RequestPut->OnProcessRequestComplete().BindUObject(this, &UProjectUMReturnOrbItem::OnResponseReceived);
	RequestPut->SetURL("https://eqsntvit1c.execute-api.us-east-2.amazonaws.com/inventory");
	RequestPut->SetVerb("PUT");
	RequestPut->SetHeader("Content-Type", "application/json");
	RequestPut->SetContentAsString(PutString);
	RequestPut->ProcessRequest();

	UE_LOG(LogTemp, Display, TEXT("PUT %s"), *PutString);
	
	
	//CharacterUser->Disconnect();
}

void UProjectUMReturnOrbItem::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	TSharedPtr<FJsonObject> ResponseObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	FJsonSerializer::Deserialize(Reader, ResponseObj);

	UE_LOG(LogTemp, Display, TEXT("Response %s"), *Response->GetContentAsString());
}
