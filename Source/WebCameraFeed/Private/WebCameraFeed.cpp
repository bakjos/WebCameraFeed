// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "WebCameraFeed.h"
#include "VideoGrabberPool.h"

#define LOCTEXT_NAMESPACE "FWebCameraFeedModule"

#if WITH_EDITOR
#include "PropertyEditorModule.h"
#include "WebCameraDeviceIdCustomization.h"
#endif

void FWebCameraFeedModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
#if WITH_EDITOR
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomPropertyTypeLayout("WebCameraDeviceId",  FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FWebCameraDeviceIdCustomization::MakeInstance));
#endif
}

void FWebCameraFeedModule::ShutdownModule()
{
	VideoGrabberPool::ReleaseInstance();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FWebCameraFeedModule, WebCameraFeed)