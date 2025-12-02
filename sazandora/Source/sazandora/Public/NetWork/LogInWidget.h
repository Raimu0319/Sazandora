// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Blueprint/UserWidget.h"
#include "ServerListWidget.h"
#include "UnableConnectWidget.h"
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

	//Hostボタン
	UPROPERTY(meta = (BindWidget))
	UButton* HostButton;

	//Joinボタン
	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;


	//Joinを押した際にサーバー選択用画面にて使用するUI
	// ScrollBoxにサーバー行を追加する用の参照（UMGでバインド）
	UPROPERTY(meta = (BindWidget))
	UScrollBox* ServerListScrollBox;

	//サーバーリスト更新ボタン
	UPROPERTY(meta = (BindWidget))
	UButton* RefreshButton;

	//Backボタン
	UPROPERTY(meta = (BindWidget))
	UButton* BackButton;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* IPAddressTextBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Server List")
	TSubclassOf<class UServerListWidget> ServerListWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnableConnectWidget")
	TSubclassOf<class UUnableConnectWidget> UnableConnectWidget;

private:
	
	FString SelectedServerAddress;

	TArray<FSocket*> OccupiedPortSockets;
	TArray<int32> OccupiedPorts;

	bool HostServerStart = false;

	FString APIServerIP;

	void UnableConnectView(bool flag);
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
