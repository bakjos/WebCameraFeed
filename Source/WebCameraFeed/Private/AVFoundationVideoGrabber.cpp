// Fill out your copyright notice in the Description page of Project Settings.

#include "AVFoundationVideoGrabber.h"

#if PLATFORM_MAC
AVFoundationVideoGrabber::AVFoundationVideoGrabber()
{
}

AVFoundationVideoGrabber::~AVFoundationVideoGrabber()
{
}

TArray<FVideoDevice> AVFoundationVideoGrabber::listDevices() const {
	//TODO:
	TArray<FVideoDevice> devices;
	return devices;
	
}
#endif
