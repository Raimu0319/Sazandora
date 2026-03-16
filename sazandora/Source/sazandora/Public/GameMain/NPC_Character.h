// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Item_Type.h"
#include "NPC_Character.generated.h"

UCLASS()
class SAZANDORA_API ANPC_Character : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ANPC_Character();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// プレイヤーが会話範囲にいるかどうか
	UPROPERTY(VisibleAnywhere, Category = "NPC",Replicated)
	bool Is_Talk_Flg;

	// オーバーレイマテリアルの保存
	UPROPERTY(EditAnywhere, Category = "Effects")
	UMaterialInterface* OverlayMaterial;

	// オーバーレイマテリアルの保存
	UPROPERTY(EditAnywhere, Category = "Effects")
	UMaterialInterface* OverlayMaterial_Is_Talk;

	// アウトラインを表示変更フラグ
	UPROPERTY()
	bool is_outline = false;

	// 販売するアイテム
	E_ITEM_TYPE e_mytype;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// プレイヤーと会話できる範囲を検出するコリジョン
	UPROPERTY(VisibleAnywhere, Category="NPC")
	class USphereComponent* TalkRange;

	UPROPERTY(VisibleAnywhere, Category="NPC")
	class UCapsuleComponent* Capsule;

	// 会話開始関数（プレイヤーが近づいたときなどに呼ぶ）
	UFUNCTION()
	virtual void OnPlayerEnterRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComo, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// プレイヤーが離れたとき
	UFUNCTION()
	virtual void OnPlayerLeaveRange(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// アウトライン表示変更処理
	UFUNCTION()
	void ChangeOutlineVisibility(bool flg);

	UFUNCTION()
	// アウトラインの色変更処理		flg : false = red, true = green
	void Is_TalkCheck(bool flg);

	// 会話イベント
	UFUNCTION()
	virtual void Talk_Event(AMain_Character* player);

	UFUNCTION()
	E_ITEM_TYPE Get_ItemType();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

};
