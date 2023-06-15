#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArenaActor.generated.h"

class UBoxComponent;
class AProjectUMCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FArenaTimersUpdateEvent, const TArray<float>&, _Timers);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOutboundTimersUpdateEvent, const TArray<float>&, _Timers);

UCLASS()
class PROJECTUM_API AArenaActor : public AActor
{
	GENERATED_BODY()

public:
	AArenaActor();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* CollisionComponent;

	UPROPERTY(BlueprintAssignable, Category = "Stat Update")
		FArenaTimersUpdateEvent OnArenaTimerUpdate;

	UPROPERTY(BlueprintAssignable, Category = "Stat Update")
		FOutboundTimersUpdateEvent OnOutboundTimersUpdate;

	UFUNCTION()
	void OnPlayerEnterArena(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnPlayerExitArena(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void StartFight();
	void EndFight(AProjectUMCharacter* LosingPlayer);
	void ResetArenaState(AProjectUMCharacter* LosingPlayer);

	void StartCountdown();
	void CountdownTick();
	void OutOfBoundsTick(int32 PlayerIndex);

	bool AreAllPlayersInArena() const;
	bool IsPlayerOutOfBounds(AProjectUMCharacter* Player) const;
	void HandlePlayerOutOfBounds(AProjectUMCharacter* Player, int32 PlayerIndex);
	void HandlePlayerInBounds(AProjectUMCharacter* Player);

	void LogMessage(const FString& Message);

	UPROPERTY(VisibleInstanceOnly, Category = "Arena")
	bool bIsFightStarted;

	UPROPERTY(VisibleInstanceOnly, Category = "Arena")
	bool bIsCountdownStarted;

	UPROPERTY(VisibleInstanceOnly, Category = "Arena", ReplicatedUsing = OnRep_CountdownTimer)
		float CountdownTimer = -1.0f;

	UFUNCTION()
		void OnRep_CountdownTimer();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditDefaultsOnly, Category = "Arena")
	float OutOfBoundsTimerDuration;

	UPROPERTY(VisibleInstanceOnly, Category = "Arena", ReplicatedUsing = OnRep_OutboundTicks)
		TArray<float> OutboundTicks = {-1.0f, -1.0f };

	UFUNCTION()
		void OnRep_OutboundTicks();

	FTimerHandle CountdownTimerHandle;
	TMap<AProjectUMCharacter*, FTimerHandle> OutOfBoundsTimerHandlers;
	TArray<AProjectUMCharacter*> PlayersInArena;
	TArray<AProjectUMCharacter*> PlayersInFight;
	TSet<AProjectUMCharacter*> OutOfBoundsPlayers;
};
