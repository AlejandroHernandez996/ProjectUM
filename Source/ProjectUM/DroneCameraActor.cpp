#include "DroneCameraActor.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "ProjectUMCharacter.h"

ADroneCameraActor::ADroneCameraActor()
{
	// Create a sphere component and set it as the root component
	USphereComponent* CameraSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CameraSphereComponent"));
	RootComponent = CameraSphereComponent;

	// Set the sphere radius
	CameraSphereComponent->InitSphereRadius(50.0f);
	CameraSphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Disable collision for the sphere

	// Create a camera component and attach it to the root component
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(RootComponent);
	CameraComponent->SetHiddenInGame(false); // Set the camera component to be visible in game

	// Set default values for speed and distance
	InterpolationSpeed = 10.0f; // Adjust this value to change the speed of the camera movement
	FollowDistance = 500.0f; // Adjust this value to change the distance of the camera from the characters
}

void ADroneCameraActor::BeginPlay()
{
	Super::BeginPlay();

	// Attach the camera to the first ProjectUMCharacter found in the level
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AProjectUMCharacter::StaticClass(), FoundActors);
	if (FoundActors.Num() > 0)
	{
		AProjectUMCharacter* Character = Cast<AProjectUMCharacter>(FoundActors[0]);
		if (Character)
		{
			AttachToComponent(Character->GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
		}
	}
}

void ADroneCameraActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CharactersToFollow.Num() == 0)
		return;

	FVector AverageLocation = FVector::ZeroVector;
	for (AProjectUMCharacter* Character : CharactersToFollow)
	{
		if (Character)
			AverageLocation += Character->GetActorLocation();
	}
	AverageLocation /= CharactersToFollow.Num();

	TargetLocation = AverageLocation - GetActorForwardVector() * FollowDistance;
	TargetLocation.Z += TargetOffsetZ;

	FVector CurrentLocation = GetActorLocation();
	FVector NewLocation = FMath::VInterpTo(CurrentLocation, TargetLocation, DeltaTime, InterpolationSpeed);
	SetActorLocation(NewLocation);

	FVector LookAtDirection = AverageLocation - CurrentLocation;
	FRotator NewRotation = LookAtDirection.Rotation();
	SetActorRotation(NewRotation);
}
