// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectUMEquippableItem.h"
#include "ProjectUMCharacter.h"
#include "Engine/Engine.h"
#include "ProjectUMInventoryComponent.h"

void UProjectUMEquippableItem::Use(class AProjectUMCharacter* CharacterUser) {

	if (!CharacterUser->GetInventory()->EquipmentMap.FindRef(EquipSlot) || CharacterUser->GetInventory()->EquipmentMap.FindRef(EquipSlot)->ItemId != ItemId) {
		UProjectUMEquippableItem* EquippedItem = CharacterUser->GetInventory()->EquipmentMap.FindRef(EquipSlot);
		if (EquippedItem) {
			CharacterUser->DeAttachEquipment(EquipSlot);
			CharacterUser->GetInventory()->UnEquipItem(EquippedItem, EquipSlot);
		}
		CharacterUser->GetInventory()->EquipItem(this, EquipSlot);
		CharacterUser->AttachEquipment(EquipmentClass, EquipSlot);
	}
	else {
		CharacterUser->DeAttachEquipment(EquipSlot);
		CharacterUser->GetInventory()->UnEquipItem(this, EquipSlot);
	}
}
