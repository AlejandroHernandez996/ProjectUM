// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectUMInventoryComponent.h"
#include "ProjectUMItem.h"
#include "EquippableSlotsEnum.h"
#include "ProjectUMCharacter.h"
#include "Engine/Engine.h"

// Sets default values for this component's properties
UProjectUMInventoryComponent::UProjectUMInventoryComponent()
{
	Capacity = 20;
}

bool UProjectUMInventoryComponent::ContainsItem(UProjectUMItem* StackableItem) {
	for (auto& Item : Items) {
		if (Item->ItemId == StackableItem->ItemId) {
			return true;
		}
	}
	return false;
}

void UProjectUMInventoryComponent::AddItem(UProjectUMItem* Item)
{
	if (!Item) return;
	if (Item->bIsStackable && ContainsItem(Item)) {
		AddStackableItem(Item, Item->StackSize);
		return;
	}

	if (Items.Num() >= Capacity || !Item) {
		return;
	}

	Item->OwningInventory = this;
	Item->World = GetWorld();
	Items.Add(Item);
	UpdateInventoryToLootingCharacters();
	if (OwningCharacter) {
		OwningCharacter->BroadcastInventory();
	}

}

void UProjectUMInventoryComponent::RemoveItem(UProjectUMItem* Item)
{
	if (!Item) {
		return;
	}

	if (Item->bIsStackable && ContainsItem(Item) && Item->StackSize > 1) {
		RemoveStackableItem(Item, Item->StackSize);
		return;
	}
	Item->OwningInventory = nullptr;
	Item->World = nullptr;
	Items.Remove(Item);
	UpdateInventoryToLootingCharacters();
	if (OwningCharacter) {
		OwningCharacter->BroadcastInventory();
	}
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("REMOVE ITEM"));
}

void UProjectUMInventoryComponent::DropItem(UProjectUMItem* Item)
{
	if (!Item) {
		return;
	}
	Item->OwningInventory = nullptr;
	Item->World = nullptr;
	Items.Remove(Item);
	UpdateInventoryToLootingCharacters();
	if (OwningCharacter) {
		OwningCharacter->BroadcastInventory();
	}
}

void UProjectUMInventoryComponent::AddStackableItem(UProjectUMItem* StackableItem, int32 Amount)
{
	for (auto& Item : Items) {
		if (Item->ItemId == StackableItem->ItemId) {
			Item->StackSize += StackableItem->StackSize;
		}
	}
	
	if (OwningCharacter) {
		OwningCharacter->BroadcastInventory();
	}
	UpdateInventoryToLootingCharacters();
}

void UProjectUMInventoryComponent::RemoveStackableItem(UProjectUMItem* StackableItem, int32 Amount)
{
	if (StackableItem->StackSize < Amount) return;

	for (auto& Item : Items) {
		if (Item->ItemId == StackableItem->ItemId) {
			Item->StackSize -= Amount;
		}
	}
	if (StackableItem->StackSize == 0) {
		StackableItem->OwningInventory = nullptr;
		StackableItem->World = nullptr;
		Items.Remove(StackableItem);
	}
	if (OwningCharacter) {
		OwningCharacter->BroadcastInventory();
	}
	UpdateInventoryToLootingCharacters();
}

void UProjectUMInventoryComponent::EquipItem(class UProjectUMEquippableItem* Item, EEquippableSlotsEnum EquipSlot) {

	if (!EquipmentMap.FindRef(EquipSlot)) {
		EquipmentMap.Add(EquipSlot, Item);
		Items.Remove(Item);
		UpdateInventoryToLootingCharacters();
	}
}

void UProjectUMInventoryComponent::UnEquipItem(class UProjectUMEquippableItem* Item, EEquippableSlotsEnum EquipSlot) {
	if (EquipmentMap.FindRef(EquipSlot) == Item) {
		EquipmentMap.Add(EquipSlot, nullptr);
		Items.Add(Item);
		UpdateInventoryToLootingCharacters();
	}
}


void UProjectUMInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UProjectUMInventoryComponent::UpdateInventoryToLootingCharacters() {
	for (auto& LootingCharacter : LootingCharacters) {
		LootingCharacter->BroadcastNpcLoot(GetAllInventoryItems());
	}
}

TArray<FItemStruct> UProjectUMInventoryComponent::GetAllInventoryItems() {
	TArray<FItemStruct> InventoryItems = TArray<FItemStruct>();
	for (auto& Item : Items) {
		if (Item) {
			FItemStruct ItemStruct;
			ItemStruct._bIsStackable = Item->bIsStackable;
			ItemStruct._ItemId = Item->ItemId;
			ItemStruct._StackSize = Item->StackSize;
			InventoryItems.Add(ItemStruct);
		}
	}
	return InventoryItems;
}

TArray<FItemStruct> UProjectUMInventoryComponent::GetAllEquippedItems() {
	TArray<UProjectUMEquippableItem*> EquippedItems;
	EquipmentMap.GenerateValueArray(EquippedItems);

	TArray<FItemStruct> EquippedItemStructs = TArray<FItemStruct>();
	for (auto& Item : EquippedItems) {
		if (Item) {
			FItemStruct ItemStruct;
			ItemStruct._bIsStackable = Item->bIsStackable;
			ItemStruct._ItemId = Item->ItemId;
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, "Item Id:" + FString::FromInt(ItemStruct._ItemId));
			ItemStruct._StackSize = Item->StackSize;
			EquippedItemStructs.Add(ItemStruct);
		}
	}
	return EquippedItemStructs;
}



