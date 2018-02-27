// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include <Engine/Texture2D.h>

/**
 * 
 */
class WEBCAMERAFEED_API ImageUtility
{
public:
    static void CompressImageArray(int32 ImageWidth, int32 ImageHeight, const TArray<FColor> &SrcData, TArray<uint8> &DstData);
    
    static bool SaveTextureAsFile (UTexture2D* texture, const FString& fileName );
    
private:
	ImageUtility();
	~ImageUtility();
};
