// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseVideoGrabber.h"
#if PLATFORM_ANDROID

class AndroidVideoGrabber: public BaseVideoGrabber
{
public:
	AndroidVideoGrabber();
	virtual ~AndroidVideoGrabber();
    
    void setDeviceID(int deviceID) override;

	TArray<FVideoDevice>	listDevices() const override;

	bool isFrameNew() const override;

	void close() override;

	void update() override;

	bool setup(int w, int h, bool mirrored) override;

	int getHeight() const override;

	int getWidth() const override;
    
    bool switchBackAndFront() override;

    int getBackCamera() const override;

    int getFrontCamera() const override;

	void updatePixelsCB(unsigned char *isrc, int w, int h );
    
protected:

	void loadTexture();
	void deleteTexture();
    
    void pause() override;
    void resume() override;
    
    bool newFrame = false;
    bool bHavePixelsChanged = false;
    int width, height;
    int device = 0;
    bool bIsInit = false;
    int fps  = -1;

	int textureID;
    

private:

	int getFacingOfCamera(int device=-1)const;

	int getCameraFacing(int facing)const;

	/// Get number of cameras available
	int getNumCameras()const;

};
#endif
