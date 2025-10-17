// Fill out your copyright notice in the Description page of Project Settings.


#include "Main_Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/InputComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"

// Sets default values
AMain_Character::AMain_Character()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//カプセルコリジョン
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	//変数の初期化
	b_IsJump_ButtonHold = false;	//ジャンプボタンを押しているかどうか
	b_IsJump = false;				//ジャンプをしているかどうか
	Jump_HoldTime = 0.0f;			//押し続けた時間
	Max_Jump_HoldTime = 0.3f;		//最大押し続け時間(秒)
	Min_Jump_Strength = 300.0f;		//最低ジャンプ力
	Add_Jump_Boost = 50.0f;			//追加ジャンプ力
	b_IsRun = false;				//ダッシュしているかどうか
	Run_Speed = 0.0f;				//ダッシュ速度
	Max_Run_Speed = 15.0f;			//最大ダッシュ速度

	// カメラ用のSpringArm（カメラアーム）を作成
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 300;	//カメラ距離
	SpringArmComp->bUsePawnControlRotation = false;
	SpringArmComp->SocketOffset = FVector(30.0f, 90.0f, 30.0f);		//カメラの上下左右の調整

	// カメラを作成
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp);

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

	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

// Called when the game starts or when spawned
void AMain_Character::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMain_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// ジャンプ処理
	AMain_Character::Jump(DeltaTime);

}

// Called to bind functionality to input
void AMain_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// ジャンプ入力
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMain_Character::OnJumpPressed);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AMain_Character::OnJumpReleased);

	// 移動入力のバインド
	PlayerInputComponent->BindAxis("MoveForward", this, &AMain_Character::MoveForward);
	PlayerInputComponent->BindAxis("MoveR/L", this, &AMain_Character::MoveRight);

	// カメラ操作のバインド
	PlayerInputComponent->BindAxis("MouseX", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("MouseY", this, &APawn::AddControllerPitchInput);
}

//ジャンプボタンが押されたら
void AMain_Character::OnJumpPressed()
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
}

//ジャンプボタンが離された時
void  AMain_Character::OnJumpReleased()
{
	b_IsJump_ButtonHold = false;		//ボタンが離されたためfalseにする
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

void AMain_Character::Jump(float DeltaTime)
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
			NewVelocity.Z += ExtraPower * DeltaTime * 60.0f;			//Z方向へ1フレーム分の加速
			GetCharacterMovement()->Velocity = NewVelocity;				//移動量の変更
		}
		else
		{
			// 最大時間を超えたらジャンプ終了
			b_IsJump_ButtonHold = false;
		}
	}
}

void AMain_Character::Move_Run(float DeltaTime)
{
	
}