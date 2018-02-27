// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseVideoGrabber.h"
#include "ImageUtility.h"
#include <UnrealEngine.h>
#include <Classes/Engine/World.h>
#include <Public/GlobalShader.h>
#include <Public/PipelineStateCache.h>
#include <Public/RHIStaticStates.h>
#include <Public/RHIUtilities.h>
#include <Engine/TextureRenderTarget2D.h>

TGlobalResource<FTextureVertexDeclaration> GTextureVertexDeclaration;

DEFINE_LOG_CATEGORY(LogVideoGrabber)

BaseVideoGrabber::BaseVideoGrabber()
{
	_running = false;
	runnableThread = nullptr;
	deviceID  = 0;
    mirrored = false;
}

void BaseVideoGrabber::setDeviceID(int _deviceID) {
	deviceID = _deviceID;
}

int BaseVideoGrabber::getDeviceID() {
	return deviceID;
}
BaseVideoGrabber::~BaseVideoGrabber(){
	stopThread();
}

void BaseVideoGrabber::startThread() {
	stopThread();
	_running = true;
	runnableThread = FRunnableThread::Create(this, TEXT("VideoGrabberThread"));
}

void BaseVideoGrabber::stopThread() {
    
	if (_running && runnableThread != nullptr) {
        _running = false;
		runnableThread->Kill();
		runnableThread= nullptr;
	}
}

uint32 BaseVideoGrabber::Run() {
	while (_running)
	{
		update();
		FPlatformProcess::Sleep(0.016);
	}
	
	return 0;
}

void BaseVideoGrabber:: allocateData(int w, int h, EPixelFormat InFormat) {
	
	uint32 MemorySize = w*h * 4;
	pixels.Reset();
	pixels.AddUninitialized(MemorySize);
	FMemory::Memzero(pixels.GetData(), MemorySize);
    
    
    
    struct FUpdateTextureResourceData
    {
        UTexture2D* texture;
        UTexture2D* mirroredTexture;
    };
   
    
    
    cameraTexture = UTexture2D::CreateTransient(w, h, InFormat);
    
    FUpdateTextureResourceData* data = new FUpdateTextureResourceData();
    data->texture = cameraTexture.Get();


    if ( mirrored) {
        mirroredTexture = UTexture2D::CreateTransient(w, h, InFormat);
        data->mirroredTexture = mirroredTexture.Get();
    } else {
        data->mirroredTexture = nullptr;
    }
   
    
	ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
	UpdateTextureResource,
	FUpdateTextureResourceData*, data, data,
	{
		data->texture->UpdateResource();
        if ( data->mirroredTexture != nullptr) {
            data->mirroredTexture->UpdateResource();
            
        }
        delete data;
	});
    
    
    

}

