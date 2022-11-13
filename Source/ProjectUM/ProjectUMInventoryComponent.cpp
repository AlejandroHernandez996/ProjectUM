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
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Adding... Item " + Item->GetName()));

	if (Items.Num() >= Capacity || !Item) {
		return;
	}
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Added Item " + Item->GetName()));
	Item->OwningInventory = this;
	Item->World = GetWorld();
	Items.Add(Item);
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
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("REMOVE ITEM"));
}

void UProjectUMInventoryComponent::EquipItem(class UProjectUMEquippableItem* Item, EEquippableSlotsEnum EquipSlot) {

	if (!EquipmentMap.FindRef(EquipSlot)) {
		EquipmentMap.Add(EquipSlot, Item);
		Items.Remove(Item);
	}
}

void UProjectUMInventoryComponent::UnEquipItem(class UProjectUMEquippableItem* Item, EEquippableSlotsEnum EquipSlot) {
	if (EquipmentMap.FindRef(EquipSlot) == Item) {
		EquipmentMap.Add(EquipSlot, nullptr);
		Items.Add(Item);
	}
}


void UProjectUMInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}




