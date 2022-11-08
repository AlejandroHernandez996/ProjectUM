// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectUMInventoryComponent.h"
#include "ProjectUMItem.h"
#include "EquippableSlotsEnum.h"
#include "Engine/Engine.h"

// Sets default values for this component's properties
UProjectUMInventoryComponent::UProjectUMInventoryComponent()
{

	Capacity = 20;
	
}

void UProjectUMInventoryComponent::AddItem(UProjectUMItem* Item)
{
	if (Items.Num() >= Capacity || !Item) {
		return;
	}
	Item->OwningInventory = this;
	Item->World = GetWorld();
	Items.Add(Item);
	OnInventoryUpdated.Broadcast();
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("ADD ITEM"));

}

void UProjectUMInventoryComponent::RemoveItem(UProjectUMItem* Item)
{
	if (!Item) {
		return;
	}
	Item->OwningInventory = nullptr;
	Item->World = nullptr;
	Items.Remove(Item);
	OnInventoryUpdated.Broadcast();
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("REMOVE ITEM"));
}

void UProjectUMInventoryComponent::EquipItem(class UProjectUMEquippableItem* Item, EEquippableSlotsEnum EquipSlot) {
	if (EquipSlot == EEquippableSlotsEnum::CHEST && ChestSlot != Item) {
		ChestSlot = Item;
	}

	else if (EquipSlot == EEquippableSlotsEnum::LEGS && LegsSlot != Item) {
		LegsSlot = Item;
	}

	else if (EquipSlot == EEquippableSlotsEnum::HEAD && HeadSlot != Item) {
		HeadSlot = Item;
	}
	else {
		return;
	}
	Items.Remove(Item);
	OnInventoryUpdated.Broadcast();
}

void UProjectUMInventoryComponent::UnEquipItem(class UProjectUMEquippableItem* Item, EEquippableSlotsEnum EquipSlot) {
	if (EquipSlot == EEquippableSlotsEnum::CHEST && ChestSlot == Item) {
		ChestSlot = nullptr;
	}

	else if (EquipSlot == EEquippableSlotsEnum::LEGS && LegsSlot == Item) {
		LegsSlot = nullptr;
	}

	else if (EquipSlot == EEquippableSlotsEnum::HEAD && HeadSlot == Item) {
		HeadSlot = nullptr;
	}
	else {
		return;
	}
	Items.Add(Item);
	OnInventoryUpdated.Broadcast();
}


void UProjectUMInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}




