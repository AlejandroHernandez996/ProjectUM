// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectUMFishingRod.h"
#include "ProjectUMCharacter.h"
#include "ProjectUMFishingBaubleProjectile.h"
#include "CableComponent.h"
#include "RenderCore.h"

AProjectUMFishingRod::AProjectUMFishingRod() {
	PrimaryActorTick.bCanEverTick = true;
	bIsFiringWeapon = false;
	bIsChanneling = false;
	FireRate = 1.0f;
	ChannelStartTime = 0.0f;
	ChannelCurrentTime = 0.0f;
	MaxChannelTime = .5f;

	FishingLine = CreateDefaultSubobject<UCableComponent>("Fishing Line");
	FishingLine->SetCollisionProfileName("NoCollision");
	const FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, false);
	FishingLine->AttachToComponent(MeshComponent, AttachmentRules);
	FishingLine->SetIsReplicated(true);
}

void AProjectUMFishingRod::BeginPlay()
{
	Super::BeginPlay();
}

void AProjectUMFishingRod::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
	
	ChannelTimer.Tick(DeltaTime);
}

void AProjectUMFishingRod::StartFire()
{
	if (ChannelState == EChanneledStateEnum::CHANNELING && !bIsFiringWeapon && ChannelCurrentTime - ChannelStartTime >= MaxChannelTime) {
		bIsFiringWeapon = true;
		UWorld* World = GetWorld();
		World->GetTimerManager().SetTimer(FiringTimer, this, &AProjectUMFishingRod::StopFire, FireRate, false);
		HandleFire();
	}
	else {
		ChannelState = EChanneledStateEnum::IDLE;
		bIsFiringWeapon = false;
	}
}

void AProjectUMFishingRod::StopFire()
{
	bIsFiringWeapon = false;
}

void AProjectUMFishingRod::HandleFire()
{
	
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString(TEXT("FIRE ARROW")));
	ChannelState = EChanneledStateEnum::IDLE;
	FVector spawnLocation = OwningActor->GetActorLocation() + (OwningActor->GetActorRotation().Vector() * 200.0) + (OwningActor->GetActorUpVector() * 50.0f);
	FRotator spawnRotation = OwningActor->GetActorRotation();

	FActorSpawnParameters spawnParameters;
	spawnParameters.Instigator = OwningActor->GetInstigator();
	spawnParameters.Owner = OwningActor;

	SpawnedBauble = GetWorld()->SpawnActor<AProjectUMFishingBaubleProjectile>(spawnLocation, spawnRotation, spawnParameters);
	SpawnedBauble->SetOwningCharacter(OwningCharacter);
	FishingLine->SetAttachEndToComponent(SpawnedBauble->GetRootComponent());
}

void AProjectUMFishingRod::Channel_Implementation()
{

	if (ChannelState == EChanneledStateEnum::IDLE && !bIsFiringWeapon) {
		if (SpawnedBauble) {
			SpawnedBauble->Destroy();
		}
		ChannelState = EChanneledStateEnum::CHANNELING;
		ChannelStartTime = ChannelTimer.GetCurrentTime();
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString(TEXT("START BOW CHANNEL")));

	}
}

void AProjectUMFishingRod::Release_Implementation()
{
	ChannelCurrentTime = ChannelTimer.GetCurrentTime();
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString(TEXT("START BOW RELEASE")));
	StartFire();
}