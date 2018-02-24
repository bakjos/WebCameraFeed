#include "SWebCameraImage.h"



SWebCameraImage::SWebCameraImage() {
	bCanTick = true;
}


void SWebCameraImage::Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime ) {
	if ( videoGrabber.IsValid() && videoGrabber->isFrameNew() ) {
		UTexture2D* texture = (UTexture2D*)videoGrabber->getTexture();
		Brush.SetResourceObject(texture);
		Brush.ImageSize.X = texture->GetSizeX();
		Brush.ImageSize.Y = texture->GetSizeY();
		Image = &Brush;
		Invalidate(EInvalidateWidget::LayoutAndVolatility);
	}
}

void SWebCameraImage::SetVideoGrabber(TSharedPtr<VideoGrabber> videoGrabber) {
	this->videoGrabber = videoGrabber;
}