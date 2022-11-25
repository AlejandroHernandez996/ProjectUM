// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectUMEquipment.h"
#include "Components/StaticMeshComponent.h"

AProjectUMEquipment::AProjectUMEquipment()
{

	USceneComponent* SceneRootComponent = CreateDefaultSubobject<UStaticMeshComponent>("Root Component");
	bReplicates = true;
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("Equipment");
	const FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, false);
	MeshComponent->AttachToComponent(SceneRootComponent, AttachmentRules);
}

// Called when the game starts or when spawned
void AProjectUMEquipment::BeginPlay()
{
	Super::BeginPlay();
	MeshComponent->SetCollisionProfileName("NoCollision");
	MeshComponent->SetHiddenInGame(false);
	MeshComponent->SetNotifyRigidBodyCollision(false);
	MeshComponent->SetIsReplicated(true);

}

void AProjectUMEquipment::SetMesh(UStaticMesh* Mesh, FQuat Rot, FVector Loc, FVector Scale) {
	MeshComponent->SetStaticMesh(Mesh);
	MeshComponent->SetRelativeLocationAndRotation(Loc, Rot);
	MeshComponent->SetRelativeScale3D(Scale);
}