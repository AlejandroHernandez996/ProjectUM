#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DroneCameraFeedPlane.generated.h"

UCLASS()
class PROJECTUM_API ADroneCameraFeedPlane : public AActor
{
	GENERATED_BODY()

public:
	ADroneCameraFeedPlane();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UStaticMeshComponent* StaticMeshComponent;

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Drone Camera")
		void UpdateCameraFeed(UTexture2D* CameraFeedTexture);

private:
	UPROPERTY()
		UMaterialInstanceDynamic* DynamicMaterialInstance;
};