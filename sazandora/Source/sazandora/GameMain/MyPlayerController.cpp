// Fill out your copyright notice in the Description page of Project Settings.

#include "MyPlayerController.h"
#include "Engine/Engine.h"
#include "MyPlayerState.h"
#include "../sazandoraGameMode.h"

// コンストラクタ
AMyPlayerController::AMyPlayerController()
{
	static ConstructorHelpers::FClassFinder<UHUDWidget> widgetclass(TEXT("/Game/ThirdPerson/widget/BP_HUDWidget2"));
	static ConstructorHelpers::FClassFinder<UStartWaitWidget> create_startwait_widgetclass(TEXT("/Game/ThirdPerson/widget/BP_StartWaitWidget"));
	static ConstructorHelpers::FClassFinder<UEndWidget> create_EndWidget_class(TEXT("/Game/UI/End_UI/End_Widget"));

	// HUDwidgetclssが見つかっているか
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

	// StartWaitwidgetclssが見つかっているか
	if (create_startwait_widgetclass.Succeeded())
	{
		// 参照できた場合はクラス情報をwait_widgetに保存する
		StartWaitWidget_class = create_startwait_widgetclass.Class;
	}
	else
	{
		// 参照できない場合はログを出力
		UE_LOG(LogTemp, Log, TEXT("create_startwait_widgetclass is not find..."));
	}


	// StartWaitwidgetclssが見つかっているか
	if (create_EndWidget_class.Succeeded())
	{
		// 参照できた場合はクラス情報をend_widgetに保存する
		EndWidget_class = create_EndWidget_class.Class;
	}
	else
	{
		// 参照できない場合はログを出力
		UE_LOG(LogTemp, Log, TEXT("create_EndWidget_class is not find..."));
	}
}

void AMyPlayerController::Server_RequestStartGame_Implementation()
{
	AsazandoraGameMode* GM = GetWorld()->GetAuthGameMode<AsazandoraGameMode>();

	if (GM)
	{
		GM->Start_Game();
		GM->Set_Gameplay(true);		//GameModeのゲームスタートフラグをtrueにする
		GM->UpdateServerInfoOnAPI();//APIサーバーの情報を更新
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
			// キャラやUIのロードが完了したタイミングで通知
			NotifyLoaded();
		}, 0.2f, false);
	}
}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

}

// Playerがログインした時の処理
void AMyPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// サーバーだけがPlayerStateを初期化する
	if (HasAuthority())
	{
		AMyPlayerState* PS = GetPlayerState<AMyPlayerState>();
		
		// nullチェック
		if (PS)
		{
			UE_LOG(LogTemp, Log, TEXT("[OnPossess] Init PlayerState %s"), *PS->GetName());
			PS->My_State_Initialize();
		}
		else
		{
			// PlayerStateがまだ来ていないことがあるので再試行
			FTimerHandle RetryHandle;
			GetWorld()->GetTimerManager().SetTimer(RetryHandle, [this]()
				{
					if (AMyPlayerState* LatePS = GetPlayerState<AMyPlayerState>())
					{
						UE_LOG(LogTemp, Log, TEXT("[OnPossess Retry] Init PlayerState %s"), *LatePS->GetName());
						LatePS->My_State_Initialize();
					}
				}, 0.1f, false);
		}
	}
}

// PlayerStateがちゃんと紐づけられたタイミングで呼ぶ関数
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

	// 待機画面の作成
	Create_WaitStartWidget();

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

	// 所有権が自分で、HUDWidget_classがnullptrではなければ
	if (IsLocalController() && HUDWidget_class)
	{
		// UHUDWidgetの生成
		HUDWidget_pointer = CreateWidget<UHUDWidget>(this, HUDWidget_class);
		
		// 生成に失敗してなければ
		if (HUDWidget_pointer)
		{
			// 画面に表示
			HUDWidget_pointer->AddToViewport();

			// PlayerStateの紐づけ
			AMyPlayerState* ps = GetPlayerState<AMyPlayerState>();
			HUDWidget_pointer->InitializeWidget(ps);				// Widgetの初期化

			// カメラ操作や入力権限を渡す
			FInputModeGameOnly InputMode;
			SetInputMode(InputMode);
			bShowMouseCursor = false;
		}
	}
}

// 待機画面生成
void AMyPlayerController::Create_WaitStartWidget()
{
	// すでに生成済みならスキップ
	if (wait_widget && wait_widget->IsValidLowLevel() && wait_widget->IsInViewport())
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] wait_widget already exists - skipped creation"), *GetName());
		return;
	}

	// 所有権が自分で、StartWaitWidget_classがnullptrではなければ
	if (IsLocalController() && StartWaitWidget_class)
	{
		// wait_widgetの生成
		wait_widget = CreateWidget<UStartWaitWidget>(this, StartWaitWidget_class);

		// 生成に失敗してなければ
		if (wait_widget)
		{
			// 画面に表示
			wait_widget->AddToViewport();

			// PlayerController側で
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(wait_widget->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

			// 入力をマウスカーソルのみにする
			SetInputMode(InputMode);
			bShowMouseCursor = true;
		}
	}
}

