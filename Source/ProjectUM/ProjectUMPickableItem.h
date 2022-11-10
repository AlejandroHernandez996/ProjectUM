// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableObjectInterface.h"
#include "ProjectUMPickableItem.generated.h"

UCLASS()
class PROJECTUM_API AProjectUMPickableItem : public AActor, public IInteractableObjectInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectUMPickableItem();

	void Interact_Implementation(AProjectUMCharacter* Interactor) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item", Instanced)
		class UProjectUMItem* Item;
};
