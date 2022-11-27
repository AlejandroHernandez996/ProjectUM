// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectUMFishingRod.h"
#include "ProjectUMCharacter.h"
#include "ProjectUMFishingBaubleProjectile.h"
#include "CableComponent.h"
#include "Camera/CameraComponent.h"
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

	FVector CameraLocation = OwningCharacter->GetActorLocation();
	FRotator CameraRotation = OwningCharacter->GetFollowCamera()->GetComponentRotation();
	FVector SpawnLocation = CameraLocation + FTransform(CameraRotation).TransformVector(FVector(200.0f, 0.0f, 50.0f));

	// Skew the aim to be slightly upwards.
	FRotator MuzzleRotation = CameraRotation;
	MuzzleRotation.Pitch += 10.0f;
	FVector spawnLocation = SpawnLocation;
	FRotator spawnRotation = MuzzleRotation;

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