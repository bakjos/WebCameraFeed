#include "AndroidVideoGrabber.h"
#include <Async/Async.h>
#if PLATFORM_ANDROID
#include <Public/Android/AndroidApplication.h>
#include <Runtime/Launch/Public/Android/AndroidJNI.h>
#include <android/log.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>


#define LOG_TAG "CameraLOG"

bool AndroidThunkCpp_startCamera(int w, int h, int fps, int texId);
void AndroidThunkCpp_stopCamera();

int SetupJNICamera(JNIEnv* env);
JNIEnv* ENV = NULL;
static jmethodID jToast;
static jmethodID AndroidThunkJava_startCamera;
static jmethodID AndroidThunkJava_stopCamera;
static jmethodID AndroidThunkJava_getNumCameras;
static jmethodID AndroidThunkJava_getCameraFacing;
static jmethodID AndroidThunkJava_getFacingOfCamera;
static jmethodID AndroidThunkJava_setDeviceID;
static AndroidVideoGrabber* currentGrabber = NULL;

static long int crv_tab[256];
static long int cbu_tab[256];
static long int cgu_tab[256];
static long int cgv_tab[256];
static long int tab_76309[256];
static unsigned char clp[1024];

void InitConvertTable()
{
	static bool inited = false;
	if (inited) return;
	long int crv, cbu, cgu, cgv;
	int i, ind;

	crv = 104597; cbu = 132201;  /* fra matrise i global.h */
	cgu = 25675;  cgv = 53279;

	for (i = 0; i < 256; i++) {
		crv_tab[i] = (i - 128) * crv;
		cbu_tab[i] = (i - 128) * cbu;
		cgu_tab[i] = (i - 128) * cgu;
		cgv_tab[i] = (i - 128) * cgv;
		tab_76309[i] = 76309 * (i - 16);
	}

	for (i = 0; i < 384; i++)
		clp[i] = 0;
	ind = 384;
	for (i = 0; i < 256; i++)
		clp[ind++] = i;
	ind = 640;
	for (i = 0; i < 384; i++)
		clp[ind++] = 255;

	inited = true;
}

void ConvertYUV2RGBA(unsigned char *src0, unsigned char *src1, unsigned char *dst_ori,
	int width, int height, bool mirror)
{
	register int y1, y2, u, v;
	register unsigned char *py1, *py2;
	register int i, j, c1, c2, c3, c4;
	register unsigned char *d1, *d2;

	int width4 = 4 * width;
	py1 = src0;
	py2 = py1 + width;
	if ( mirror ) {
		d1 = dst_ori + width4 - 1;
		d2 = d1 + width4;
	} else {
		d1 = dst_ori;
		d2 = d1 + width4;
	}
	for (j = 0; j < height; j += 2) {
		for (i = 0; i < width; i += 2) {

			v = *src1++;
			u = *src1++;

			c1 = crv_tab[v];
			c2 = cgu_tab[u];
			c3 = cgv_tab[v];
			c4 = cbu_tab[u];

			if ( mirror ) { 
				//up-left
				y1 = tab_76309[*py1++];
				*d1-- = 255;
				*d1-- = clp[384 + ((y1 + c4) >> 16)];
				*d1-- = clp[384 + ((y1 - c2 - c3) >> 16)];
				*d1-- = clp[384 + ((y1 + c1) >> 16)];
				
				//down-left
				y2 = tab_76309[*py2++];
				*d2-- = 255;
				*d2-- = clp[384 + ((y2 + c4) >> 16)];
				*d2-- = clp[384 + ((y2 - c2 - c3) >> 16)];
				*d2-- = clp[384 + ((y2 + c1) >> 16)];

				//up-right
				y1 = tab_76309[*py1++];
				*d1-- = 255;
				*d1-- = clp[384 + ((y1 + c4) >> 16)];
				*d1-- = clp[384 + ((y1 - c2 - c3) >> 16)];
				*d1-- = clp[384 + ((y1 + c1) >> 16)];
		
				//down-right
				y2 = tab_76309[*py2++];
				*d2-- = 255;
				*d2-- = clp[384 + ((y2 + c4) >> 16)];
				*d2-- = clp[384 + ((y2 - c2 - c3) >> 16)];
				*d2-- = clp[384 + ((y2 + c1) >> 16)];
			} else {
				//up-left
				y1 = tab_76309[*py1++];
				*d1++ = clp[384 + ((y1 + c1) >> 16)];
				*d1++ = clp[384 + ((y1 - c2 - c3) >> 16)];
				*d1++ = clp[384 + ((y1 + c4) >> 16)];
				*d1++ = 255;

				//down-left
				y2 = tab_76309[*py2++];
				*d2++ = clp[384 + ((y2 + c1) >> 16)];
				*d2++ = clp[384 + ((y2 - c2 - c3) >> 16)];
				*d2++ = clp[384 + ((y2 + c4) >> 16)];
				*d2++ = 255;

				//up-right
				y1 = tab_76309[*py1++];
				*d1++ = clp[384 + ((y1 + c1) >> 16)];
				*d1++ = clp[384 + ((y1 - c2 - c3) >> 16)];
				*d1++ = clp[384 + ((y1 + c4) >> 16)];
				*d1++ = 255;

				//down-right
				y2 = tab_76309[*py2++];
				*d2++ = clp[384 + ((y2 + c1) >> 16)];
				*d2++ = clp[384 + ((y2 - c2 - c3) >> 16)];
				*d2++ = clp[384 + ((y2 + c4) >> 16)];
				*d2++ = 255;
			}
		}
		if ( mirror) {
			d1 = dst_ori + (j+1)*width4 - 1;
			d2 = d1 + width4;
		} else {
			d1 += width4;
			d2 += width4;
		}
		py1 += width;
		py2 += width;
	}


}


