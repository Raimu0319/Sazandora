// Fill out your copyright notice in the Description page of Project Settings.


#include "../Public/GameMain/Main_Character.h"
#include "../Public/GameMain/NPC_Character.h"
#include "TimerManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/InputComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "../Public/GameMain/MyPlayerState.h"
#include "../Public/GameMain/MyPlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AMain_Character::AMain_Character()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	//カプセルコリジョン
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	//変数の初期化
	b_IsJump_ButtonHold = false;	//ジャンプボタンを押しているかどうか
	b_IsJump = false;				//ジャンプをしているかどうか
	Jump_HoldTime = 0.0f;			//押し続けた時間
	Max_Jump_HoldTime = 0.3f;		//最大押し続け時間(秒)
	Min_Jump_Strength = 300.0f;		//最低ジャンプ力
	Add_Jump_Boost = 150.0f;			//追加ジャンプ力

	b_IsDash = false;				//ダッシュしているかどうか
	Dash_HoldTime = 0.0f;			//ダッシュキーのホールド時間
	Dash_Speed = 0.0f;				//ダッシュ速度
	Max_Dash_Speed = 850.0f;		//最大ダッシュ速度

	// カメラ用のSpringArm（カメラアーム）を作成
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 300;	//カメラ距離
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SocketOffset = FVector(30.0f, 90.0f, 30.0f);		//カメラの上下左右の調整

	// カメラを作成
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp);

	// NPCのポインタ
	TargetNPC = nullptr;
	Is_Talk = false;

	// メッシュを作成
	GetMesh()->SetupAttachment(RootComponent);
	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));	//座標微調整
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));	//向きを調整

	// skeletal Mesh(キャラの見た目)を読み込む
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshObj(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny"));
	
	// Meshが正しく読み込めているかどうか
	// Suceeded()がだ正しくメッシュを作成できているかどうかを確認する返り値がbool型の関数
	if (MeshObj.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshObj.Object);
	}

	// アニメーションBPを設定（任意）
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBP(TEXT("/Game/Characters/Mannequins/Animations/ABP_Quinn"));
	
	if (AnimBP.Succeeded())
	{
		GetMesh()->SetAnimInstanceClass(AnimBP.Class);
	}

	// カメラ操作をプレイヤーに渡す
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	AutoPossessPlayer = EAutoReceiveInput::Disabled;

	GuideArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("GuideArrow"));
	GuideArrow->SetupAttachment(RootComponent);
	GuideArrow->SetRelativeLocation(FVector(0, 0, 100)); // 頭の上
	GuideArrow->SetHiddenInGame(false);
	GuideArrow->SetVisibility(true);
}

// Called when the game starts or when spawned
void AMain_Character::BeginPlay()
{
	Super::BeginPlay();

	// NPCの検索
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANPC_Character::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		if (ANPC_Character* NPC = Cast<ANPC_Character>(Actor))
		{
			NPCList.Add(NPC);
		}
	}

	// タイムハンドルの作成
	FTimerHandle TimerHandle;

	// ゴールアクターが生成されていない状態での参照を防ぐため１フレームずらして実行
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,this, &AMain_Character::FindMyGoalPoint,0.3,false
	);
}

