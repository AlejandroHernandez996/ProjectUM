// DroneCameraActor.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraComponent.h"
#include "ProjectUMCharacter.h"
#include "DroneCameraActor.generated.h"

UCLASS()
class PROJECTUM_API ADroneCameraActor : public AActor
{
	GENERATED_BODY()

public:
	ADroneCameraActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		UCameraComponent* CameraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Movement", meta = (AllowPrivateAccess = "true"))
		float CameraSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Movement", meta = (AllowPrivateAccess = "true"))
		float TargetOffsetZ;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Movement", meta = (AllowPrivateAccess = "true"))
		float InterpolationSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Movement", meta = (AllowPrivateAccess = "true"))
		float FollowDistance;

	TArray<AProjectUMCharacter*> CharactersToFollow;

	FVector TargetLocation;
};
