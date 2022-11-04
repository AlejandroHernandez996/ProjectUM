// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectUMNonPlayableCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values
AProjectUMNonPlayableCharacter::AProjectUMNonPlayableCharacter()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AProjectUMNonPlayableCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AProjectUMNonPlayableCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProjectUMNonPlayableCharacter::Destroyed()
{
	FVector spawnLocation = GetActorLocation();
	UGameplayStatics::SpawnEmitterAtLocation(this, ExplosionEffect, spawnLocation, FRotator::ZeroRotator, true, EPSCPoolMethod::AutoRelease);
	Destroy();
}

