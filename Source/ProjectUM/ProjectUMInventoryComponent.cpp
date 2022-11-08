// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectUMInventoryComponent.h"
#include "ProjectUMItem.h"
#include "Engine/Engine.h"

// Sets default values for this component's properties
UProjectUMInventoryComponent::UProjectUMInventoryComponent()
{

	Capacity = 20;
	
}

void UProjectUMInventoryComponent::AddItem_Implementation(UProjectUMItem* Item)
{
	if (Items.Num() >= Capacity || !Item) {
		return;
	}
	Item->OwningInventory = this;
	Item->World = GetWorld();
	Items.Add(Item);
	OnInventoryUpdated.Broadcast();
}

void UProjectUMInventoryComponent::RemoveItem_Implementation(UProjectUMItem* Item)
{
	if (!Item) {
		return;
	}
	Item->OwningInventory = nullptr;
	Item->World = nullptr;
	Items.Remove(Item);
	OnInventoryUpdated.Broadcast();
}


void UProjectUMInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}




