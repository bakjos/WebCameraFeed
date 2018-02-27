// Fill out your copyright notice in the Description page of Project Settings.

#include "AVFoundationVideoGrabber.h"

#if PLATFORM_MAC

#import <Accelerate/Accelerate.h>

@interface OSXVideoGrabber ()
@property (nonatomic,retain) AVCaptureSession *captureSession;
@end

@implementation OSXVideoGrabber
@synthesize captureSession;

#pragma mark -
#pragma mark Initialization
- (id)init {
    self = [super init];
    if (self) {
        captureInput = nil;
        captureOutput = nil;
        device = nil;
        
        bInitCalled = NO;
        grabberPtr = NULL;
        deviceID = 0;
        width = 0;
        height = 0;
        currentFrame = 0;
    }
    return self;
}

- (BOOL)initCapture:(int)framerate capWidth:(int)w capHeight:(int)h  capMirror:(BOOL)mirror{
    NSArray * devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
    
    if([devices count] > 0) {
        if(deviceID>[devices count]-1)
            deviceID = [devices count]-1;
        
        
        // We set the device
        device = [devices objectAtIndex:deviceID];
        
        NSError *error = nil;
        [device lockForConfiguration:&error];
        
        if(!error) {
            
            float smallestDist = 99999999.0;
            int bestW, bestH = 0;
            
            // Set width and height to be passed in dimensions
            // We will then check to see if the dimensions are supported and if not find the closest matching size.
            width = w;
            height = h;
            
            FVector2D requestedDimension(width, height);
            
            AVCaptureDeviceFormat * bestFormat  = nullptr;
            for ( AVCaptureDeviceFormat * format in [device formats] ) {
                CMFormatDescriptionRef desc = format.formatDescription;
                CMVideoDimensions dimensions = CMVideoFormatDescriptionGetDimensions(desc);
                
                float tw = dimensions.width;
                float th = dimensions.height;
                FVector2D formatDimension(tw, th);
                
                if( tw == width && th == height ){
                    bestW = tw;
                    bestH = th;
                    bestFormat = format;
                    break;
                }
                
                float dist = (formatDimension-requestedDimension).Size();
                if( dist < smallestDist ){
                    smallestDist = dist;
                    bestW = tw;
                    bestH = th;
                    bestFormat = format;
                }
                UE_LOG(LogVideoGrabber, Error,  TEXT("supported dimensions are: %d %d"), dimensions.width, dimensions.height);
            }
            
            // Set the new dimensions and format
            if( bestFormat != nullptr && bestW != 0 && bestH != 0 ){
                if( bestW != width || bestH != height ){
                    UE_LOG(LogVideoGrabber, Warning,  TEXT(" requested width and height aren't supported. Setting capture size to closest match: %d by %d"), bestW , bestH);
                }
                
                [device setActiveFormat:bestFormat];
                width = bestW;
                height = bestH;
            }
            
            //only set the framerate if it has been set by the user
            if( framerate > 0 ){
                
              
                
                AVFrameRateRange * desiredRange = [AVFrameRateRange alloc];
                NSArray * supportedFrameRates = device.activeFormat.videoSupportedFrameRateRanges;
                
                int numMatch = 0;
                for(AVFrameRateRange * range in supportedFrameRates){
                    
                    if( (floor(range.minFrameRate) <= framerate && ceil(range.maxFrameRate) >= framerate) ){
                        UE_LOG(LogVideoGrabber, Verbose,  TEXT("found good framerate range, min: %d max %d, requested fps: %f"), framerate );
                        desiredRange = range;
                        numMatch++;
                    }
                }
                
                if( numMatch > 0 ){
                    //TODO: this crashes on some devices ( Orbecc Astra Pro )
                    device.activeVideoMinFrameDuration = desiredRange.minFrameDuration;
                    device.activeVideoMaxFrameDuration = desiredRange.maxFrameDuration;
                }else{
                   UE_LOG(LogVideoGrabber, Error,  TEXT( " could not set framerate to: %f. Device supports: "), framerate);
                    for(AVFrameRateRange * range in supportedFrameRates){
                         UE_LOG(LogVideoGrabber, Error,  TEXT(" framerate range of: %f to %f"), range.minFrameRate ,range.maxFrameRate);
                    }
                }
                
            }
            
            [device unlockForConfiguration];
        } else {
            NSLog(@"OSXVideoGrabber Init Error: %@", error);
        }
        
        // We setup the input
        captureInput                        = [AVCaptureDeviceInput
                                               deviceInputWithDevice:device
                                               error:nil];
        
        // We setup the output
        captureOutput = [[AVCaptureVideoDataOutput alloc] init];
        // While a frame is processes in -captureOutput:didOutputSampleBuffer:fromConnection: delegate methods no other frames are added in the queue.
        // If you don't want this behaviour set the property to NO
        captureOutput.alwaysDiscardsLateVideoFrames = YES;
       
        
        
        
        // We create a serial queue to handle the processing of our frames
        dispatch_queue_t queue;
        queue = dispatch_queue_create("cameraQueue", NULL);
        [captureOutput setSampleBufferDelegate:self queue:queue];
        dispatch_release(queue);
        
        NSDictionary* videoSettings =[NSDictionary dictionaryWithObjectsAndKeys:
                                      [NSNumber numberWithDouble:width], (id)kCVPixelBufferWidthKey,
                                      [NSNumber numberWithDouble:height], (id)kCVPixelBufferHeightKey,
                                      [NSNumber numberWithUnsignedInt:kCVPixelFormatType_32BGRA], (id)kCVPixelBufferPixelFormatTypeKey,
                                      nil];
        [captureOutput setVideoSettings:videoSettings];
        
        // And we create a capture session
        if(self.captureSession) {
            self.captureSession = nil;
        }
        self.captureSession = [[[AVCaptureSession alloc] init] autorelease];
        
        [self.captureSession beginConfiguration];
        
        // We add input and output
        [self.captureSession addInput:captureInput];
        [self.captureSession addOutput:captureOutput];
        
        // We specify a minimum duration for each frame (play with this settings to avoid having too many frames waiting
        // in the queue because it can cause memory issues). It is similar to the inverse of the maximum framerate.
        // In this example we set a min frame duration of 1/10 seconds so a maximum framerate of 10fps. We say that
        // we are not able to process more than 10 frames per second.
        // Called after added to captureSession
        
        AVCaptureConnection *conn = [captureOutput connectionWithMediaType:AVMediaTypeVideo];
       
        if ([conn isVideoMinFrameDurationSupported] == YES &&
            [conn isVideoMaxFrameDurationSupported] == YES) {
            [conn setVideoMinFrameDuration:CMTimeMake(1, framerate)];
            [conn setVideoMaxFrameDuration:CMTimeMake(1, framerate)];
        }
        
        // We start the capture Session
        [self.captureSession commitConfiguration];
        [self.captureSession startRunning];
        
        bInitCalled = YES;
        return YES;
    }
    return NO;
}

