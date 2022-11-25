// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectUMEquipment.generated.h"

UCLASS()
class PROJECTUM_API AProjectUMEquipment : public AActor
{
	GENERATED_BODY()
	

public:
	AProjectUMEquipment();

	AActor* OwningActor;
	class AProjectUMCharacter* OwningCharacter;
	UFUNCTION()
		UStaticMeshComponent* GetHitboxComponent() { return MeshComponent; };

	void SetMesh(UStaticMesh* Mesh, FQuat Rot, FVector Loc, FVector Scale);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Collisions, meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent* MeshComponent;

};
