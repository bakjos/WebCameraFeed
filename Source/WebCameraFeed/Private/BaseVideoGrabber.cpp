// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseVideoGrabber.h"

DEFINE_LOG_CATEGORY(LogVideoGrabber)

BaseVideoGrabber::BaseVideoGrabber()
{
	_running = false;
	runnableThread = nullptr;
	deviceID  = 0;
}

void BaseVideoGrabber::setDeviceID(int _deviceID) {
	deviceID = _deviceID;
}

int BaseVideoGrabber::getDeviceID() {
	return deviceID;
}

BaseVideoGrabber::~BaseVideoGrabber()
{
	stopThread();

	close();
}

void BaseVideoGrabber::startThread() {
	stopThread();
	_running = true;
	runnableThread = FRunnableThread::Create(this, TEXT("VideoGrabberThread"));
}

void BaseVideoGrabber::stopThread() {
	if (runnableThread != nullptr) {
		_running = false;
		runnableThread->Kill();
		runnableThread= nullptr;
	}
}

uint32 BaseVideoGrabber::Run() {
	while (_running)
	{
		update();
		FPlatformProcess::Sleep(0.016);
	}
	
	return 0;
}

void BaseVideoGrabber:: allocateData(int w, int h) {
	
	uint32 MemorySize = w*h * 4;
	pixels.Reset();
	pixels.AddUninitialized(MemorySize);
	FMemory::Memzero(pixels.GetData(), MemorySize);


	
	cameraTexture = UTexture2D::CreateTransient(w, h, PF_R8G8B8A8);
	ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
	UpdateTextureResource,
	UTexture2D*, texture, cameraTexture.Get(),
	{
		texture->UpdateResource();

	});

}

void BaseVideoGrabber::copyDataToTexture(unsigned char * pData, int TextureWidth, int TextureHeight, int numColors) {
	struct FUpdateTextureRegionsData
	{
		FTexture2DResource* Texture2DResource;
		int32 MipIndex;
		FUpdateTextureRegion2D Region;
		uint32 SrcPitch;
		uint32 SrcBpp;
		uint8* SrcData;
	};

	FUpdateTextureRegionsData* RegionData = new FUpdateTextureRegionsData;
	if ( cameraTexture.IsValid() ) {
		RegionData->Texture2DResource = (FTexture2DResource*)cameraTexture->Resource;
		RegionData->MipIndex = 0;
		RegionData->Region = FUpdateTextureRegion2D(0, 0, 0, 0, TextureWidth, TextureHeight);;
		RegionData->SrcPitch = (uint32)(numColors * TextureWidth); //TODO: Clap to 32bits
		RegionData->SrcBpp = (uint32)numColors;
		RegionData->SrcData = pData;



		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
			UpdateTextureRegionsData,
			FUpdateTextureRegionsData*, RegionData, RegionData,
			{
				int32 CurrentFirstMip = RegionData->Texture2DResource->GetCurrentFirstMip();
				if (RegionData->MipIndex >= CurrentFirstMip)
				{
					RHIUpdateTexture2D(RegionData->Texture2DResource->GetTexture2DRHI(),
						RegionData->MipIndex - CurrentFirstMip,
						RegionData->Region,
						RegionData->SrcPitch,
						RegionData->SrcData + RegionData->Region.SrcY * RegionData->SrcPitch + RegionData->Region.SrcX * RegionData->SrcBpp);
				}

				delete RegionData;
			});
	}
}

UTexture2D* BaseVideoGrabber::getTexture() {
	if (cameraTexture.IsValid()) {
		return cameraTexture.Get();
	}
	return nullptr;
}
