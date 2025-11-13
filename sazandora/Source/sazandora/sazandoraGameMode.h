// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "sazandoraGameMode.generated.h"

class AMyPlayerState;

UCLASS(minimalapi)
class AsazandoraGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AsazandoraGameMode();

	UFUNCTION()
	void CheckAllPlayersLoaded();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StartGame();

	virtual void PostLogin(APlayerController* NewPlayer) override;

	// Playerがクリアしているかどうか
	UFUNCTION()
	void ClearCheck(AMyPlayerState* p);

	UFUNCTION()
	void Start_Game();

protected:
	UPROPERTY()
	bool start_flg = false;

	virtual AActor* FindPlayerStart_Implementation(AController* player, const FString& IncomingName) override;

private:
	int32 NextPlayerIndex = 0;
};