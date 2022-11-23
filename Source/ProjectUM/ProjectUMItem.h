// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectUMItemRarityEnum.h"
#include "CharacterStatEnum.h"
#include "ProjectUMItem.generated.h"

UCLASS(Abstract, BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class PROJECTUM_API UProjectUMItem : public UDataAsset
{
	GENERATED_BODY()

public:
	UProjectUMItem();

	UPROPERTY(Transient)
		class UWorld* World;

	virtual class UWorld* GetWorld() const { return World; }

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Item")
		FString ItemGuid;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Item")
		int32 ItemId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		FText ItemName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		FText ItemDescription;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		float ItemInventoryWeight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		EProjectUMItemRarityEnum ItemRarity = EProjectUMItemRarityEnum::COMMON;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		UTexture2D* Thumbnail;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		UStaticMesh* PickupMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		TMap<ECharacterStatEnum, float> Stats;

	UPROPERTY()
		class UProjectUMInventoryComponent* OwningInventory;

	virtual void Use(class AProjectUMCharacter* CharacterUser);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		bool bIsStackable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		int32 StackSize;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animation", meta = (AllowPrivateAccess = "true"))
		class UAnimMontage* AttackMontage;

	UFUNCTION()
		float GetStat(ECharacterStatEnum Stat) {
			return Stats.FindRef(Stat);
		};
	UFUNCTION()
		static ECharacterStatEnum CharacterStatStringToEnum(FString StatString) {
		if (StatString.Compare(TEXT("Str")) == 0) {
			return ECharacterStatEnum::STRENGTH;
		}
		if (StatString.Compare(TEXT("Int")) == 0) {
			return ECharacterStatEnum::INTELLECT;
		}
		if (StatString.Compare(TEXT("Wis")) == 0) {
			return ECharacterStatEnum::WISDOM;
		}
		if (StatString.Compare(TEXT("Agi")) == 0) {
			return ECharacterStatEnum::AGILITY;
		}
		if (StatString.Compare(TEXT("Health")) == 0) {
			return ECharacterStatEnum::HEALTH;
		}
		if (StatString.Compare(TEXT("Mana")) == 0) {
			return ECharacterStatEnum::MANA;
		}
		return ECharacterStatEnum::HEALTH;
	}

	UFUNCTION()
		static EProjectUMItemRarityEnum RarityStringToEnum(FString RarityString) {
		if (RarityString.Compare(TEXT("COMMON")) == 0) {
			return EProjectUMItemRarityEnum::COMMON;
		}
		if (RarityString.Compare(TEXT("UNCOMMON")) == 0) {
			return EProjectUMItemRarityEnum::UNCOMMON;
		}
		if (RarityString.Compare(TEXT("RARE")) == 0) {
			return EProjectUMItemRarityEnum::RARE;
		}
		if (RarityString.Compare(TEXT("EPIC")) == 0) {
			return EProjectUMItemRarityEnum::EPIC;
		}
		if (RarityString.Compare(TEXT("LEGENDARY")) == 0) {
			return EProjectUMItemRarityEnum::LEGENDARY;
		}
		return EProjectUMItemRarityEnum::COMMON;
	}

	UFUNCTION()
		static FString RarityEnumToString(EProjectUMItemRarityEnum RarityEnum) {
		if (RarityEnum == EProjectUMItemRarityEnum::COMMON) {
			return FString(TEXT("COMMON"));
		}
		if (RarityEnum == EProjectUMItemRarityEnum::UNCOMMON) {
			return FString(TEXT("UNCOMMON"));
		}
		if (RarityEnum == EProjectUMItemRarityEnum::RARE) {
			return FString(TEXT("RARE"));
		}
		if (RarityEnum == EProjectUMItemRarityEnum::EPIC) {
			return FString(TEXT("EPIC"));
		}
		if (RarityEnum == EProjectUMItemRarityEnum::LEGENDARY) {
			return FString(TEXT("LEGENDARY"));
		}
		return "COMMON";
	}
};

USTRUCT(BlueprintType)
struct FItemStruct
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		int32 _ItemId;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		int32 _StackSize;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		bool _bIsStackable;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		EProjectUMItemRarityEnum _Rarity;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		float _Health;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		float _Mana;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		float _Agility;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		float _Intellect;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		float _Strength;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		float _Wisdom;
};

USTRUCT()
struct FItemJsonStruct
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
		int32 item_id;
	UPROPERTY()
		FString rarity;
	UPROPERTY()
		int32 collection_index;
	UPROPERTY()
		bool is_stackable;
	UPROPERTY()
		int32 quantity;
	UPROPERTY()
		TMap<FString, float> stats;
};
