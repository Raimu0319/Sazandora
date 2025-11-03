// Fill out your copyright notice in the Description page of Project Settings.


#include "HUDWidget.h"

void UHUDWidget::InitializeWidget(AMyPlayerState* PlayerState)
{
	player_state = PlayerState;

	if (player_state)
	{
		// playerstateにポインタを渡す
		player_state->wiget_p = this;

		player_state->OnItemUpdated();
	}
}
