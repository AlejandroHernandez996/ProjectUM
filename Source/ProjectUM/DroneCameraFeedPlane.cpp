#include "DroneCameraFeedPlane.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

ADroneCameraFeedPlane::ADroneCameraFeedPlane()
{
	PrimaryActorTick.bCanEverTick = false;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	RootComponent = StaticMeshComponent;
}

void ADroneCameraFeedPlane::BeginPlay()
{
	Super::BeginPlay();

	UMaterialInterface* Material = StaticMeshComponent->GetMaterial(0);
	if (Material)
	{
		DynamicMaterialInstance = UMaterialInstanceDynamic::Create(Material, this);
		StaticMeshComponent->SetMaterial(0, DynamicMaterialInstance);
	}
}

void ADroneCameraFeedPlane::UpdateCameraFeed(UTexture2D* CameraFeedTexture)
{
	if (DynamicMaterialInstance)
	{
		DynamicMaterialInstance->SetTextureParameterValue("CameraFeedTexture", CameraFeedTexture);
	}
}
