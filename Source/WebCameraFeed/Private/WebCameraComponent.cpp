// Fill out your copyright notice in the Description page of Project Settings.

#include "WebCameraComponent.h"
#include "VideoGrabberPool.h"


// Sets default values for this component's properties
UWebCameraComponent::UWebCameraComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	DeviceId.selectedDevice = 0;
	requestedWidth = 640;
	requestedHeight = 480;
}


// Called when the game starts
void UWebCameraComponent::BeginPlay()
{
	Super::BeginPlay();
	currentVideoGrabber = VideoGrabberPool::GetVideoGrabber(DeviceId.selectedDevice, requestedWidth, requestedHeight);
	
}

void UWebCameraComponent::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);
	if (currentVideoGrabber.IsValid()) {
		VideoGrabberPool::ReleaseVideoGrabber(currentVideoGrabber);
	}
}


// Called every frame
void UWebCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

UTexture2D* UWebCameraComponent::GetTexture() {
	if ( currentVideoGrabber.IsValid()) {
		return currentVideoGrabber->getTexture();
	}
	return nullptr;
}

