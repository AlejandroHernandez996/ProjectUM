// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableObjectInterface.h"
#include "GameFramework/Actor.h"
#include "ProjectUMLootableProp.generated.h"

UCLASS()
class PROJECTUM_API AProjectUMLootableProp : public AActor, public IInteractableObjectInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectUMLootableProp();

	void Interact_Implementation(AProjectUMCharacter* Interactor) override;

	UFUNCTION()
		UProjectUMInventoryComponent* GetInventory() { return Inventory; }

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
protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Iventory", meta = (AllowPrivateAccess = "true"))
		class UProjectUMInventoryComponent* Inventory;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