// Called every frame
void AMain_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())
	{
		// ダッシュ処理
		AMain_Character::Dash(DeltaTime);

		// ジャンプ処理
		AMain_Character::Custom_Jump(DeltaTime);

		/*UE_LOG(LogTemp, Warning, TEXT("SERVER Character Pos: %s"),
			*GetActorLocation().ToString());*/
	}

	// MyPlayerStateの入手
	AMyPlayerState* ps = GetPlayerState<AMyPlayerState>();

	// nullチェック
	if (!ps)
	{
		return;
	}

	// プレイヤーがお使いを終わらせているかどうか
	if (ps->Is_Cleared())
	{
		// 終わらせてる場合ゴール地点の探索
		if (MyGoalPoint && GuideArrow)
		{
			FVector Dir = MyGoalPoint->GetActorLocation() - GetActorLocation();		// 座標計算
			FRotator Rot = Dir.Rotation();											// 角度の計算

			GuideArrow->SetWorldRotation(Rot);			// 向きの変更
		}
	}
	else		// 終わらせていない場合
	{

		// 一番近いNPCのポインタを取得する
		NearestNPC = FindNearestNPC_FromList();

		// NPCのポインタとArrowのポインタが存在している場合
		if (NearestNPC && GuideArrow)
		{
			FVector Dir = NearestNPC->GetActorLocation() - GetActorLocation();		// 座標計算
			FRotator Rot = Dir.Rotation();											// 角度の計算

			GuideArrow->SetWorldRotation(Rot);				// 向きの変更
		}
	}

	// NPCとの距離計算
	AMain_Character::CheckInteract();
}

// Called to bind functionality to input
void AMain_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// ジャンプ入力
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMain_Character::OnJumpPressed);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AMain_Character::OnJumpReleased);

	// ダッシュ入力
	//PlayerInputComponent->BindAction("Dash", IE_Pressed, this, &AMain_Character::On_Dash_Pressed);
	//PlayerInputComponent->BindAction("Dash", IE_Released, this, &AMain_Character::On_Dash_Released);

	// 会話キー
	PlayerInputComponent->BindAction("Talk", IE_Pressed, this, &AMain_Character::Try_Talk);

	// 移動入力のバインド
	PlayerInputComponent->BindAxis("MoveForward", this, &AMain_Character::MoveForward);
	PlayerInputComponent->BindAxis("MoveR/L", this, &AMain_Character::MoveRight);

	// カメラ操作のバインド
	PlayerInputComponent->BindAxis("MouseX", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("MouseY", this, &APawn::AddControllerPitchInput);
}

// ジャンプボタンが押された瞬間の処理
void AMain_Character::OnJumpPressed()
{
	// サーバー以外が関数を呼んだ場合
	if (!HasAuthority())
	{
		// サーバーで実行するようにする
		Server_JumpPressed();
		return;
	}

	// サーバー側が関数を呼んだ場合はそのまま実行
	ExecJumpStart();
}

// サーバーでジャンプ開始関数を呼ぶ関数
void AMain_Character::Server_JumpPressed_Implementation()
{
	// ジャンプ開始関数
	ExecJumpStart();
}

// サーバーでジャンプ停止関数を呼ぶ関数
void AMain_Character::Server_JumpReleased_Implementation()
{
	// ジャンプ停止関数
	ExecJumpStop();
}

// ジャンプ開始関数
void AMain_Character::ExecJumpStart()
{
	//	空中にいる場合はジャンプをしないようにする
	if (GetCharacterMovement()->IsFalling())
	{
		//	空中にいる場合は処理を行わない
		return;
	}

	b_IsJump_ButtonHold = true;		//	ボタンが押されているのでtrue
	b_IsJump = true;				//	押されたということはジャンプしているのでtrue
	Jump_HoldTime = 0.0f;			//	押された時に初期化

	// LaunchCharacterは特定の地点に瞬時に移動させる関数（今回は慣性（初速度）を適用する形でジャンプの動きを再現）
	//（LaunchCharacter( FVector LaunchVelocity, bool bXYOverride, bool bZOverride )）
	// LaunchVelocity = 与える速度ベクトル　
	// bXYOverride = XY成分（水平）をfalseで既存の速度に加算し、trueで既存の値を書き換える
	// bZOverride = Z成分（垂直）を既存のZ速度で上書きする(trueの場合現在のZが消えて新しいZに書き変わる）
	LaunchCharacter(FVector(0, 0, Min_Jump_Strength), false, true);
	
	/*Jump();
	GetCharacterMovement()->Velocity.Z = Min_Jump_Strength;*/
}

// ジャンプ停止関数
void AMain_Character::ExecJumpStop()
{
	b_IsJump_ButtonHold = false;		//ボタンが離されたためfalseにする
}