- (void)startCapture{
    
    [self.captureSession startRunning];
    
    [captureInput.device lockForConfiguration:nil];
    
    //if( [captureInput.device isExposureModeSupported:AVCaptureExposureModeAutoExpose] ) [captureInput.device setExposureMode:AVCaptureExposureModeAutoExpose ];
    if( [captureInput.device isFocusModeSupported:AVCaptureFocusModeAutoFocus] )    [captureInput.device setFocusMode:AVCaptureFocusModeAutoFocus ];
    
}

- (void)lockExposureAndFocus{
    
    [captureInput.device lockForConfiguration:nil];
    
    //if( [captureInput.device isExposureModeSupported:AVCaptureExposureModeLocked] ) [captureInput.device setExposureMode:AVCaptureExposureModeLocked ];
    if( [captureInput.device isFocusModeSupported:AVCaptureFocusModeLocked] )    [captureInput.device setFocusMode:AVCaptureFocusModeLocked ];
    
    
}

-(void)stopCapture{
    if(self.captureSession) {
        if(captureOutput){
            if(captureOutput.sampleBufferDelegate != nil) {
                [captureOutput setSampleBufferDelegate:nil queue:NULL];
            }
        }
        
        // remove the input and outputs from session
        for(AVCaptureInput *input1 in self.captureSession.inputs) {
            [self.captureSession removeInput:input1];
        }
        for(AVCaptureOutput *output1 in self.captureSession.outputs) {
            [self.captureSession removeOutput:output1];
        }
        
        [self.captureSession stopRunning];
    }
}

-(CGImageRef)getCurrentFrame{
    return currentFrame;
}

-(TArray <FString>)listDevices{
    TArray <FString> deviceNames;
    NSArray * devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
    int i=0;
    for (AVCaptureDevice * captureDevice in devices){
        FString name = UTF8_TO_TCHAR([captureDevice.localizedName UTF8String]);
        deviceNames.Add(name);
        UE_LOG(LogVideoGrabber, Verbose,  TEXT("Device %d: %s"), i, *name);
        i++;
    }
    return deviceNames;
}

-(void)setDevice:(int)_device{
    deviceID = _device;
}

