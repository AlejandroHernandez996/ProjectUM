// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectUMInventoryComponent.h"
#include "ProjectUMItem.h"
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


void UProjectUMInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}




