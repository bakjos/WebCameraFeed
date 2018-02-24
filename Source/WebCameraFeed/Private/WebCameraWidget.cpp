// Fill out your copyright notice in the Description page of Project Settings.

#include "WebCameraWidget.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ShowFlags.h"
#include "VideoGrabberPool.h"

#define LOCTEXT_NAMESPACE "WebCamera"

UWebCameraWidget::UWebCameraWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), ColorAndOpacity(FLinearColor::White)
{
	requestedWidth = 640;
	requestedHeight = 480;
	DeviceId.selectedDevice = 0;
}

 UWebCameraWidget::~UWebCameraWidget() {
	if ( currentVideoGrabber.IsValid()) {
		 VideoGrabberPool::ReleaseVideoGrabber(currentVideoGrabber);
	}
}

TSharedRef<SWidget> UWebCameraWidget::RebuildWidget()
{
	if (IsDesignTime())
	{
		return SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Web Camera", "Web Camera"))
			];
	}
	else
	{
		MyImage = SNew(SWebCameraImage);


		return MyImage.ToSharedRef();
	}
}


void UWebCameraWidget::SynchronizeProperties() {
	Super::SynchronizeProperties();

	TAttribute<FSlateColor> ColorAndOpacityBinding = PROPERTY_BINDING(FSlateColor, ColorAndOpacity);
	if (MyImage.IsValid())
	{
		currentVideoGrabber = VideoGrabberPool::GetVideoGrabber(DeviceId.selectedDevice, requestedWidth, requestedHeight);
		MyImage->SetVideoGrabber(currentVideoGrabber);
		MyImage->SetColorAndOpacity(ColorAndOpacityBinding);
		//MyImage->SetOnMouseButtonDown(BIND_UOBJECT_DELEGATE(FPointerEventHandler, HandleMouseButtonDown));
	}
}

const FSlateBrush* UWebCameraWidget::ConvertImage(TAttribute<FSlateBrush> InImageAsset) const
{
	UWebCameraWidget* MutableThis = const_cast<UWebCameraWidget*>(this);
	MutableThis->CameraBrush = InImageAsset.Get();

	return &CameraBrush;
}


void UWebCameraWidget::SetColorAndOpacity(FLinearColor InColorAndOpacity)
{
	ColorAndOpacity = InColorAndOpacity;
	if (MyImage.IsValid())
	{
		MyImage->SetColorAndOpacity(ColorAndOpacity);
	}
}



