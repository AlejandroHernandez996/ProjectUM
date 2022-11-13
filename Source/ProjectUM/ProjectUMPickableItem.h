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
	
	// Sphere component used to test collision.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
		class USphereComponent* SphereComponent;

	// Static Mesh used to provide a visual representation of the object.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
		class UStaticMeshComponent* StaticMesh;

	UFUNCTION()
		void SetStaticMesh(UStaticMesh* Mesh) {
			StaticMesh->SetStaticMesh(Mesh);
		}

	UFUNCTION()
		void SetItem(UProjectUMItem* NewItem) {
		Item = NewItem;
	}

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
