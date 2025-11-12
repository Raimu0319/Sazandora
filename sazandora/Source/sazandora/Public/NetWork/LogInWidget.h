// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ServerListWidget.h"
#include "LogInWidget.generated.h"


/**
 * 
 */
UCLASS()
class SAZANDORA_API ULogInWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UButton* ServerButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ClientButton;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* TextBoxIPAddress;

	// ScrollBoxにサーバー行を追加する用の参照（UMGでバインド）
	UPROPERTY(meta = (BindWidget))
	UScrollBox* ServerListScrollBox;

	// 「更新」ボタン
	UPROPERTY(meta = (BindWidget))
	UButton* RefreshButton;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server List")
	TSubclassOf<class UServerListWidget> ServerListWidget;

	

	//void OnServerListReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

private:

	UFUNCTION()
	void OnServerButtonClicked();

	UFUNCTION()
	void OnClientButtonClicked();

	FString SelectedServerAddress;

public:

	ULogInWidget(const FObjectInitializer& ObjectInitializer);

	// サーバーリスト取得ボタンを押したとき
	UFUNCTION(BlueprintCallable)
	void OnRefreshServerListClicked();

	// HTTPレスポンスを受け取ったときに呼ばれる
	void OnServerListReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};
