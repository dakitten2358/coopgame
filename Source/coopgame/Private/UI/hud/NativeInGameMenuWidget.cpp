// Fill out your copyright notice in the Description page of Project Settings.

#include "NativeInGameMenuWidget.h"

void UNativeInGameMenuWidget::BroadcastHideMenuRequested()
{
	OnHideMenuRequested.Broadcast();
}



