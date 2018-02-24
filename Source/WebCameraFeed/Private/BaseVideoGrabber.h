// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WebCameraDevice.h"

#include "Runnable.h"
#include "RunnableThread.h"
#include <Engine/Texture2D.h>

DECLARE_LOG_CATEGORY_EXTERN(LogVideoGrabber,Log, All)

/**
 * 
 */
class BaseVideoGrabber: public FRunnable
{
public:
	BaseVideoGrabber();
	virtual ~BaseVideoGrabber();

	uint32 Run();

	virtual void setDeviceID(int deviceID);

	virtual int getDeviceID();

	virtual TArray<FVideoDevice>	listDevices() const = 0;

	virtual bool setup(int w, int h) = 0;

	virtual void update()=0;

	virtual bool isFrameNew() const = 0;

	virtual void close() {};
	

	virtual int getHeight() const = 0;

	virtual int getWidth() const = 0;

	UTexture2D* getTexture();


protected:
	void allocateData(int w, int h);
	void copyDataToTexture(unsigned char * pData, int TextureWidth, int TextureHeight, int numColors);
	void startThread();
	void stopThread();

	FRunnableThread* runnableThread;
	bool _running;
	int deviceID;

	TArray<uint8>	pixels;
	TWeakObjectPtr<UTexture2D> 				cameraTexture;
	


};
