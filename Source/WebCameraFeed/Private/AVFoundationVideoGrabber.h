// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseVideoGrabber.h"
/**
 * 
 */
#if PLATFORM_MAC
class AVFoundationVideoGrabber: public BaseVideoGrabber
{
public:
	AVFoundationVideoGrabber();
	virtual ~AVFoundationVideoGrabber();

	TArray<FVideoDevice>	listDevices() const override;
};
#endif