void BaseVideoGrabber::copyDataToTexture(unsigned char * pData, int TextureWidth, int TextureHeight, int numColors) {
	struct FUpdateTextureRegionsData
	{
		FTexture2DResource* Texture2DResource;
        FTexture2DResource* Mirrored2DResource;
		int32 MipIndex;
		FUpdateTextureRegion2D Region;
		uint32 SrcPitch;
		uint32 SrcBpp;
		uint8* SrcData;
        FDepthStencilStateRHIParamRef DepthStencilState;
	};
    
    static FGlobalBoundShaderState BoundShaderState;
    
	FUpdateTextureRegionsData* RegionData = new FUpdateTextureRegionsData;
	if ( cameraTexture.IsValid() ) {
		RegionData->Texture2DResource = (FTexture2DResource*)cameraTexture->Resource;
		RegionData->MipIndex = 0;
		RegionData->Region = FUpdateTextureRegion2D(0, 0, 0, 0, TextureWidth, TextureHeight);;
		RegionData->SrcPitch = (uint32)(numColors * TextureWidth); //TODO: Clap to 32bits
		RegionData->SrcBpp = (uint32)numColors;
		RegionData->SrcData = pData;

        if ( mirrored && mirroredTexture.IsValid()) {
            RegionData->Mirrored2DResource = (FTexture2DResource*)mirroredTexture->Resource;
            RegionData->DepthStencilState= TStaticDepthStencilState<false, CF_Always>::GetRHI();
            
        } else {
            RegionData->Mirrored2DResource = nullptr;
        }
        
        
        ERHIFeatureLevel::Type FeatureLevel = GMaxRHIFeatureLevel;
        if ( GEngine->GetWorld() != nullptr && GEngine->GetWorld()->Scene != nullptr ) {
            FeatureLevel = GEngine->GetWorld()->Scene->GetFeatureLevel();
        }

		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
			UpdateTextureRegionsData,
			FUpdateTextureRegionsData*, RegionData, RegionData,
			{
				int32 CurrentFirstMip = RegionData->Texture2DResource->GetCurrentFirstMip();
				if (RegionData->MipIndex >= CurrentFirstMip)
				{
					RHIUpdateTexture2D(RegionData->Texture2DResource->GetTexture2DRHI(),
						RegionData->MipIndex - CurrentFirstMip,
						RegionData->Region,
						RegionData->SrcPitch,
						RegionData->SrcData + RegionData->Region.SrcY * RegionData->SrcPitch + RegionData->Region.SrcX * RegionData->SrcBpp);
				}
                
                if ( RegionData->Mirrored2DResource != nullptr) {
                    
                  
                    ::SetRenderTarget(RHICmdList, RegionData->Mirrored2DResource->GetTexture2DRHI(), FTextureRHIRef());
                    
                    RHICmdList.SetViewport(
                                           0, 0, 0.f,
                                           RegionData->Mirrored2DResource->GetSizeX(), RegionData->Mirrored2DResource->GetSizeY(), 1.f);
                    
                    ERHIFeatureLevel::Type FeatureLevel = GMaxRHIFeatureLevel;
                    
                    TShaderMapRef<FWebCameraMirrorVS> VertexShader(GetGlobalShaderMap(FeatureLevel));
                    TShaderMapRef<FWebCameraMirrorPS> PixelShader(GetGlobalShaderMap(FeatureLevel));
                    
                    
                    FGraphicsPipelineStateInitializer GraphicsPSOInit;
                    RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
                    GraphicsPSOInit.DepthStencilState = RegionData->DepthStencilState;
                    GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
                    GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
                    GraphicsPSOInit.PrimitiveType = PT_TriangleStrip;
                    GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GTextureVertexDeclaration.VertexDeclarationRHI;
                    GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VertexShader);
                    GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(*PixelShader);
                    
                    SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);
                    
        
                    PixelShader->SetParameters(RHICmdList,  RegionData->Texture2DResource->GetTexture2DRHI(), true );
                    
                    // Draw a fullscreen quad that we can run our pixel shader on
                    FTextureVertex Vertices[4];
                    Vertices[0].Position = FVector4(-1.0f, 1.0f, 0, 1.0f);
                    Vertices[1].Position = FVector4(1.0f, 1.0f, 0, 1.0f);
                    Vertices[2].Position = FVector4(-1.0f, -1.0f, 0, 1.0f);
                    Vertices[3].Position = FVector4(1.0f, -1.0f, 0, 1.0f);
                    Vertices[0].UV = FVector2D(0, 0);
                    Vertices[1].UV = FVector2D(1, 0);
                    Vertices[2].UV = FVector2D(0, 1);
                    Vertices[3].UV = FVector2D(1, 1);
                    
                    DrawPrimitiveUP(RHICmdList, PT_TriangleStrip, 2, Vertices, sizeof(Vertices[0]));
                    
                }

				delete RegionData;
			});
	}
}

UTexture2D* BaseVideoGrabber::getTexture() {
    if ( mirrored && mirroredTexture.IsValid()) {
        return mirroredTexture.Get();
    }
    
	if (cameraTexture.IsValid()) {
		return cameraTexture.Get();
	}
	return nullptr;
}

bool BaseVideoGrabber::isVideoMirrored() {
    return mirrored;
}

void BaseVideoGrabber::setVideoMirrored( bool mirrored ) {
    this->mirrored = mirrored;
}


bool BaseVideoGrabber::saveTextureAsFile ( const FString& fileName ) {
    return ImageUtility::SaveTextureAsFile(cameraTexture.Get(), fileName);
}
