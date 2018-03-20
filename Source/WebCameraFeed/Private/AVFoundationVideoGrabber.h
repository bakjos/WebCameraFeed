// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseVideoGrabber.h"
#if PLATFORM_MAC || PLATFORM_IOS

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import <Accelerate/Accelerate.h>
#import <CoreMedia/CoreMedia.h>
#import <CoreVideo/CoreVideo.h>

class AVFoundationVideoGrabber;

@interface OSXVideoGrabber : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate> {
    
@public
    CGImageRef currentFrame;
    
    int width;
    int height;
    
    BOOL bInitCalled;
    int deviceID;
    bool paused;
    
    AVCaptureDeviceInput        *captureInput;
    AVCaptureVideoDataOutput    *captureOutput;
    AVCaptureDevice                *device;
    AVCaptureSession            *captureSession;
    
    AVFoundationVideoGrabber * grabberPtr;
}

-(BOOL)initCapture:(int)framerate capWidth:(int)w capHeight:(int)h capMirror:(BOOL)mirror;
-(void)startCapture;
-(void)stopCapture;
-(void)lockExposureAndFocus;
-(TArray <FString>)listDevices;
-(void)setDevice:(int)_device;
-(void)eraseGrabberPtr;
-(int)switchBackAndFront;
-(void)pause;
-(void)resume;
-(CGImageRef)getCurrentFrame;

@end

/**
 * 
 */

class AVFoundationVideoGrabber: public BaseVideoGrabber
{
public:
	AVFoundationVideoGrabber();
	virtual ~AVFoundationVideoGrabber();
    
    void setDeviceID(int deviceID) override;

	TArray<FVideoDevice>	listDevices() const override;

	bool isFrameNew() const override;

	void close() override;

	void update() override;

	bool setup(int w, int h, bool mirrored) override;

	int getHeight() const override;

	int getWidth() const override;
    
    bool switchBackAndFront() override;
    
protected:
    
    void pause() override;
    void resume() override;
    
    bool newFrame = false;
    bool bHavePixelsChanged = false;
    void clear();
    int width, height;
    
    int device = 0;
    bool bIsInit = false;
    
    int fps  = -1;
    
    OSXVideoGrabber * grabber;

public:
    void updatePixelsCB(unsigned char *isrc, int w, int h );
    bool bLock ;
};
#endif
