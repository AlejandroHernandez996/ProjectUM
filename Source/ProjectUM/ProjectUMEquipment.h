// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectUMEquipment.generated.h"

UCLASS()
class PROJECTUM_API AProjectUMEquipment : public AActor
{
	GENERATED_BODY()
	
public:	
	AActor* OwningActor;
	// Sets default values for this actor's properties
	AProjectUMEquipment();

	UFUNCTION()
		USphereComponent* GetHitboxComponent() { return HitboxComponent; };
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Collisions, meta = (AllowPrivateAccess = "true"))
		class USphereComponent* HitboxComponent;
};
