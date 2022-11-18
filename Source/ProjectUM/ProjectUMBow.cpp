// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectUMBow.h"
#include "ProjectUMArrowProjectile.h"
#include "RenderCore.h"

AProjectUMBow::AProjectUMBow() {
	PrimaryActorTick.bCanEverTick = true;
	bIsFiringWeapon = false;
	bIsChanneling = false;
	FireRate = 1.0f;
	ChannelStartTime = 0.0f;
	ChannelCurrentTime = 0.0f;
	MaxChannelTime = .5f;
}

void AProjectUMBow::BeginPlay()
{
	Super::BeginPlay();
}

void AProjectUMBow::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	ChannelTimer.Tick(DeltaTime);
}

void AProjectUMBow::StartFire()
{
	if (ChannelState == EChanneledStateEnum::CHANNELING && !bIsFiringWeapon && ChannelCurrentTime-ChannelStartTime >= MaxChannelTime) {
		bIsFiringWeapon = true;
		UWorld* World = GetWorld();
		World->GetTimerManager().SetTimer(FiringTimer, this, &AProjectUMBow::StopFire, FireRate, false);
		HandleFire();
	}
	else {
		ChannelState = EChanneledStateEnum::IDLE;
		bIsFiringWeapon = false;
	}
}

void AProjectUMBow::StopFire()
{
	bIsFiringWeapon = false;
}

void AProjectUMBow::HandleFire()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString(TEXT("FIRE ARROW")));
	ChannelState = EChanneledStateEnum::IDLE;
	FVector spawnLocation = OwningActor->GetActorLocation() + (OwningActor->GetActorRotation().Vector() * 200.0) + (OwningActor->GetActorUpVector() * 50.0f);
	FRotator spawnRotation = OwningActor->GetActorRotation();

	FActorSpawnParameters spawnParameters;
	spawnParameters.Instigator = OwningActor->GetInstigator();
	spawnParameters.Owner = OwningActor;

	AProjectUMArrowProjectile* spawnedProjectile = GetWorld()->SpawnActor<AProjectUMArrowProjectile>(spawnLocation, spawnRotation, spawnParameters);
}

void AProjectUMBow::Channel_Implementation()
{
	if (ChannelState == EChanneledStateEnum::IDLE && !bIsFiringWeapon) {
		ChannelState = EChanneledStateEnum::CHANNELING;
		ChannelStartTime = ChannelTimer.GetCurrentTime();
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString(TEXT("START BOW CHANNEL")));

	}
}

void AProjectUMBow::Release_Implementation()
{
	ChannelCurrentTime = ChannelTimer.GetCurrentTime();
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString(TEXT("START BOW RELEASE")));
	StartFire();
}

