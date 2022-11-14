// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProjectUMLootTable.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTUM_API UProjectUMLootTable : public UActorComponent
{
	GENERATED_BODY()

public:	
	UProjectUMLootTable();

	class AProjectUmNpc* OwningNpc;

	void GenerateLoot();

	TArray<class UProjectUMItem*> GetLoot() { return Loot;  }

protected:
	int MaxLoot;

	int MinLoot;

	virtual void BeginPlay() override;

	TArray<class UProjectUMItem*> Loot;
};
