// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Item_Type.h"
#include "NPC_Character.h"
#include "GoalActor.h"
#include "Components/ArrowComponent.h"
#include "Main_Character.generated.h"

class ANPC_Character;

UCLASS()
class SAZANDORA_API AMain_Character : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMain_Character();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	ANPC_Character* TargetNPC;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// ジャンプ処理
	void OnJumpPressed();
	void OnJumpReleased();
	void Custom_Jump(float DeltaTime);

	// ダッシュ処理
	void On_Dash_Pressed();

	void On_Dash_Released();

	UFUNCTION(Server, Reliable)
	void Server_StartDash();

	void Dash(float DeltaTime);		

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Talk_NPC();

	// 移動処理
	void MoveForward(float value);	//前後移動処理
	void MoveRight(float value);	//左右移動処理

	// NPC関連
	void Set_NPC_Pointer(ANPC_Character* npc_charcter);		// NPCのポインタをセットする関数
	void Set_Talk_Flg(bool talk_flg);						// Talkフラグのセット

	// ジャンプ処理をサーバー側で実行するようにする関数
	UFUNCTION(Server, Reliable)
	void Server_JumpPressed();

	// ジャンプ停止処理をサーバー側で実行するようにする関数
	UFUNCTION(Server, Reliable)
	void Server_JumpReleased();

	// ジャンプ開始関数
	UFUNCTION()
	void ExecJumpStart();

	// ジャンプ開始関数
	UFUNCTION()
	void ExecJumpStop();

	//int team_number = 0;

private:

	// 例）UPROPERTY(EditAnywhere(エディタ上で編集可能)、Category = 項目名)
	UPROPERTY(EditAnywhere, Category = "Jump Settings");
	bool  b_IsJump_ButtonHold;		//ジャンプボタンを押しているかどうか

	UPROPERTY(EditAnywhere, Category = "Jump Settings");
	bool  b_IsJump;		//ジャンプをしているかどうか
	
	UPROPERTY(EditAnywhere, Category = "Jump Settings");
	float Jump_HoldTime;			//押し続けた時間

	UPROPERTY(EditAnywhere, Category = "Jump Settings");
	float Max_Jump_HoldTime;		//最大押し続け時間(秒)

	UPROPERTY(EditAnywhere, Category = "Jump Settings");
	float Min_Jump_Strength;		//最低ジャンプ力

	UPROPERTY(EditAnywhere, Category = "Jump Settings");
	float Add_Jump_Boost;			//追加ジャンプ力

	UPROPERTY(EditAnywhere, Category = "Dash Settings")
	bool b_IsDash;					//ダッシュ状態かどうか

	UPROPERTY(EditAnywhere, Category = "Dash Settings")
	float Dash_HoldTime;				//ダッシュキーのホールド時間

	UPROPERTY(EditAnywhere, Category = "Dash Settings")
	float Dash_Speed;				//ダッシュした時の移動速度

	UPROPERTY(EditAnywhere, Category = "Dash Settings")
	float Max_Dash_Speed;			//最大ダッシュ速度

	UPROPERTY()
	bool Is_LocalCan_Innteract = false;		//NPCが近くにいるかどうか

	UPROPERTY()
	bool Is_Talk;			//最大ダッシュ速度

	ANPC_Character* CurrentInteractNPC = nullptr;

	// Server(サーバーで実行)、Reliable(確実に呼び出す)
	// サーバー側でNPCと近いかの判定
	UFUNCTION(Server, Reliable)
	void Server_SetInteractNPC(ANPC_Character* NPC, bool is_can_interact);

protected:
	//	スプリングアーム（カメラの追従位置を制御）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera");
	class USpringArmComponent* SpringArmComp;

	// カメラ
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera");
	class UCameraComponent* CameraComp;

	// オーバーレイマテリアルの保存
	UPROPERTY(EditAnywhere, Category = "Effects")
	UMaterialInterface* OverlayMaterial;


	UFUNCTION()
	void CheckInteract();

	UFUNCTION()
	void Try_Talk();

	UFUNCTION(Server, Reliable)
	void Server_RequestTalk(ANPC_Character* npc);

	UPROPERTY()
	TArray<ANPC_Character*> NPCList;

	UPROPERTY()
	ANPC_Character* NearestNPC;

	UPROPERTY()
	AGoalActor* MyGoalPoint;

	// 矢印（3Dで出す想定）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UArrowComponent* GuideArrow;

	ANPC_Character* FindNearestNPC_FromList();

	// ゴール地点登録処理
	UFUNCTION()
	void FindMyGoalPoint();

};
