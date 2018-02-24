// Fill out your copyright notice in the Description page of Project Settings.

#include "AVFoundationVideoGrabber.h"

#if PLATFORM_MAC
AVFoundationVideoGrabber::AVFoundationVideoGrabber()
{
}

AVFoundationVideoGrabber::~AVFoundationVideoGrabber()
{
}

TArray<FVideoDevice> AVFoundationVideoGrabber::listDevices() const {
	//TODO:
	TArray<FVideoDevice> devices;
	return devices;
	
}

bool AVFoundationVideoGrabber::isFrameNew() const {
	
}

void AVFoundationVideoGrabber::close() {
	
}

void AVFoundationVideoGrabber::update() {
	
}

bool AVFoundationVideoGrabber::setup(int w, int h) {
	
}

int AVFoundationVideoGrabber::getHeight() const {
	
}

int AVFoundationVideoGrabber::getWidth() const {
	
}

#endif
