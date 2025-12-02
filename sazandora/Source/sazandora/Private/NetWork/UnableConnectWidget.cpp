// Fill out your copyright notice in the Description page of Project Settings.


#include "NetWork/UnableConnectWidget.h"

void UUnableConnectWidget::NativeConstruct()
{
	if (OKButton)
	{
		OKButton->OnClicked.AddDynamic(this, &UUnableConnectWidget::OnOKButtonClicked);
	}
}

void UUnableConnectWidget::OnOKButtonClicked()
{
	this->SetVisibility(ESlateVisibility::Hidden);
}