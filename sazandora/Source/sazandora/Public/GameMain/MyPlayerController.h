// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h" 
#include "../Public/GameMain/EndWidget.h"
#include "HUDWidget.h"
#include "StartWaitWidget.h"
#include "MyPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class SAZANDORA_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AMyPlayerController();

	UPROPERTY()
	bool bAlreadyNotified = false;

	// プレイヤーのロード完了をサーバーへ通知する関数
	UFUNCTION(BlueprintCallable)
	void NotifyLoaded();

	UFUNCTION()
	void Create_HUDWidget();

	UFUNCTION()
	void Create_WaitStartWidget();

	UFUNCTION()
	void Create_EndWidget();

	UFUNCTION(Client, Reliable)
	void Client_StartGame();

	// ゲーム開始関数
	UFUNCTION(Server, Reliable)
	void Server_RequestStartGame();

	UFUNCTION()
	void Set_EndWidget_Text(bool flg);

	// ゲーム終了関数
	UFUNCTION(Client, Reliable)
	void Client_EndGame(bool is_clear);
	
	// UIの更新
	void Refresh_UI();

	// HUDWidgetの画像表示、非表示処理
	UFUNCTION(Client, Reliable)
	void A_Button_SetVisibility(bool flg);

	// AControllerクラスまたはAPawnクラスでPlayerStateのポインタが
	// クライアントにレプリケートされた時に呼び出されるコールバック関数
	virtual void OnRep_PlayerState() override;

protected:
	virtual void BeginPlay() override;

	// HUDWidget_classのクラスポインタ
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class  UHUDWidget> HUDWidget_class;

	// UStartWaitWidgetのクラスポインタ
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class  UStartWaitWidget> StartWaitWidget_class;

	// UEndWidgetのクラスポインタ
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class  UEndWidget> EndWidget_class;
	
	// 実際に生成されたウィジットのポインタ
	UHUDWidget* HUDWidget_pointer = nullptr;
	UStartWaitWidget* wait_widget = nullptr;
	UEndWidget* end_widget = nullptr;
	
	virtual void OnPossess(APawn* InPawn) override;

	// Aボタンの表示変更フラグ
	UPROPERTY()
	bool is_button_visible = false;
};
