// Fill out your copyright notice in the Description page of Project Settings.

#include "WebCameraComponent.h"


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

	// ...
	
}


// Called every frame
void UWebCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

