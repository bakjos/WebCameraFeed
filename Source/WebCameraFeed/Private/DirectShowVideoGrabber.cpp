// Fill out your copyright notice in the Description page of Project Settings.

#include "DirectShowVideoGrabber.h"
#include <Async/Async.h>

#if PLATFORM_WINDOWS
DirectShowVideoGrabber::DirectShowVideoGrabber()
{
	device = 0;
	width = 640;
	height = 480;
	bChooseDevice = false;
	bDoWeNeedToResize = false;
	bIsFrameNew = false;
}

DirectShowVideoGrabber::~DirectShowVideoGrabber()
{
	stopThread();
	close();
}

void DirectShowVideoGrabber::setDeviceID(int _deviceID) {
	deviceID = _deviceID;
	bChooseDevice = true;
}

TArray<FVideoDevice> DirectShowVideoGrabber::listDevices() const{
	TArray<FVideoDevice> devices;
	std::vector <std::string> devList = VI.getDeviceList();

	for (size_t i = 0; i < devList.size(); i++) {
		FVideoDevice vd;
		vd.deviceName = UTF8_TO_TCHAR(devList[i].c_str());
		vd.id = i;
		vd.bAvailable = true;
		devices.Add(vd);
	}

	return devices;
}

bool DirectShowVideoGrabber::setup(int w, int h, bool _mirrored) {
	if (bChooseDevice) {
		device = deviceID;
		std::vector <std::string> devList = VI.getDeviceList();
		if (device >= devList.size()) {
			device = devList.size() - 1;
		}
		if (device < 0) {
			device = 0;
		}

		UE_LOG(LogVideoGrabber, Verbose,  TEXT("initGrabber(): choosing %d"), deviceID);
	}
	else {
		device = 0;
	}

	width = w;
	height = h;
	bGrabberInited = false;

	bool bOk = VI.setupDevice(device, width, height);

	int ourRequestedWidth = width;
	int ourRequestedHeight = height;

	if (bOk == true) {
		bGrabberInited = true;
		width = VI.getWidth(device);
		height = VI.getHeight(device);

		if (width == ourRequestedWidth && height == ourRequestedHeight) {
			bDoWeNeedToResize = false;
		}
		else {
			bDoWeNeedToResize = true;
			width = ourRequestedWidth;
			height = ourRequestedHeight;
		}
		registerDelegates();
        setVideoMirrored(_mirrored);
		allocateData(width, height);
		startThread();
		return true;
	} else {
		UE_LOG(LogVideoGrabber, Error,  TEXT("initGrabber(): error allocating a video device"));
		UE_LOG(LogVideoGrabber, Error,  TEXT("initGrabber(): please check your camera with AMCAP or other software"));

		bGrabberInited = false;
		return false;
	}
}

void DirectShowVideoGrabber::update() {
	if (bGrabberInited == true){
		bIsFrameNew = false;
		if (VI.isFrameNew(device)){

			if (!cameraTexture.IsValid()) {
				UE_LOG(LogVideoGrabber, Warning, TEXT("The texture is invalid, reallocating"));
				FEvent* fSemaphore = FGenericPlatformProcess::GetSynchEventFromPool(false);
				int w = width;
				int h = height;
				AsyncTask(ENamedThreads::GameThread, [this, w, h, fSemaphore]() {
					this->resizeData(w, h, PF_B8G8R8A8);
					fSemaphore->Trigger();
				});
				fSemaphore->Wait();
				FGenericPlatformProcess::ReturnSynchEventToPool(fSemaphore);
			}

			bIsFrameNew = true;

			unsigned char * viPixels = VI.getPixels(device, true, true);

			if (bDoWeNeedToResize == true){

				int inputW = VI.getWidth(device);
				int inputH = VI.getHeight(device);

				float scaleW =	(float)inputW / (float)width;
				float scaleH =	(float)inputH / (float)height;

				for(int i=0;i<width;i++){
					for(int j=0;j<height;j++){

						float posx = i * scaleW;
						float posy = j * scaleH;
						int posPix = (((int)posy * inputW * 3) + ((int)posx * 3));

						pixels.GetData()[(j*width * 4) + i * 4] = viPixels[posPix];
						pixels.GetData()[(j*width * 4) + i * 4 + 1] = viPixels[posPix + 1];
						pixels.GetData()[(j*width * 4) + i * 4 + 2] = viPixels[posPix + 2];
						pixels.GetData()[(j*width * 4) + i * 4 + 3] = 255;

					}
				}
				copyDataToTexture(pixels.GetData(), width, height, 4);
			} else {
				for (int i = 0; i < width; i++) {
					for (int j = 0; j < height; j++) {
						int posPix = (j*width * 3) + i * 3;
						int posDst = (j*width * 4) + i * 4;
						pixels.GetData()[posDst] = viPixels[posPix];
						pixels.GetData()[posDst + 1] = viPixels[posPix + 1];
						pixels.GetData()[posDst + 2] = viPixels[posPix + 2];
						pixels.GetData()[posDst + 3] = 255;
					}
				}
				uint32 MemorySize = width*height * 3;
				copyDataToTexture(pixels.GetData(), width, height, 4);
				
			}
		}
	}
	
}

bool DirectShowVideoGrabber::isFrameNew() const {
	return bIsFrameNew;
}



void DirectShowVideoGrabber::close() {
	unRegisterDelegates();

	stopThread();


	if (bGrabberInited == true) {
		VI.stopDevice(device);
		bGrabberInited = false;
	}

	pixels.Empty();
}

int DirectShowVideoGrabber::getHeight() const {
	return height;
}

int DirectShowVideoGrabber::getWidth() const {
	return width;
}

void DirectShowVideoGrabber::pause() {
	UE_LOG(LogVideoGrabber, Display,  TEXT("paused"));
}
void DirectShowVideoGrabber::resume() {
	UE_LOG(LogVideoGrabber, Display,  TEXT("resume"));
}

#endif
