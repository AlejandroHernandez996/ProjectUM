// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectUmProjectile.h"
#include "ProjectUMFishingBaubleProjectile.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTUM_API AProjectUMFishingBaubleProjectile : public AProjectUmProjectile
{
	GENERATED_BODY()
	AProjectUMFishingBaubleProjectile();

protected:
	class AProjectUMCharacter* OwningCharacter;

	UFUNCTION(Category = "Projectile")
		void OnBaubleOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);


public:
	void SetOwningCharacter(AProjectUMCharacter* Character) {
		OwningCharacter = Character;
	}
};
