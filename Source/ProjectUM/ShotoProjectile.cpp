#include "ShotoProjectile.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h" // Include the GameplayStatics header

AShotoProjectile::AShotoProjectile()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    // Set up the root component and mesh
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
    ProjectileMesh->SetupAttachment(RootComponent);

    // Set up default values
    InitialSpeed = 2000.0f;
    MaxSpeed = 10000.0f;

    if (!HasAuthority()) {
        ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    else {
        UPrimitiveComponent* Collider = Cast<UPrimitiveComponent>(ProjectileMesh);
        if (Collider)
        {
            Collider->SetCollisionProfileName("Projectile");
            Collider->SetGenerateOverlapEvents(true);
            Collider->OnComponentBeginOverlap.AddDynamic(this, &AShotoProjectile::OnProjectileOverlap);

        }
    }
}

void AShotoProjectile::BeginPlay()
{
    Super::BeginPlay();

    // Set the initial velocity of the projectile
    FVector LaunchDirection = GetActorForwardVector();
    CurrentSpeed = InitialSpeed;
    ProjectileMesh->SetPhysicsLinearVelocity(LaunchDirection * CurrentSpeed, false);
}

void AShotoProjectile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Increase speed over time
    CurrentSpeed += 100.0 * DeltaTime;
    CurrentSpeed = FMath::Clamp(CurrentSpeed, InitialSpeed, MaxSpeed);

    // Move the projectile forward
    FVector Movement = GetActorForwardVector() * CurrentSpeed * DeltaTime;
    SetActorLocation(GetActorLocation() + Movement, true);
}

void AShotoProjectile::OnProjectileOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor != this && OtherComp)
    {
        // Apply damage to the hit actor
        UGameplayStatics::ApplyDamage(OtherActor, Damage, nullptr, this, UDamageType::StaticClass());
    }

    // Destroy the projectile upon hitting an actor or any other object
    Destroy();
}