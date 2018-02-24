#pragma once

#include "SImage.h"
#include "VideoGrabber.h"

class WEBCAMERAFEED_API SWebCameraImage: public SImage {
public:

	SWebCameraImage();

	virtual void Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime ) override;

	void SetVideoGrabber(TSharedPtr<VideoGrabber> videoGrabber);
	
protected:
	TSharedPtr<VideoGrabber> videoGrabber;

	FSlateBrush Brush;
};