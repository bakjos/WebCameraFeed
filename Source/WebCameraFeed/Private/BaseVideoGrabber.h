// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WebCameraDevice.h"

#include "Runnable.h"
#include "RunnableThread.h"
#include <Engine/Texture2D.h>
#include <Engine/TextureRenderTarget2D.h>
#include "PixelShaderDeclaration.h"
#include <Misc/CoreDelegates.h>


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

	virtual bool setup(int w, int h, bool mirrored = false) = 0;

	virtual void update()=0;

	virtual bool isFrameNew() const = 0;

	virtual void close() = 0;
	

	virtual int getHeight() const = 0;

	virtual int getWidth() const = 0;

	UTexture* getTexture();
    
    bool isVideoMirrored();
    
    void setVideoMirrored( bool mirrored );

    bool saveTextureAsFile (const FString& fileName );
    
    virtual bool switchBackAndFront() = 0;

    
protected:
    virtual void pause() = 0;
    virtual void resume() = 0;
    
    void registerDelegates();
    void unRegisterDelegates();
	void allocateData(int w, int h, EPixelFormat InFormat = PF_R8G8B8A8);
    void resizeData(int w, int h, EPixelFormat InFormat = PF_R8G8B8A8);
	void copyDataToTexture(unsigned char * pData, int TextureWidth, int TextureHeight, int numColors);
	void startThread();
	void stopThread();
	static void mirrorTexture_RenderThread(FRHICommandList& RHICmdList, FTexture2DRHIRef TextureRHIRef, FTextureRenderTargetResource* MirrorTextureRef, FDepthStencilStateRHIParamRef DepthStencilState);

	FRunnableThread* runnableThread;
	bool _running;
	int deviceID;

	TArray<uint8>	pixels;
	TWeakObjectPtr<UTexture2D>				cameraTexture;
    TWeakObjectPtr<UTextureRenderTarget2D>  mirroredTexture;
    
    void ApplicationWillDeactivateDelegate_Handler();
    void ApplicationHasReactivatedDelegate_Handler();	
    
    bool  mirrored;
    FRWLock  frwLock;
	
};
