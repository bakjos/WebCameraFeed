// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseVideoGrabber.h"

#if PLATFORM_WINDOWS

#include "AllowWindowsPlatformTypes.h"
#include "videoInput.h"
#include "HideWindowsPlatformTypes.h"

class DirectShowVideoGrabber: public BaseVideoGrabber
{
public:
	DirectShowVideoGrabber();
	virtual ~DirectShowVideoGrabber();

	TArray<FVideoDevice>	listDevices() const override;

protected:
	int width;
	int height;
	int 					device;
	videoInput 				VI;
};
#endif
