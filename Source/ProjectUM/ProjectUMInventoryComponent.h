// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
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
	void RemoveItem(class UProjectUMItem* Item);


	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
		int32 Capacity;

	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Inventory")
		TArray<class UProjectUMItem*> DefaultItems;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
		FOnInventoryUpdated OnInventoryUpdated; 

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Instanced, Category = "Inventory")
		TArray<class UProjectUMItem*> Items;
};
