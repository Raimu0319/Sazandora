// Fill out your copyright notice in the Description page of Project Settings.


#include "Main_Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/InputComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

// Sets default values
AMain_Character::AMain_Character()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//変数の初期化
	b_IsJump_ButtonHold = false;	//ジャンプボタンを押しているかどうか
	Jump_HoldTime = 0.0f;			//押し続けた時間
	Max_Jump_HoldTime = 1.0f;		//最大押し続け時間(秒)
	Min_Jump_Strength = 300.0f;		//最低ジャンプ力
	Max_Jump_Strength = 800.0f;		//最大ジャンプ力

	// カメラ用のSpringArm（カメラアーム）を作成
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 300.0f;	//カメラ距離
	SpringArmComp->bUsePawnControlRotation = true;

	// カメラを作成
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp);

	// 🔹 Skeletal Mesh（キャラの見た目）を読み込む
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshObj(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny"));
	if (MeshObj.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshObj.Object);
	}

	// 🔹 アニメーションBPを設定（任意）
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBP(TEXT("/Game/Characters/Mannequins/Animations/ABP_Manny_C"));
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

}

// Called to bind functionality to input
void AMain_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMain_Character::OnJumpPressed);

}

//ジャンプボタンが押されたら
void AMain_Character::OnJumpPressed()
{
	b_IsJump_ButtonHold = true;
	Jump_HoldTime = 0.0f;
}

//ジャンプボタンが離された時
void  AMain_Character::OnJumpReleased()
{
	b_IsJump_ButtonHold = false;		//ボタンが離されたためfalseにする

	// Fmath::Clamp(変数, 最小値, 最大値)で使える関数
	// 変数の値が最小値以下なら最小値を返し、最大値以上なら最大値を返す。範囲内ならそのままの値を返す
	float ClampedHoldTime = FMath::Clamp(Jump_HoldTime, 0.0f, Max_Jump_HoldTime);

	//線形補間につかう値を算出する。Alphaの値は 0 ～ 1　になる。
	float Alpha = ClampedHoldTime / Max_Jump_HoldTime;

	// Fmath::Lerpは２つの値の間の線形補間を行う
	// Alphaが0.5ならちょうどMin_Jump_StrengthとMax_Jump_Strengthの中間の数値になる
	float JumpPower = FMath::Lerp(Min_Jump_Strength, Max_Jump_Strength, Alpha);

	// LaunchCharacterは特定の地点に瞬時に移動させる関数（今回は慣性（初速度）を適用する形でジャンプの動きを再現）
	//（LaunchCharacter( FVector LaunchVelocity, bool bXYOverride, bool bZOverride )）
	// LaunchVelocity = 与える速度ベクトル　
	// bXYOverride = XY成分（水平）をfalseで既存の速度に加算し、trueで既存の値を書き換える
	// bZOverride = Z成分（垂直）を既存のZ速度で上書きする(trueの場合現在のZが消えて新しいZに書き変わる）
	LaunchCharacter(FVector(0, 0, JumpPower), false, true);
}