//ジャンプボタンが離された時
void  AMain_Character::OnJumpReleased()
{
	// サーバー以外が関数を呼んだ場合
	if (!HasAuthority())
	{
		// サーバーで実行するようにする
		Server_JumpReleased();
		return;
	}

	// サーバー側が関数を呼んだ場合はそのまま実行
	ExecJumpStop();
}

// ジャンプ処理
void AMain_Character::Custom_Jump(float DeltaTime)
{
	// ジャンプボタンが離されてない状態かつジャンプ中なら
	if (b_IsJump_ButtonHold && b_IsJump)
	{
		//	押し続けている時間
		Jump_HoldTime += DeltaTime;

		//	押し続けている時間が最大ホールド時間以内なら
		if (Jump_HoldTime <= Max_Jump_HoldTime)
		{
			//	押している時間がどのくらいに達しているかを0～1の割合で表す
			float Alpha = Jump_HoldTime / Max_Jump_HoldTime;

			// Fmath::Lerpは２つの値の間の線形補間を行う
			// Alphaが0.5ならちょうどMin_Jump_StrengthとMax_Jump_Strengthの中間の数値になる

			// Fmath::Clamp(変数, 最小値, 最大値)で使える関数
			// 変数の値が最小値以下なら最小値を返し、最大値以上なら最大値を返す。範囲内ならそのままの値を返す
			Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);

			// イーズアウト補間は、最初は早く、徐々に減速して目的地に到達するという動き
			//	開始時は補間の開始地点から急速に移動します
			//	補間の終了地点に近づくにつれて速度が減速し、滑らかに停止する
			/*　引数について	<FMath::InterpEaseOut(A,B,alpha,Exp);>
				InterpEaseOut（開始値: 補間を開始する値(A)、終了値:補間が目標とする値(B)、
					alpha(補間率):補間の進行度を0.0から1.0までの値で指定する。(0.0の時は開始値が返され、1.0の時は終了値が返される)、
					Exp(指数):イーズアウト曲線の曲がり具合を調整する値、この値が大きいほど、減速がより急になり、最終的な値にゆっくりと近づく)
			*/
			float PowerCurve = FMath::InterpEaseOut(1.0f, 0.0f, Alpha, 3.0f);		//PowerCurveには1.0f～0.0fの値が入る➘（Alpha値が0なら１が、Alpha値が1なら0)

			//	追加ジャンプ力にPowerCurve(1.0f ～ 0.0f)を掛ける
			float ExtraPower = Add_Jump_Boost * PowerCurve;

			//	ジャンプ力の加算
			FVector NewVelocity = GetCharacterMovement()->Velocity;		//移動力の取得
			NewVelocity.Z += ExtraPower;			//Z方向へ1フレーム分の加速
			GetCharacterMovement()->Velocity = NewVelocity;			//移動量の変更
			GetCharacterMovement()->bApplyGravityWhileJumping = false;
		}
		else
		{
			// 最大時間を超えたらジャンプ終了
			b_IsJump_ButtonHold = false;
		}
	}
}

// 前後移動処理
void AMain_Character::MoveForward(float value)
{
	//	対応のキーが入力されていて、入力値が0.0fではないとき
	if (Controller && value != 0.0f)
	{
		//	プレイヤーが操作しているカメラの向きを取得
		//	Rotationはピッチ（上下）・ヨー（左右）・ロール（傾き）をもつ回転情報　例）カメラが右を剥いていた場合はRotation.Yawの角度は右向きになる
		const FRotator Rotation = Controller->GetControlRotation();

		//　水平方向の回転を抽出
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		//	現在の向きから前方向(X軸方向)のベクトルを算出することでカメラが今向いている方向がわかる
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		//	Direction(どの方向)に、value(どれだけ動くか)を指定して実行する
		AddMovementInput(Direction, value);
	}
}

// 左右移動処理
void AMain_Character::MoveRight(float value)
{
	if (Controller && value != 0.0f)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, value);
	}
}