#pragma mark -
#pragma mark AVCaptureSession delegate
- (void)captureOutput:(AVCaptureOutput *)captureOutput
didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
       fromConnection:(AVCaptureConnection *)connection
{
    if(grabberPtr != NULL) {
        @autoreleasepool {
            CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
            // Lock the image buffer
            CVPixelBufferLockBaseAddress(imageBuffer,0);
            
            if( grabberPtr != NULL && !grabberPtr->bLock ){
                
                unsigned char *isrc4 = (unsigned char *)CVPixelBufferGetBaseAddress(imageBuffer);
                size_t widthIn  = CVPixelBufferGetWidth(imageBuffer);
                size_t heightIn    = CVPixelBufferGetHeight(imageBuffer);
                
                if( widthIn != grabberPtr->getWidth() || heightIn != grabberPtr->getHeight() ){
                    UE_LOG(LogVideoGrabber, Error,  TEXT("incoming image dimensions %d by %d on't match. This shouldn't happen! Returning."), widthIn , heightIn);
                    return;
                }
                grabberPtr->updatePixelsCB(isrc4, widthIn, heightIn);
            }
        }
    }
}

#pragma mark -
#pragma mark Memory management

- (void)dealloc {
    // Stop the CaptureSession
    if(self.captureSession) {
        [self stopCapture];
        self.captureSession = nil;
    }
    if(captureOutput){
        if(captureOutput.sampleBufferDelegate != nil) {
            [captureOutput setSampleBufferDelegate:nil queue:NULL];
        }
        [captureOutput release];
        captureOutput = nil;
    }
    
    captureInput = nil;
    device = nil;
    
    if(grabberPtr) {
        [self eraseGrabberPtr];
    }
    grabberPtr = nil;
    if(currentFrame) {
        // release the currentFrame image
        CGImageRelease(currentFrame);
        currentFrame = nil;
    }
    [super dealloc];
}

- (void)eraseGrabberPtr {
    grabberPtr = NULL;
}

@end

AVFoundationVideoGrabber::AVFoundationVideoGrabber()
{
    fps        = -1;
    grabber = [OSXVideoGrabber alloc];
    width = 0;
    height = 0;
    bIsInit = false;
    newFrame = false;
    bHavePixelsChanged = false;
    bLock = false;
}


AVFoundationVideoGrabber::~AVFoundationVideoGrabber()
{
    close();
    stopThread();
}

void AVFoundationVideoGrabber::close() {
    bLock = true;
    if(grabber) {
        // Stop and release the the OSXVideoGrabber
        [grabber stopCapture];
        [grabber eraseGrabberPtr];
        [grabber release];
        grabber = nil;
    }
    
    bIsInit = false;
    width = 0;
    height = 0;
    fps        = -1;
    newFrame = false;
    bHavePixelsChanged = false;
    bLock = false;
}

bool AVFoundationVideoGrabber::setup(int w, int h, bool mirrored) {
    if( grabber == nil ){
        grabber = [OSXVideoGrabber alloc];
    }
    
    grabber->grabberPtr = this;
    
    if( [grabber initCapture:fps capWidth:w capHeight:h capMirror: mirrored] ) {
        //update the pixel dimensions based on what the camera supports
        width = grabber->width;
        height = grabber->height;
        setVideoMirrored (mirrored);
        allocateData(width, height, PF_B8G8R8A8);
        [grabber startCapture];
        startThread();
        
        newFrame=false;
        bIsInit = true;
        return true;
    } else {
        return false;
    }
    
}

TArray<FVideoDevice> AVFoundationVideoGrabber::listDevices() const {
	TArray<FVideoDevice> devices;
    TArray <FString> devList = [grabber listDevices];
    for(int i = 0; i < devList.Num(); i++){
        FVideoDevice vd;
        vd.deviceName = devList[i];
        vd.id = i;
        vd.bAvailable = true;
        devices.Add(vd);
    }
	return devices;
	
}

bool AVFoundationVideoGrabber::isFrameNew() const {
	return newFrame;
}

void AVFoundationVideoGrabber::update() {
    newFrame = false;
    
    if (bHavePixelsChanged == true){
        bHavePixelsChanged = false;
        frwLock.ReadLock();
        copyDataToTexture(pixels.GetData(), width, height, 4);
        frwLock.ReadUnlock();
        newFrame = true;
    }
}

int AVFoundationVideoGrabber::getHeight() const {
    return height;
}

int AVFoundationVideoGrabber::getWidth() const {
    return width;
	
}

void AVFoundationVideoGrabber::updatePixelsCB(unsigned char *isrc, int w, int h ) {
    frwLock.WriteLock();
    if ( pixels.Num() > 0 ){
        uint32 MemorySize = w*h*4;
        FMemory::Memcpy(pixels.GetData(), isrc, MemorySize);
        bHavePixelsChanged = true;
    }
    frwLock.WriteUnlock();
}

#endif