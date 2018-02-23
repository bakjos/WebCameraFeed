// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WebCameraDevice.h"
/**
 * 
 */
class BaseVideoGrabber
{
public:
	BaseVideoGrabber();
	virtual ~BaseVideoGrabber();

	virtual TArray<FVideoDevice>	listDevices() const = 0;
};
