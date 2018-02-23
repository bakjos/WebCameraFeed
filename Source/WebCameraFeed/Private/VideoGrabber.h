#pragma once 

#if PLATFORM_WINDOWS
	#include "DirectShowVideoGrabber.h"
	typedef DirectShowVideoGrabber VideoGrabber;
#endif

#if PLATFORM_MAC
	#include "AVFoundationVideoGrabber.h"
	typedef AVFoundationVideoGrabber VideoGrabber;
#endif 