// ダッシュボタンが押された時
void AMain_Character::On_Dash_Pressed()
{
	b_IsDash = true;
	Dash_HoldTime = 0.0f;
	Server_StartDash();
}

// ダッシュボタンが離された時
void AMain_Character::On_Dash_Released()
{
	b_IsDash = false;
	Dash_Speed = 0.0f;
}


void AMain_Character::Server_StartDash_Implementation()
{
	b_IsDash = true;
	Dash_HoldTime = 0.0f;
}

// ダッシュ処理
void AMain_Character::Dash(float DeltaTime)
{
	// ダッシュ状態かどうか
	if (b_IsDash)
	{
		// ホールド時間の計測
		Dash_HoldTime += DeltaTime;

		// ダッシュ状態がtrueかつDash_SpeedがMax_Dash_Speedより小さい時
		if (Dash_Speed < Max_Dash_Speed)
		{
			// Dash_HoldTimeを/0.5で割った時に0.0f～1.0fの間になるか	例）Dash_HoldTimeが0.5の場合は1になるので1.0fが返ってくる。0.25の場合は0.5が返ってくる
			float alpha = FMath::Clamp(Dash_HoldTime / 0.5f, 0.0f, 1.0f);

			// イーズ関数を使用して徐々に速度が早くなる計算
			float PowerCurve = FMath::InterpEaseOut(0.0f, 1.0f, alpha, 1.0f);

			//　毎フレームごとに加算される数値 ＝ 基礎速度 * PowerCurve(0.0f ～1.0f) * １フレームの経過時間
			Dash_Speed = 100.0f * PowerCurve * DeltaTime;

			//FVector NewVelocity = GetCharacterMovement()->Velocity;		//移動力の取得
			//NewVelocity.Y += ExtraPower * DeltaTime * 60.0f;			//Z方向へ1フレーム分の加速
			//GetCharacterMovement()->Velocity = NewVelocity;				//移動量の変更
		}
		else if (b_IsDash == true && Dash_Speed > Max_Dash_Speed)
		{
			Dash_Speed = Max_Dash_Speed;
		}

		// 実際に移動速度を反映
		float walk_speed = GetCharacterMovement()->MaxWalkSpeed + Dash_Speed;

		if (walk_speed <= Max_Dash_Speed)
		{
			GetCharacterMovement()->MaxWalkSpeed = walk_speed + Dash_Speed;
		}
		else
		{
			GetCharacterMovement()->MaxWalkSpeed = Max_Dash_Speed;
		}
	}
	else
	{
		// ダッシュが終了したら速度を戻す
		GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	}
}

bool AMain_Character::Server_Talk_NPC_Validate()
{
	return true;
}

// サーバーで会話イベントを起動する
void AMain_Character::Server_Talk_NPC_Implementation()
{
	// NPCのポインタが入っていて、話しかけられる状態か？
	//if (TargetNPC && Is_Talk)
	//{
	//	// 会話イベントの開始
	//	TargetNPC->Talk_Event(this);
	//}
}

void AMain_Character::Set_Talk_Flg(bool talk_flg)
{
	this->Is_Talk = talk_flg;
}

