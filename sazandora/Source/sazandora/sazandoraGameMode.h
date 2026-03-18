// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Public/NetWork/LogInWidget.h"
#include "GameFramework/GameModeBase.h"
#include "sazandoraGameMode.generated.h"

class AMyPlayerState;

UCLASS(minimalapi)
class AsazandoraGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:

	virtual void BeginPlay() override;

public:
	AsazandoraGameMode();

	UFUNCTION()
	void CheckAllPlayersLoaded();

	//UFUNCTION(NetMulticast, Reliable)
	//void Multicast_StartGame();

	virtual void PostLogin(APlayerController* NewPlayer) override;

	// Playerがクリアしているかどうか
	UFUNCTION()
	void ClearCheck(AMyPlayerState* p);

	// 
	UFUNCTION()
	void Start_Game();
	
	UFUNCTION()
	void Set_Gameplay(bool flag);

	void UpdateServerInfoOnAPI();
protected:
	UPROPERTY()
	bool start_flg = false;

	virtual AActor* FindPlayerStart_Implementation(AController* player, const FString& IncomingName) override;
	virtual void Logout(AController* Exiting) override;

private:
	int32 NextPlayerIndex = 0;
	bool gameplay  = false;

	void RegisterServerToAPI();

	float Time_count;
	FString Get_IPAddress();
	FString URL; //APIサーバー用のURL格納変数

	TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> HeartbeatRequest;
};