AndroidVideoGrabber::AndroidVideoGrabber() {
	if (ENV == NULL) {
		JNIEnv* env = FAndroidApplication::GetJavaEnv();
		SetupJNICamera(env);
		InitConvertTable();
	}

	fps = -1;
	width = 0;
	height = 0;
	bIsInit = false;
	newFrame = false;
	bHavePixelsChanged = false;
	textureID = 0;
	_mirror = false;
}

AndroidVideoGrabber:: ~AndroidVideoGrabber() {
	close();
}

TArray<FVideoDevice> AndroidVideoGrabber::listDevices() const {
	TArray<FVideoDevice> devices;
	int numDevices = getNumCameras();
	for(int i = 0; i < numDevices; i++){
		int facing = getFacingOfCamera(i);
		FVideoDevice vd;
		vd.deviceName = facing == 0? TEXT("Back") : TEXT("Front");
		vd.id = i;
		vd.bAvailable = true;
		devices.Add(vd);
	}
	return devices;
}

void AndroidVideoGrabber::loadTexture() {
	
	GLuint TextureID = 0;
	glGenTextures(1, &TextureID);

	glEnable(GL_TEXTURE_EXTERNAL_OES);
	glBindTexture(GL_TEXTURE_EXTERNAL_OES, TextureID);

	glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


	glDisable(GL_TEXTURE_EXTERNAL_OES);

	textureID = TextureID;
	
}

void AndroidVideoGrabber::deleteTexture () {
	if ( textureID != 0) {
		GLuint TextureID = TextureID;
		glDeleteTextures(1, &TextureID);
		textureID = 0;
	}
}

void AndroidVideoGrabber::setDeviceID(int deviceID) {

	this->deviceID = deviceID;

	if (!AndroidThunkJava_setDeviceID || !ENV) return;
	FJavaWrapper::CallVoidMethod(ENV, FJavaWrapper::GameActivityThis, AndroidThunkJava_setDeviceID, deviceID);
}

bool AndroidVideoGrabber::isFrameNew() const {
	return newFrame;
}

int AndroidVideoGrabber::getHeight() const {
	return height;
}

int AndroidVideoGrabber::getWidth() const {
	return width;
}

bool  AndroidVideoGrabber::setup(int w, int h, bool mirrored) {
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "Called setup");
	loadTexture();

	if ( AndroidThunkCpp_startCamera(w, h, -1, textureID) ) {
		setVideoMirrored(false);
		_mirror = mirrored;
		width = w;
		height = h;
		allocateData(width, height, PF_B8G8R8A8);
		startThread();
		bIsInit = true;
		registerDelegates();
		currentGrabber = this;
		return true;
	}
	return false;
}

void  AndroidVideoGrabber::close() {
	currentGrabber = NULL;
	stopThread();
	AndroidThunkCpp_stopCamera();
	deleteTexture();
	unRegisterDelegates();
	bIsInit = false;
	width = 0;
	height = 0;
	fps = -1;
	newFrame = false;
	bHavePixelsChanged = false;
}