void AMain_Character::Server_SetInteractNPC_Implementation(ANPC_Character* NPC, bool is_can_interact)
{
	// NPCのポインタをここで保存したりnullptrに変更する
	CurrentInteractNPC = NPC;

	// ログの標示
	if (CurrentInteractNPC)
	{
		UE_LOG(LogTemp, Log, TEXT("CurrentInteractNPC is true"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("CurrentInteractNPC is false"));
	}

	// AMyPlayerControllerの取得に成功したらAボタンの表示を変更する
	if (AMyPlayerController* pc = Cast<AMyPlayerController>(GetController()))
	{
		pc->A_Button_SetVisibility(is_can_interact);
	}
}

// ライントレースで会話可能範囲にいるかどうかの判定
void AMain_Character::CheckInteract()
{
	// ライントレースの範囲にNPCがいるかどうか
	bool Is_hit_npc = false;

	int32 ViewportX, ViewportY;		// 画面（ビューポート）の横・縦サイズ（ピクセル）
	FVector WorldPos;				// 線の始点
	FVector WorldDir;				// 選の向き（長さは１）
	APlayerController* P_Controller = GetController<APlayerController>();
	if (!P_Controller)
	{
		return;
	}

	P_Controller->GetViewportSize(ViewportX, ViewportY);	// 画面サイズの取得
	
	// 画面中央からカメラを通る一本の線を取得
	P_Controller->DeprojectScreenPositionToWorld(
	ViewportX * 0.5f, 
	ViewportY * 0.5f,
	WorldPos,
	WorldDir);

	FHitResult hit;
	FVector Start = WorldPos;
	FVector End = Start + WorldDir * 550.0f;


	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if(GetWorld()->SweepSingleByChannel(
		hit,
		Start,
		End,
		FQuat::Identity,
		ECC_Visibility,
		FCollisionShape::MakeSphere(40.0f), // ← 太さ
		Params
	))
	{
		// ヒットしているオブジェクトがNPCならポインタを取得
		ANPC_Character* hit_npc_ptr = Cast<ANPC_Character>(hit.GetActor());

		// npcが検出されたら
		if (hit_npc_ptr)
		{
			// ポインタをセット
			/*CurrentInteractNPC = hit_npc;
			/*UE_LOG(LogTemp, Log, TEXT("find is npc "))

			// MyPlayerStateの入手
			AMyPlayerState* ps = GetPlayerState<AMyPlayerState>();
			UE_LOG(LogTemp, Log, TEXT("AMyPlayerState* ps = GetPlayerState<AMyPlayerState>();"));

			// NULLチェック
			if (!ps)
			{
				UE_LOG(LogTemp, Log, TEXT("ps->Set_Is_button_visible(true) is not play"));
				return;
			}

			// Aボタンの表示変更
			ps->Set_Is_button_visible(true);
			UE_LOG(LogTemp, Log, TEXT("ps->Set_Is_button_visible(true)"));*/

			Is_hit_npc = true;
		}
		
		// 差分があった場合のみRPCを実行
		if(Is_LocalCan_Innteract != Is_hit_npc)
		{
			Is_LocalCan_Innteract = Is_hit_npc;
			// サーバー側でCurrentInteractNPCやボタンの表示を変更するように通知する
			Server_SetInteractNPC_Implementation(hit_npc_ptr,Is_hit_npc);
		}
	}
}

// 会話開始リクエスト
void AMain_Character::Try_Talk()
{
	// 会話するNPCが範囲にいるか
	if (CurrentInteractNPC)
	{
		UE_LOG(LogTemp, Log, TEXT("talk start npc "));

		// サーバー経由で話しかける
		Server_RequestTalk(CurrentInteractNPC);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("npc is not find..."));
	}
}

// サーバー経由で会話イベントの起動
void AMain_Character::Server_RequestTalk_Implementation(ANPC_Character* npc)
{
	// ヌルポチェック
	if (!npc)
	{
		UE_LOG(LogTemp, Log, TEXT("npc not find... "));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("talk_event..."));

	// サーバーで最終的な距離判定
	float Dist = FVector::Dist(GetActorLocation(), npc->GetActorLocation());

	if (Dist < 300.0f)
	{
		UE_LOG(LogTemp, Log, TEXT("talk_event start!!"));
		npc->Talk_Event(this);
	}

	UE_LOG(LogTemp, Log, TEXT("talk_event end... "));
}

void AMain_Character::Set_NPC_Pointer(ANPC_Character* npc_charcter)
{
	TargetNPC = npc_charcter;
}

