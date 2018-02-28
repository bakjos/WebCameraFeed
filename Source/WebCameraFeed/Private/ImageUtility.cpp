// Fill out your copyright notice in the Description page of Project Settings.

#include "ImageUtility.h"

#include <IImageWrapperModule.h>
#include <Modules/ModuleManager.h>
#include <IImageWrapper.h>
#include <ObjectThumbnail.h>
#include "Misc/FileHelper.h"

ImageUtility::ImageUtility()
{
}

ImageUtility::~ImageUtility()
{
}

void ImageUtility::CompressImageArray(int32 ImageWidth, int32 ImageHeight, const TArray<FColor> &SrcData, TArray<uint8> &DstData)
{
    TArray<FColor> MutableSrcData = SrcData;
    
    // PNGs are saved as RGBA but FColors are stored as BGRA. An option to swap the order upon compression may be added at
    // some point. At the moment, manually swapping Red and Blue
    for (int32 Index = 0; Index < ImageWidth*ImageHeight; Index++)
    {
        uint8 TempRed = MutableSrcData[Index].R;
        MutableSrcData[Index].R = MutableSrcData[Index].B;
        MutableSrcData[Index].B = TempRed;
    }
    
    FObjectThumbnail TempThumbnail;
    TempThumbnail.SetImageSize(ImageWidth, ImageHeight);
    TArray<uint8>& ThumbnailByteArray = TempThumbnail.AccessImageData();
    
    // Copy scaled image into destination thumb
    int32 MemorySize = ImageWidth*ImageHeight * sizeof(FColor);
    ThumbnailByteArray.AddUninitialized(MemorySize);
    FMemory::Memcpy(ThumbnailByteArray.GetData(), MutableSrcData.GetData(), MemorySize);
    
    IImageWrapperModule& ImageWrapperModule = FModuleManager::GetModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
    TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
    if (ImageWrapper.IsValid() && ImageWrapper->SetRaw(&TempThumbnail.AccessImageData()[0], TempThumbnail.AccessImageData().Num(), ImageWidth, ImageHeight, ERGBFormat::RGBA, 8))
    {
        DstData = ImageWrapper->GetCompressed();
    }
    
}

bool ImageUtility::SaveTextureAsFile (FTexture2DRHIRef texture, const FString& fileName ) {
    struct FGetTextureData
    {
        FTexture2DRHIRef texture;
        TArray<FColor> Bitmap;
        bool bExtracted;
        FEvent* fSemaphore;
    };
    
    
    FGetTextureData* data = new FGetTextureData();
    data->texture = texture;
    data->fSemaphore = FGenericPlatformProcess::GetSynchEventFromPool(false);
    
    
    
    ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
       ObtainScreenShot,FGetTextureData*, data, data,
       {
           
           FIntRect rect  = FIntRect(0, 0, data->texture->GetSizeX(), data->texture->GetSizeY());
           data->Bitmap.Reset();
           RHICmdList.ReadSurfaceData(
                                      data->texture,
                                      rect,
                                      data->Bitmap,
                                      FReadSurfaceDataFlags()
                                      );
           data->bExtracted = data->Bitmap.Num() > 0;
           data->fSemaphore->Trigger();
       }
    );
    
    data->fSemaphore->Wait();
    FGenericPlatformProcess::ReturnSynchEventToPool(data->fSemaphore);
    bool retVal = false;
    if (data->bExtracted) {
        TArray<uint8> CompressedBitmap;
        CompressImageArray(data->texture->GetSizeX(), data->texture->GetSizeY(), data->Bitmap, CompressedBitmap);
        if ( FFileHelper::SaveArrayToFile( CompressedBitmap, *fileName ) )     {
            retVal = true;
        }
    }
    delete data;
    return retVal;
}

