// Fill out your copyright notice in the Description page of Project Settings.

#include "WebCameraWidget.h"
#include "Widgets/Images/SImage.h"

UWebCameraWidget::UWebCameraWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	requestedWidth = 640;
	requestedHeight = 480;
	DeviceId.selectedDevice = 0;
}

TSharedRef<SWidget> UWebCameraWidget::RebuildWidget()
{
	MyImage = SNew(SImage);
	return MyImage.ToSharedRef();
}




