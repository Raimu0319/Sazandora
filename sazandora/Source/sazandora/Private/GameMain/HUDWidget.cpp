// Fill out your copyright notice in the Description page of Project Settings.


#include "../Public/GameMain/HUDWidget.h"

void UHUDWidget::InitializeWidget(AMyPlayerState* PlayerState)
{
	player_state = PlayerState;

	if (player_state)
	{
		player_state->OnItemUpdated();
	}
}

// 表示変更関数
void UHUDWidget::SetVisible(bool is_visible)
{
	// ポインタの中身があり、is_visibleがtrueかどうか
	if (ItemBuy_Text && A_Button && is_visible)
	{
		// 表示する
		A_Button->SetVisibility(ESlateVisibility::Visible);
		ItemBuy_Text->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		// 表示しない
		A_Button->SetVisibility(ESlateVisibility::Hidden);
		ItemBuy_Text->SetVisibility(ESlateVisibility::Hidden);
	}
}