void  AndroidVideoGrabber::update() {

	if ( !bIsInit) return;

	newFrame = false;

	if ( currentGrabber == NULL) {
		currentGrabber = this;
	}

	if (bHavePixelsChanged ) {
		bHavePixelsChanged = false;
		frwLock.WriteLock();

		if (cameraTexture.IsValid()) {
			FUpdateTextureRegion2D region(0, 0, 0, 0, width, height);
			cameraTexture->UpdateTextureRegions(0, 1, &region, width*4, 4, pixels.GetData());
		}
		frwLock.WriteUnlock();
		newFrame = true;	
	}
} 

void AndroidVideoGrabber::updatePixelsCB(unsigned char *isrc, int w, int h) {
	if (!bIsInit) return;
    
	if (w != width || h != height || !cameraTexture.IsValid()) {
		if (!cameraTexture.IsValid()) {
			UE_LOG(LogVideoGrabber, Warning, TEXT("The texture is invalid, reallocating"));
			__android_log_print(ANDROID_LOG_WARN, LOG_TAG, "The texture is invalid, reallocating");
		}
		else {
			UE_LOG(LogVideoGrabber, Warning, TEXT("The incoming image dimensions %d by %d don't match with the current dimensions %d by %d"), w, h, width, height);
			__android_log_print(ANDROID_LOG_WARN, LOG_TAG, "The incoming image dimensions %d by %d don't match with the current dimensions %d by %d", w, h, width, height );
		}

		FEvent* fSemaphore = FGenericPlatformProcess::GetSynchEventFromPool(false);
		AsyncTask(ENamedThreads::GameThread, [this, w, h, fSemaphore]() {
			this->resizeData(w, h, PF_B8G8R8A8);
			__android_log_print(ANDROID_LOG_INFO, LOG_TAG, "Resized data");
			fSemaphore->Trigger();
		});

		fSemaphore->Wait();
		FGenericPlatformProcess::ReturnSynchEventToPool(fSemaphore);
		width = w;
		height = h;

	}
	frwLock.ReadLock();
	if (pixels.Num() > 0) {
		
		if ( copyPixels.Num() != pixels.Num()) {
			copyPixels.Reset(pixels.Num());
		}
		frwLock.ReadUnlock();
	
		ConvertYUV2RGBA(isrc, // y component
			isrc + (w * h),  // uv components
			copyPixels.GetData(), w, h, _mirror);

		frwLock.WriteLock();
		uint32 MemorySize = w*h*4;
		FMemory::Memcpy(pixels.GetData(), copyPixels.GetData(), MemorySize);
		bHavePixelsChanged = true;
		frwLock.WriteUnlock();
	} else {
		frwLock.ReadUnlock();
	}
	
}


int SetupJNICamera(JNIEnv* env)
{
	if (!env) return JNI_ERR;

	ENV = env;

	AndroidThunkJava_startCamera = FJavaWrapper::FindMethod(ENV, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_startCamera", "(IIII)Z", false);
	if (!AndroidThunkJava_startCamera) {
		__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "ERROR: AndroidThunkJava_startCamera Method cant be found T_T ");
		return JNI_ERR;
	}

	AndroidThunkJava_stopCamera = FJavaWrapper::FindMethod(ENV, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_stopCamera", "()V", false);
	if (!AndroidThunkJava_stopCamera) {
		__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "ERROR: AndroidThunkJava_stopCamera Method cant be found T_T ");
		return JNI_ERR;
	}

	//FJavaWrapper::CallVoidMethod(ENV, FJavaWrapper::GameActivityThis, jToast);
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "module load success!!! ^_^");

	AndroidThunkJava_getNumCameras = FJavaWrapper::FindMethod(ENV, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_getNumCameras", "()I", false);

	if (!AndroidThunkJava_getNumCameras) {
		__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "ERROR: AndroidThunkJava_getNumCameras Method cant be found T_T ");
		return JNI_ERR;
	}

	AndroidThunkJava_getCameraFacing = FJavaWrapper::FindMethod(ENV, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_getCameraFacing", "(I)I", false);

	if (!AndroidThunkJava_getCameraFacing) {
		__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "ERROR: AndroidThunkJava_getCameraFacing Method cant be found T_T ");
		return JNI_ERR;
	}

	AndroidThunkJava_getFacingOfCamera = FJavaWrapper::FindMethod(ENV, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_getFacingOfCamera", "(I)I", false);

	if (!AndroidThunkJava_getFacingOfCamera) {
		__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "ERROR: AndroidThunkJava_getFacingOfCamera Method cant be found T_T ");
		return JNI_ERR;
	}

	AndroidThunkJava_setDeviceID = FJavaWrapper::FindMethod(ENV, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_setDeviceID", "(I)V", false);

	if (!AndroidThunkJava_setDeviceID) {
		__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "ERROR: AndroidThunkJava_setDeviceID Method cant be found T_T ");
		return JNI_ERR;
	}


	return JNI_OK;
}

