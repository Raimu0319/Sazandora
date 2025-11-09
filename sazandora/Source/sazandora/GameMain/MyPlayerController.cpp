// Fill out your copyright notice in the Description page of Project Settings.

#include "MyPlayerController.h"
#include "Engine/Engine.h"
#include "MyPlayerState.h"
#include "../sazandoraGameMode.h"

// コンストラクタ
AMyPlayerController::AMyPlayerController()
{
	static ConstructorHelpers::FClassFinder<UHUDWidget> widgetclass(TEXT("/Game/ThirdPerson/widget/BP_HUDWidget2"));

	// widgetclssが見つかっているか
	if (widgetclass.Succeeded())
	{
		// 参照できた場合はクラス情報をHUDWidget_classに保存する
		HUDWidget_class = widgetclass.Class;
	}
	else
	{
		// 参照できない場合はログを出力
		UE_LOG(LogTemp, Log, TEXT("HUDWidget is not find..."));
	}
}

// PlayerStateがレプリケートされた時に呼び出されるコールバック関数
void AMyPlayerController::OnRep_PlayerState()
{
	// 親クラスの関数を呼び出し
	Super::OnRep_PlayerState();

	// クライアントのみでの実行
	//if (IsLocalController())
	//{
	//	// PlayerStateが生成されたタイミングで通知
	//	NotifyLoaded();
	//}

	FString PSName = GetPlayerState<AMyPlayerState>()
		? GetPlayerState<AMyPlayerState>()->GetName()
		: TEXT("null");

	UE_LOG(LogTemp, Log, TEXT("[PC OnRep_PlayerState] %s | IsLocal=%d | PlayerState=%s"),
		*GetName(),
		IsLocalController() ? 1 : 0,
		*PSName);

	// 対象のplayerControllerが自分のものか
	if (IsLocalController())
	{
		// PlayerStateの所有者として自分を明示的にセット
		if (AMyPlayerState* PS = GetPlayerState<AMyPlayerState>())
		{
			// 参照した所有者が自分と違う場合
			if (PS->GetOwner() != this)
			{
				// 所有者を自分に変更する
				PS->SetOwner(this);
				UE_LOG(LogTemp, Warning, TEXT("[Fix] PlayerState owner corrected: %s now owned by %s"),
					*PS->GetName(), *GetName());
			}
		}

		FTimerHandle DelayHandle;
		GetWorldTimerManager().SetTimer(DelayHandle, [this]()
		{
			NotifyLoaded();
		}, 0.2f, false);
	}
}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// キャラやUIのロードが完了したタイミングで通知
	//NotifyLoaded();

	UE_LOG(LogTemp, Log, TEXT("[PC BeginPlay] %s | NetMode=%d | IsLocal=%d | Role=%d | RemoteRole=%d"),
		*GetName(), (int)GetNetMode(), IsLocalController(), (int)GetLocalRole(), (int)GetRemoteRole());
}

void AMyPlayerController::NotifyLoaded()
{
	// 一度実行済みならスキップする
	if (bAlreadyNotified)
	{
		return;
	}

	// playerstateの取得
	AMyPlayerState* player_state = GetPlayerState <AMyPlayerState>();

	// playerstateがnullptrかどうかチェック
	if (!player_state || player_state->GetOwner() != this)
	{
		// 所有者がまだ自分になっていない場合は再試行
		GetWorldTimerManager().SetTimerForNextTick(this, &AMyPlayerController::NotifyLoaded);

		// playerstateがまだ準備できていないというログ
		UE_LOG(LogTemp, Warning, TEXT("[%s] NotifyLoaded skipped: PlayerState not ready"), *GetName());
		return;
	}

	// サーバーのみで実行
	//if (!IsLocalController())
	//{
	//	player_state->My_State_Initialize();
	//}

	// playerstateがちゃんと同期（ロード）完了フラグ
	bAlreadyNotified = true;

	// serverへ自分のロード完了を通知
	player_state->Server_SetLoaded(true);

	// Notify関数が呼ばれたログ
	UE_LOG(LogTemp, Log, TEXT("[%s] NotifyLoaded() called"), *GetName());
}

// HUDWidgetの作成
void AMyPlayerController::Create_HUDWidget()
{
	// すでに生成済みならスキップ
	if (HUDWidget_pointer && HUDWidget_pointer->IsValidLowLevel() && HUDWidget_pointer->IsInViewport())
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] HUDWidget already exists - skipped creation"), *GetName());
		return;
	}

	if (IsLocalController() && HUDWidget_class)
	{
		UHUDWidget* HUDWidget = CreateWidget<UHUDWidget>(this, HUDWidget_class);
		
		if (HUDWidget)
		{
			HUDWidget->AddToViewport();

			// PlayerStateの紐づけ
			AMyPlayerState* ps = GetPlayerState<AMyPlayerState>();
			HUDWidget->InitializeWidget(ps);
		}
	}
}

void AMyPlayerController::Client_StartGame_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("[Client_StartGame] %s | IsLocal=%d | NetMode=%d | Role=%d | RemoteRole=%d"),
		*GetName(), IsLocalController(), (int)GetNetMode(), (int)GetLocalRole(), (int)GetRemoteRole());

	if (!IsLocalController())
	{
		return;
	}

	// playerstateの取得
	// 少し遅らせてHUDを生成（レプリケーション完了待ち）
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, [this]()
		{
			// playerstateがnullptrかどうかチェック
			if (AMyPlayerState* player_state = GetPlayerState<AMyPlayerState>())
			{
				player_state->My_State_Initialize();

				Create_HUDWidget();

				UE_LOG(LogTemp, Warning, TEXT("[%s] NotifyLoaded skipped: PlayerState not ready"), *GetName());
			}
			else
			{
				// 少し後で再試行(ロード遅延対策)
				FTimerHandle Retryhandle;

				GetWorldTimerManager().SetTimer(Retryhandle, [this]()
					{
						//再実行
						Client_StartGame();
					}, 0.1f, false);

				return;
			}

		}, 0.3f, false);
}