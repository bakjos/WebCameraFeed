#pragma once

#include "CoreMinimal.h"

#include "CoreMinimal.h"
#include "UObject/UnrealType.h"
#include "Engine/UserDefinedStruct.h"
#include "WebCameraDevice.generated.h"



UENUM(BluePrintType)
enum class EVideoPixelFormat : uint8 {
	GrayScale, 
	Rgb, 
	Rgba
};


USTRUCT(meta = (DisplayName = "Video Format"))
struct FVideoFormat {
	GENERATED_USTRUCT_BODY()

	EVideoPixelFormat format;

	int width;

	int height;

	TArray<float> frameRates;
};

USTRUCT(meta = (DisplayName = "Video Format"))
struct FVideoDevice {
	GENERATED_USTRUCT_BODY()

	int id;

	FString deviceName;

	FString hardwareName;

	FString serialID;

	TArray<FVideoFormat> formats;

	bool   bAvailable;

};

USTRUCT(meta = (DisplayName = "Device Id"))
struct FWebCameraDeviceId
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditAnywhere)
	int selectedDevice;

};