bool  AndroidVideoGrabber::switchBackAndFront() {
	if ( getFacingOfCamera(deviceID) == 0) {
		//Back to front
		deviceID = getFrontCamera();
		__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "Switching to front camera");
	} else {
		deviceID = getBackCamera();
		__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "Switching to backCamera camera");
	}
	AndroidThunkCpp_stopCamera();
	setDeviceID(deviceID);
	return AndroidThunkCpp_startCamera(width, height, -1, textureID);
}

void AndroidVideoGrabber::pause() {
	bIsInit = false;
	currentGrabber = NULL;
	stopThread();
	AndroidThunkCpp_stopCamera();
	deleteTexture();
}

void  AndroidVideoGrabber::resume() {
	loadTexture();
	AndroidThunkCpp_startCamera(width, height, -1, textureID);
	startThread();
	currentGrabber = this;
	bIsInit = true;
}

int  AndroidVideoGrabber::getNumCameras() const {
	if (!AndroidThunkJava_getNumCameras || !ENV) return 0;
	return FJavaWrapper::CallIntMethod(ENV, FJavaWrapper::GameActivityThis, AndroidThunkJava_getNumCameras);
}


int  AndroidVideoGrabber::getBackCamera() const {
	return getCameraFacing(0);
}

int  AndroidVideoGrabber::getFrontCamera() const {
	int id = getCameraFacing(1);
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "Returned camera ID for front camera: %d", id);
	return id;
}


int  AndroidVideoGrabber::getCameraFacing(int facing) const {
	if (!AndroidThunkJava_getCameraFacing || !ENV) return 0;
	return FJavaWrapper::CallIntMethod(ENV, FJavaWrapper::GameActivityThis, AndroidThunkJava_getCameraFacing, facing);
}


int AndroidVideoGrabber::getFacingOfCamera(int device) const {
	if (!AndroidThunkJava_getFacingOfCamera || !ENV) return 0;
	return FJavaWrapper::CallIntMethod(ENV, FJavaWrapper::GameActivityThis, AndroidThunkJava_getFacingOfCamera, device);
}

bool  AndroidThunkCpp_startCamera(int w, int h, int fps, int texID)
{
	if (!AndroidThunkJava_startCamera || !ENV) return false;
	bool v = FJavaWrapper::CallBooleanMethod(ENV, FJavaWrapper::GameActivityThis, AndroidThunkJava_startCamera, w, h, fps, texID);
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "start camera %d ", (int)v);
	return true;
}


void AndroidThunkCpp_stopCamera()
{
	if (!AndroidThunkJava_stopCamera || !ENV) return;
	FJavaWrapper::CallVoidMethod(ENV, FJavaWrapper::GameActivityThis, AndroidThunkJava_stopCamera);
	__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "shutdown camera");
}

extern "C" bool Java_com_epicgames_ue4_GameActivity_nativeGetFrameData(JNIEnv* LocalJNIEnv, jobject LocalThiz, jint frameWidth, jint frameHeight, jbyteArray data) {
	if ( currentGrabber != NULL) {
		//get the new frame
		int length = LocalJNIEnv->GetArrayLength(data);
		jboolean isCopy;

		auto currentFrame = (unsigned char *)LocalJNIEnv->GetByteArrayElements(data, &isCopy);
		if (currentFrame) {
			currentGrabber->updatePixelsCB(currentFrame, frameWidth, frameHeight);
			LocalJNIEnv->ReleaseByteArrayElements(data, (jbyte*)currentFrame, 0);
		}
		
	} 
	return JNI_TRUE;
}


#endif