// 一番近いNPCを探す関数
ANPC_Character* AMain_Character::FindNearestNPC_FromList()
{
	float MinDist = FLT_MAX;				// デカい数値
	ANPC_Character* Result = nullptr;		// 見つかった一番近いNPCを格納

	// NPCListを一つずつ、すべてチェック
	for (ANPC_Character* NPC : NPCList)
	{
		// すでに消されていたり、nullptrのnpcを避ける
		if (!IsValid(NPC)) continue;

		// 買い物リストにあるNPCだけを検出するためにPlayerStateの取得
		AMyPlayerState* ps = GetPlayerState<AMyPlayerState>();

		// nullptr参照対策
		if (!IsValid(ps))
		{
			continue;
		}

		// 買い物リストにあるアイテムを販売してるNPCを探す
		for (int32 i = 0; i < ps->player_buy_list.Num(); i++)
		{
			// 買い物リストにあるアイテムを販売しているか
			if (ps->player_buy_list[i] == NPC->Get_ItemType() && ps->buylist_crear[i] != true)
			{
				// プレイヤーの座標とNPCの座標と
				float Dist = FVector::Dist(GetActorLocation(), NPC->GetActorLocation());

				// 今まで見た中で一番近いなら更新する
				if (Dist < MinDist)
				{
					MinDist = Dist;		// MinDistの更新
					Result = NPC;		// Resultの更新
				}
			}
		}
	}

	// 一番近いnpcのポインタを返す
	//if (Result)
	//{
	//	// デバッグするため一番近いNPCの取得とそのNPCの販売アイテムの表示
	//	switch (Result->Get_ItemType())
	//	{
	//	case E_ITEM_TYPE::E_NONE:
	//		UE_LOG(LogTemp, Error, TEXT("Nearby NPC not ItemType"));
	//		break;
	//	case E_ITEM_TYPE::E_JUICE:
	//		UE_LOG(LogTemp, Log, TEXT("Nearby NPC : E_JUICE"));
	//		break;
	//	case E_ITEM_TYPE::E_HAMBRGER:
	//		UE_LOG(LogTemp, Log, TEXT("Nearby NPC : E_HAMBRGER"));
	//		break;
	//	case E_ITEM_TYPE::E_DONUT:
	//		UE_LOG(LogTemp, Log, TEXT("Nearby NPC : E_DONUT"));
	//		break;
	//	case E_ITEM_TYPE::E_POPCORN:
	//		UE_LOG(LogTemp, Log, TEXT("Nearby NPC : E_POPCORN"));
	//		break;
	//	default:
	//		break;
	//	}
	//}

	return Result;
}

// ゴール地点登録処理
void AMain_Character::FindMyGoalPoint()
{
	// 自分のゴール地点の探索
	TArray<AActor*> goal;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGoalActor::StaticClass(), goal);

	// GoalActorの数ループ
	for (int32 j = 0; j < goal.Num(); j++)
	{
		// GoalActorの取得
		AGoalActor* goal_point = Cast<AGoalActor>(goal[j]);

		// nullptrチェック
		if (!goal_point)
		{
			continue;
		}

		UE_LOG(LogTemp, Warning, TEXT("GetAllActorsOfClass success!"));

		// MyPlayerStateの取得
		AMyPlayerState* ps = GetPlayerState<AMyPlayerState>();

		// nullptrチェック
		if (!ps)
		{
			continue;
		}

		UE_LOG(LogTemp, Warning, TEXT("GetPlayerState success!"));

		// 探したいGoalPointのタグ作成	例）player_numberが0ならGoalPoint_0を作成、1ならGoalPoint_1になる
		const FName TargetTag = FName(*FString::Printf(TEXT("GoalPoint_%d"), ps->player_number));

		// AGoslActorについているタグが探しているタグ（TargetTag）が同じなら
		if (goal_point->ActorHasTag(TargetTag))
		{
			// MyGoalPointに格納
			MyGoalPoint = goal_point;

			// ログの出力
			UE_LOG(LogTemp, Warning, TEXT("MyGoalPoint = TargetName: %s"), *goal_point->GetName());
		}
	}
}