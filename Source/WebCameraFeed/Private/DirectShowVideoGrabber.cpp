// Fill out your copyright notice in the Description page of Project Settings.

#include "DirectShowVideoGrabber.h"

#if PLATFORM_WINDOWS
DirectShowVideoGrabber::DirectShowVideoGrabber()
{
	device = 0;
	width = 640;
	height = 480;
}

DirectShowVideoGrabber::~DirectShowVideoGrabber()
{
}

TArray<FVideoDevice> DirectShowVideoGrabber::listDevices() const{
	TArray<FVideoDevice> devices;
	std::vector <std::string> devList = VI.getDeviceList();

	for (int i = 0; i < devList.size(); i++) {
		FVideoDevice vd;
		vd.deviceName = UTF8_TO_TCHAR(devList[i].c_str());
		vd.id = i;
		vd.bAvailable = true;
		devices.Add(vd);
	}

	return devices;
}
#endif
