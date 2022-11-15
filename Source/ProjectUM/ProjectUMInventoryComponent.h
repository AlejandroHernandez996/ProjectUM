// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProjectUMEquippableItem.h"
#include "EquippableSlotsEnum.h"
#include "ProjectUMInventoryComponent.generated.h"

// Blueprint will bind to update the UI
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTUM_API UProjectUMInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

	virtual void BeginPlay() override;

public:
	UProjectUMInventoryComponent();

	UFUNCTION()
		void AddItem(class UProjectUMItem* Item);
	
	UFUNCTION()
		void AddStackableItem(class UProjectUMItem* StackableItem, int Amount);

	UFUNCTION()
		bool ContainsItem(class UProjectUMItem* Item);

	UFUNCTION()
		void RemoveItem(class UProjectUMItem* Item);

	UFUNCTION()
		void DropItem(class UProjectUMItem* Item);

	UFUNCTION()
		void RemoveStackableItem(class UProjectUMItem* StackableItem, int Amount);

	UFUNCTION()
		void EquipItem(class UProjectUMEquippableItem* Item, EEquippableSlotsEnum EquipSlot);

	UFUNCTION()
		void UnEquipItem(class UProjectUMEquippableItem* Item, EEquippableSlotsEnum EquipSlot);

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
		int32 Capacity;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
		class AProjectUMCharacter* OwningCharacter;

	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Inventory")
		TArray<class UProjectUMItem*> DefaultItems;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
		FOnInventoryUpdated OnInventoryUpdated; 

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Instanced, Category = "Inventory")
		TArray<class UProjectUMItem*> Items;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Instanced, Category = "Inventory")
		TMap<EEquippableSlotsEnum, UProjectUMEquippableItem*> EquipmentMap;

	UPROPERTY()
		TSet<class AProjectUMCharacter*> LootingCharacters;

	UFUNCTION()
		void UpdateInventoryToLootingCharacters();

	UFUNCTION()
		TArray<FItemStruct> GetAllInventoryItems();

	UFUNCTION()
		TArray<FItemStruct> GetAllEquippedItems();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
		UProjectUMEquippableItem* GetEquipmentItem(EEquippableSlotsEnum EquipSlot) { return EquipmentMap.FindRef(EquipSlot); }

	UFUNCTION(BlueprintCallable, Category = "Inventory")
		UProjectUMItem* GetItemById(int32 ItemId) { 
		for (auto& Item : Items) {
			if (Item->ItemId == ItemId) {
				return Item;
			}
		}
		TArray< UProjectUMEquippableItem*> EquippedItems;
		EquipmentMap.GenerateValueArray(EquippedItems);
		for (auto& Item : EquippedItems) {
			if (Item->ItemId == ItemId) {
				return Item;
			}
		}
		return nullptr;
	}
};
