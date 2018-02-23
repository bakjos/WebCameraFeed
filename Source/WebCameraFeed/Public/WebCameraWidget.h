// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "WebCameraDevice.h"
#include "WebCameraWidget.generated.h"


class SImage;

/**
 * 
 */
UCLASS()
class WEBCAMERAFEED_API UWebCameraWidget : public UWidget
{
	GENERATED_UCLASS_BODY()

public:

	

	UPROPERTY(EditAnywhere, Category = WebCamera)
	FWebCameraDeviceId  DeviceId;

	UPROPERTY(EditAnywhere, Category = WebCamera)
		int requestedWidth;

	UPROPERTY(EditAnywhere, Category = WebCamera)
		int requestedHeight;

protected:
	//~ Begin UWidget Interface
	virtual TSharedRef<SWidget> RebuildWidget() override;

	

protected:
	FSlateBrush			CameraBrush;
	TSharedPtr<SImage>	MyImage;
	
};
