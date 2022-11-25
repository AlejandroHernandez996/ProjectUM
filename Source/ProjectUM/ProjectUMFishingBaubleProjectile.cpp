// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectUMFishingBaubleProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "ProjectUMResource.h"

AProjectUMFishingBaubleProjectile::AProjectUMFishingBaubleProjectile() {

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Game/Models/Fishing_Baubble.Fishing_Baubble"));
	//Set the Static Mesh and its position/scale if you successfully found a mesh asset to use.
	if (DefaultMesh.Succeeded())
	{
		StaticMesh->SetStaticMesh(DefaultMesh.Object);
		StaticMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -37.5f));
		StaticMesh->SetRelativeScale3D(FVector(5.0f, 5.0f, 5.0f));
	}

	ProjectileMovementComponent->ProjectileGravityScale = 1.0f;
	bOnImpactDestroy = false;

	if (GetLocalRole() == ROLE_Authority)
	{
		SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AProjectUMFishingBaubleProjectile::OnBaubleOverlapBegin);
	}

	SphereComponent->SetCollisionProfileName("Projectile");
}

void AProjectUMFishingBaubleProjectile::OnBaubleOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}
	AProjectUMResource* ResourceObject = Cast<AProjectUMResource>(OtherActor);
	IProjectUMItemGenerator* ItemGeneratorObject = Cast<IProjectUMItemGenerator>(OtherActor);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, "IMPACT");

	if (ResourceObject) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, "FOUND RESOURCE");

	}

	if (ItemGeneratorObject) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, "FOUND GENERATOR");

	}
	if (ResourceObject && ResourceObject->GetToolType() == EToolTypeEnum::FISHING_POLE && ItemGeneratorObject && OwningCharacter) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, "HIT FISHING RESOURSCE");
		ItemGeneratorObject->Execute_Generate(ResourceObject->_getUObject(), OwningCharacter);
	}
}