// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectUMItem.generated.h"

UENUM(BlueprintType)
namespace RarityEnum
{
	enum Rarity
	{
		COMMON UMETA(DisplayName = "COMMON"),
		UNCOMMON = 1  UMETA(DisplayName = "UNCOMMON"),
		RARE = 2 UMETA(DisplayName = "RARE"),
		EPIC = 3 UMETA(DisplayName = "EPIC"),
		LEGENDARY = 4 UMETA(DisplayName = "LEGENDARY")
	};
}

UCLASS(Abstract, BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class PROJECTUM_API UProjectUMItem : public UDataAsset
{
	GENERATED_BODY()

public:
	UProjectUMItem();

	UPROPERTY(Transient)
		class UWorld* World;

	virtual class UWorld* GetWorld() const { return World; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		FText ItemName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		FText ItemDescription;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		float ItemInventoryWeight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		TEnumAsByte<RarityEnum::Rarity> ItemRarity = RarityEnum::Rarity::COMMON;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		UTexture2D* Thumbnail;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		UStaticMesh* PickupMesh;

	UPROPERTY()
		class UProjectUMInventoryComponent* OwningInventory;

	virtual void Use(class AProjectUMCharacter* CharacterUser);
};
