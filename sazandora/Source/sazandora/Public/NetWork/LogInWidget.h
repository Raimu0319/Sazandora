// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Blueprint/UserWidget.h"
#include "ServerListWidget.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"
#include "IPAddress.h"
#include "Common/TcpSocketBuilder.h"
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
	UButton* HostButton;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;

	// ScrollBoxにサーバー行を追加する用の参照（UMGでバインド）
	UPROPERTY(meta = (BindWidget))
	UScrollBox* ServerListScrollBox;

	// 「更新」ボタン
	UPROPERTY(meta = (BindWidget))
	UButton* RefreshButton;

	UPROPERTY(meta = (BindWidget))
	UButton* BackButton;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server List")
	TSubclassOf<class UServerListWidget> ServerListWidget;


private:
	
	FString SelectedServerAddress;

	TArray<FSocket*> OccupiedPortSockets;
	TArray<int32> OccupiedPorts;

	bool HostServerStart = false;
public:

	//コンストラクタ
	ULogInWidget(const FObjectInitializer& ObjectInitializer);

	//Hostボタンが押された時
	UFUNCTION()
	void OnHostButtonClicked();

	//Joinボタンが押された時
	UFUNCTION()
	void OnJoinButtonClicked();

	// サーバーリスト取得ボタンが押された時
	UFUNCTION(BlueprintCallable)
	void OnRefreshServerListClicked();

	//Backボタンが押された時
	UFUNCTION(BlueprintCallable)
	void OnBackButtonClicked();

	//サーバー起動用関数
	void Server_Start();

	// HTTPレスポンスを受け取ったときに呼ばれる
	void OnServerListReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	//サーバー起動の際に空いているPort番号を調べる
	int32 CheckforfreePorts();
	void ReleaseReservedPorts();
};
