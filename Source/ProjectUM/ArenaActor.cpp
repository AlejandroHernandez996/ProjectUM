#include "ArenaActor.h"
#include "Components/BoxComponent.h"
#include "ProjectUMCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"

DECLARE_LOG_CATEGORY_EXTERN(LogArenaActor, Log, All);
DEFINE_LOG_CATEGORY(LogArenaActor);

AArenaActor::AArenaActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create the collision component
	CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComponent"));
	RootComponent = CollisionComponent;

	// Bind collision events
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AArenaActor::OnPlayerEnterArena);
	CollisionComponent->OnComponentEndOverlap.AddDynamic(this, &AArenaActor::OnPlayerExitArena);

	SetReplicates(true);
	SetReplicateMovement(false);
}

void AArenaActor::BeginPlay()
{
	Super::BeginPlay();
}

void AArenaActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsFightStarted)
	{
		TArray<AProjectUMCharacter*> PlayersToRemove;

		// Check if a player is out of bounds and grounded
		for (AProjectUMCharacter* Player : OutOfBoundsPlayers)
		{

			if (!Player->GetCharacterMovement()->IsFalling())
			{
				LogMessage(FString::Printf(TEXT("Player %s is out of bounds and grounded."), *Player->GetName()));
				PlayersToRemove.Add(Player);
			}

		}

		// Remove the players from the out-of-bounds set
		for (AProjectUMCharacter* Player : PlayersToRemove)
		{
			OutOfBoundsPlayers.Remove(Player);
			LogMessage(FString::Printf(TEXT("Player %s removed from out of bounds players and will be handled accordingly"), *Player->GetName()));
			HandlePlayerOutOfBounds(Player);

		}


		// End fight if one of the players is dead
		for (AProjectUMCharacter* Player : PlayersInFight)
		{
			if (Player->GetCurrentHealth() <= 0.0f) {
				EndFight(Player);
			}
		}
	}
}

void AArenaActor::OnRep_CountdownTimer()
{
	OnArenaTimerUpdate.Broadcast({ CountdownTimer });
}

void AArenaActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AArenaActor, CountdownTimer);
}

void AArenaActor::OnPlayerEnterArena(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority())
	{
		AProjectUMCharacter* Player = Cast<AProjectUMCharacter>(OtherActor);
		if (!Player || PlayersInArena.Contains(Player)) return;
		if (bIsFightStarted && !PlayersInFight.Contains(Player)) {
			FVector LaunchDirection = (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal();
			float LaunchMagnitude = 1000.0f; // Adjust the launch magnitude as needed
			Player->LaunchCharacter(LaunchDirection * LaunchMagnitude, true, true);
			return;
		}

		PlayersInArena.Add(Player);
		LogMessage(FString::Printf(TEXT("Player %s entered the arena."), *Player->GetName()));
		if (bIsFightStarted) {
			OutOfBoundsPlayers.Remove(Player);
			HandlePlayerInBounds(Player);
		}
		else
		{
			// Check if all players are in the arena to start the fight
			if (AreAllPlayersInArena())
			{
				StartCountdown();
			}
		}
	}
}

void AArenaActor::OnPlayerExitArena(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (HasAuthority())
	{
		AProjectUMCharacter* Player = Cast<AProjectUMCharacter>(OtherActor);
		if (!Player || !PlayersInArena.Contains(Player)) return;
		PlayersInArena.Remove(Player);
		LogMessage(FString::Printf(TEXT("Player %s exited the arena."), *Player->GetName()));

		if (bIsFightStarted) {

			LogMessage(FString::Printf(TEXT("Player %s added to out of bounds players set."), *Player->GetName()));
			OutOfBoundsPlayers.Add(Player);
		}
		else
		{
			GetWorldTimerManager().ClearTimer(CountdownTimerHandle);
		}
	}
}

void AArenaActor::StartFight()
{
	bIsFightStarted = true;
	// Log that the fight started
	LogMessage(TEXT("Fight starts!"));
	for (AProjectUMCharacter* Character : PlayersInArena) {
		PlayersInFight.Add(Character);
		FTimerHandle OutOfBoundsTimer;
		OutOfBoundsTimerHandlers.Add(Character, OutOfBoundsTimer);
	}
}

void AArenaActor::EndFight(AProjectUMCharacter* LosingPlayer)
{
	bIsFightStarted = false;

	// Log that a player lost the fight
	LogMessage(FString::Printf(TEXT("Player %s lost the fight."), *LosingPlayer->GetName()));

	// Reset the arena state
	ResetArenaState(LosingPlayer);
}

void AArenaActor::StartCountdown()
{
	bIsCountdownStarted = true;
	CountdownTimer = 3.0f;

	// Log the countdown start
	LogMessage(TEXT("Fight countdown started."));

	// Schedule the countdown tick
	GetWorldTimerManager().SetTimer(CountdownTimerHandle, this, &AArenaActor::CountdownTick, 1.0f, true);
}

void AArenaActor::CountdownTick()
{
	if (bIsFightStarted) return;
	if (CountdownTimer > 0.0f)
	{
		// Log the countdown timer
		CountdownTimer--;
	}
	else
	{
		// Stop the countdown timer
		GetWorldTimerManager().ClearTimer(CountdownTimerHandle);

		StartFight();
	}
}

bool AArenaActor::AreAllPlayersInArena() const
{
	return PlayersInArena.Num() == 2;
}

void AArenaActor::HandlePlayerOutOfBounds(AProjectUMCharacter* Player)
{
	LogMessage(FString::Printf(TEXT("Player %s timer for out of bounds starting."), *Player->GetName()));
	// Start the out of bounds timer for the player
	GetWorldTimerManager().SetTimer(OutOfBoundsTimerHandlers[Player], [this, Player]()
		{
			if (!bIsFightStarted) return;
			// Player has been out of bounds for the required duration, handle loss condition here
			LogMessage(FString::Printf(TEXT("Player %s lost due to being out of bounds for too long."), *Player->GetName()));
			EndFight(Player);
		}, 3.0f, false);
}

void AArenaActor::HandlePlayerInBounds(AProjectUMCharacter* Player)
{
	// Stop the out of bounds timer for the player
	LogMessage(FString::Printf(TEXT("Player %s is inbounds so clearing timer for out of bounds"), *Player->GetName()));
	GetWorldTimerManager().ClearTimer(OutOfBoundsTimerHandlers[Player]);
}

void AArenaActor::ResetArenaState(AProjectUMCharacter* LosingPlayer)
{
	bIsFightStarted = false;
	bIsCountdownStarted = false;
	CountdownTimer = 10.0f;
	PlayersInArena.Remove(LosingPlayer);
	OutOfBoundsTimerHandlers.Empty();

	// Log that the arena state is reset
	LogMessage(TEXT("Arena state reset."));
}

void AArenaActor::LogMessage(const FString& Message)
{
	UE_LOG(LogArenaActor, Log, TEXT("%s"), *Message);
}