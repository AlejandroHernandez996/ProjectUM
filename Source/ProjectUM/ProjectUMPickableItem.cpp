// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectUMPickableItem.h"
#include "ProjectUMInventoryComponent.h"
#include "ProjectUMCharacter.h"

// Sets default values
AProjectUMPickableItem::AProjectUMPickableItem()
{
}

void AProjectUMPickableItem::Interact_Implementation(AProjectUMCharacter* Interactor)
{
	FString msg = "PICKING UP " + this->GetName();
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, msg);
	Interactor->GetInventory()->AddItem(DuplicateObject(Item, nullptr));
}