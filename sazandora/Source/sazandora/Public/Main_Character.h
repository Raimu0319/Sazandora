// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Main_Character.generated.h"

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

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// ジャンプ処理
	void OnJumpPressed();
	void OnJumpReleased();

	void MoveForward(float value);	//前後移動処理
	void MoveRight(float value);	//左右移動処理

private:

	UPROPERTY(EditAnywhere, Category = "Jump Settings");
	bool  b_IsJump_ButtonHold;		//ジャンプボタンを押しているかどうか
	
	UPROPERTY(EditAnywhere, Category = "Jump Settings");
	float Jump_HoldTime;			//押し続けた時間

	UPROPERTY(EditAnywhere, Category = "Jump Settings");
	float Max_Jump_HoldTime;		//最大押し続け時間(秒)

	UPROPERTY(EditAnywhere, Category = "Jump Settings");
	float Min_Jump_Strength;		//最低ジャンプ力

	UPROPERTY(EditAnywhere, Category = "Jump Settings");
	float Max_Jump_Strength;		//最大ジャンプ力

protected:
	//	スプリングアーム（カメラの追従位置を制御）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera");
	class USpringArmComponent* SpringArmComp;

	// カメラ
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera");
	class UCameraComponent* CameraComp;
};
