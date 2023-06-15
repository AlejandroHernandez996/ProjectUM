// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShotoProjectile.generated.h"

UCLASS()
class PROJECTUM_API AShotoProjectile : public AActor
{
    GENERATED_BODY()

public:
    AShotoProjectile();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
        UStaticMeshComponent* ProjectileMesh;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
        float InitialSize;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
        float MinSize;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
        float MaxSize;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
        float InitialSpeed;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
        float MinSpeed;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
        float MaxSpeed;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
        float GrowthRate;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
        float MaxGrowthTime;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
        float Damage;

    UFUNCTION()
        void OnProjectileOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    FVector InitialScale;
    float CurrentSpeed;
    float CurrentSize;
    float ElapsedTime;

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
};