// 終了画面生成
void AMyPlayerController::Create_EndWidget()
{
	// すでに生成済みならスキップ
	if (end_widget && end_widget->IsValidLowLevel() && end_widget->IsInViewport())
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] end_widget already exists - skipped creation"), *GetName());
		return;
	}

	// 所有権が自分で、EndWidget_classがnullptrではなければ
	if (IsLocalController() && EndWidget_class)
	{
		// end_widgetの生成
		end_widget = CreateWidget<UEndWidget>(this, EndWidget_class);

		// 生成に失敗してなければ
		if (end_widget)
		{
			// 画面に表示
			end_widget->AddToViewport();

			// PlayerController側で
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(end_widget->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

			// 入力をマウスカーソルのみにする
			SetInputMode(InputMode);
			bShowMouseCursor = true;
		}
	}
}

// ゲーム開始関数
void AMyPlayerController::Client_StartGame_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("[Client_StartGame] %s | IsLocal=%d | NetMode=%d | Role=%d | RemoteRole=%d"),
		*GetName(), IsLocalController(), (int)GetNetMode(), (int)GetLocalRole(), (int)GetRemoteRole());

	if (!IsLocalController())
	{
		return;
	}

	// wait_widgetが存在する場合削除する
	if (wait_widget)
	{
		wait_widget->RemoveFromParent();
		wait_widget = nullptr;
	}

	// playerstateの取得
	// 少し遅らせてHUDを生成（レプリケーション完了待ち）
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, [this]()
		{
			// playerstateがnullptrかどうかチェック
			if (AMyPlayerState* player_state = GetPlayerState<AMyPlayerState>())
			{
				Create_HUDWidget();

				UE_LOG(LogTemp, Warning, TEXT("Create HUDWidget"));
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

// ゲーム終了処理
void AMyPlayerController::Client_EndGame_Implementation(bool is_clear)
{
	UE_LOG(LogTemp, Warning, TEXT("[Client_EndGame] %s | IsLocal=%d | NetMode=%d | Role=%d | RemoteRole=%d"),
		*GetName(), IsLocalController(), (int)GetNetMode(), (int)GetLocalRole(), (int)GetRemoteRole());

	if (!IsLocalController())
	{
		return;
	}

	// HUDWidget_pointerが存在する場合削除する
	if (HUDWidget_pointer)
	{
		HUDWidget_pointer->RemoveFromParent();
		HUDWidget_pointer = nullptr;
	}

	if (is_clear)
	{
		UE_LOG(LogTemp, Warning, TEXT("test is_player_clear is true"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("test is_player_clear is false"));
	}

	// playerstateの取得
	// 少し遅らせてHUDを生成（レプリケーション完了待ち）
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, [this, is_clear]()
		{
			// playerstateがnullptrかどうかチェック
			if (AMyPlayerState* player_state = GetPlayerState<AMyPlayerState>())
			{
				if (is_clear)
				{
					UE_LOG(LogTemp, Warning, TEXT("is_player_clear is true"));
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("is_player_clear is false"));
				}

				// EndWidgetの生成
				Create_EndWidget();

				// クリアしたかどうかをwidgetに伝える
				//Set_EndWidget_Text(is_clear);
				end_widget->Set_ClearFlg(is_clear);

				UE_LOG(LogTemp, Warning, TEXT("Create EndWidget"));
			}
			else
			{
				// 少し後で再試行(ロード遅延対策)
				FTimerHandle Retryhandle;

				GetWorldTimerManager().SetTimer(Retryhandle, [this, is_clear]()
					{
						//再実行
						Client_EndGame(is_clear);

						UE_LOG(LogTemp, Error, TEXT("Client_EndGame restart"));

					}, 0.1f, false);

				return;
			}

		}, 0.3f, false);
}

void AMyPlayerController::Set_EndWidget_Text(bool flg)
{
	end_widget->Set_ClearFlg(flg);
}

// ウィジェットの更新
void AMyPlayerController::Refresh_UI()
{
	// ローカルコントローラーのUI更新だけ行う
	if (!IsLocalController())
	{
		return;
	}

	if (HUDWidget_pointer)
	{
		HUDWidget_pointer->RefreshUI();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Client_UpdateBuyList: HUDWidget_pointer is null"));
	}
}