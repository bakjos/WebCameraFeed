// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseVideoGrabber.h"
#include "ImageUtility.h"
#include <UnrealEngine.h>
#include <Classes/Engine/World.h>
#include <SceneInterface.h>
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

void BaseVideoGrabber::allocateData(int w, int h, EPixelFormat InFormat) {
	frwLock.WriteLock();
	uint32 MemorySize = w*h * 4;
	pixels.Reset();
	pixels.AddUninitialized(MemorySize);
	FMemory::Memzero(pixels.GetData(), MemorySize);
    
    
    
    cameraTexture = UTexture2D::CreateTransient(w, h, InFormat);
	FTexture2DMipMap& Mip = cameraTexture->PlatformData->Mips[0];
	void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(Data, pixels.GetData(), MemorySize);
	Mip.BulkData.Unlock();
	cameraTexture->UpdateResource();
	
	if (mirrored) {
		mirroredTexture = NewObject<UTextureRenderTarget2D>();
		mirroredTexture->InitCustomFormat(w, h, PF_B8G8R8A8, false);
	}
    
    frwLock.WriteUnlock();
}

void BaseVideoGrabber::resizeData(int w, int h, EPixelFormat InFormat) {
    
    if ( !cameraTexture.IsValid()) {
        allocateData(w, h, InFormat);
    } else {
        frwLock.WriteLock();
        uint32 MemorySize = w*h * 4;
        pixels.Reset();
        pixels.AddUninitialized(MemorySize);
        FMemory::Memzero(pixels.GetData(), MemorySize);
        frwLock.WriteUnlock();
        
        cameraTexture->ReleaseResource();
        
        // Allocate first mipmap.
        int32 NumBlocksX = w / GPixelFormats[InFormat].BlockSizeX;
        int32 NumBlocksY = h / GPixelFormats[InFormat].BlockSizeY;
        FTexture2DMipMap& Mip = cameraTexture->PlatformData->Mips[0];
        Mip.SizeX = w;
        Mip.SizeY = h;
        Mip.BulkData.Lock(LOCK_READ_WRITE);
        Mip.BulkData.Realloc(NumBlocksX * NumBlocksY * GPixelFormats[InFormat].BlockBytes);
        Mip.BulkData.Unlock();
        
        cameraTexture->UpdateResource();
        
        	
        
        if (mirrored) {
            if (!mirroredTexture.IsValid()) {
                mirroredTexture = NewObject<UTextureRenderTarget2D>();
            }
            mirroredTexture->InitCustomFormat(w, h, PF_B8G8R8A8, false);
        }
        
    }
    
}

void  BaseVideoGrabber::mirrorTexture_RenderThread(FRHICommandList& RHICmdList, FTexture2DRHIRef TextureRHIRef, FTextureRenderTargetResource* MirrorTextureRef, FDepthStencilStateRHIParamRef DepthStencilState) {
	if (MirrorTextureRef != nullptr) {
		::SetRenderTarget(RHICmdList, MirrorTextureRef->GetRenderTargetTexture(), FTextureRHIRef());

		RHICmdList.SetViewport(
			0, 0, 0.f,
			TextureRHIRef->GetSizeX(), TextureRHIRef->GetSizeY(), 1.f);
        
        ERHIFeatureLevel::Type FeatureLevel = GMaxRHIFeatureLevel;
        
        UWorld * world = GEngine->GetWorld();
        
        if ( world != nullptr) {
            if ( world->Scene != nullptr) {
                FeatureLevel = world->Scene->GetFeatureLevel();
            }
        }

		
        
        TShaderMap<FGlobalShaderType>* ShaderMap = GetGlobalShaderMap(FeatureLevel);

		TShaderMapRef<FWebCameraMirrorVS> VertexShader(ShaderMap);
		TShaderMapRef<FWebCameraMirrorPS> PixelShader(ShaderMap);


		FGraphicsPipelineStateInitializer GraphicsPSOInit;
		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
		GraphicsPSOInit.DepthStencilState = DepthStencilState;
		GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
		GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
		GraphicsPSOInit.PrimitiveType = PT_TriangleStrip;
		GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GTextureVertexDeclaration.VertexDeclarationRHI;
		GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VertexShader);
		GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(*PixelShader);

		SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);
		PixelShader->SetParameters(RHICmdList, TextureRHIRef, true);

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
}

void BaseVideoGrabber::copyDataToTexture(unsigned char * pData, int TextureWidth, int TextureHeight, int numColors) {


	if (cameraTexture.IsValid()) {
		FUpdateTextureRegion2D region(0, 0, 0, 0, TextureWidth, TextureHeight);
		cameraTexture->UpdateTextureRegions(0, 1, &region, TextureWidth*numColors, numColors, pData);
		if ( mirrored && mirroredTexture.IsValid()) {

			struct FUpdateTextureRegionsData
			{
				FTextureRenderTargetResource* MirrorTextureResource;
				FTexture2DRHIRef	TextureRHIRef;
			};
			FUpdateTextureRegionsData* RegionData = new FUpdateTextureRegionsData;
			RegionData->MirrorTextureResource = static_cast<FTextureRenderTarget2DResource*>( mirroredTexture->Resource );
			RegionData->TextureRHIRef = static_cast<FTexture2DResource*>(cameraTexture->Resource)->GetTexture2DRHI();
			
			ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
			UpdateTextureRegionsData,
			FUpdateTextureRegionsData*, RegionData, RegionData,
			{
				mirrorTexture_RenderThread(RHICmdList, RegionData->TextureRHIRef, RegionData->MirrorTextureResource, TStaticDepthStencilState<false, CF_Always>::GetRHI());
				delete RegionData;
			});

		}
	}


}

UTexture* BaseVideoGrabber::getTexture() {
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
	UTexture* texture = getTexture();
	if ( texture != nullptr ) {
		return ImageUtility::SaveTextureAsFile(mirrored?
			static_cast<FTextureRenderTarget2DResource*>(texture->Resource)->GetTextureRHI(): 
			static_cast<FTexture2DResource*>(cameraTexture->Resource)->GetTexture2DRHI(), fileName);
	}
	return false;
}


void BaseVideoGrabber::registerDelegates() {
    FCoreDelegates::ApplicationWillEnterBackgroundDelegate.AddRaw(this, &BaseVideoGrabber::ApplicationWillDeactivateDelegate_Handler);
    FCoreDelegates::ApplicationHasEnteredForegroundDelegate.AddRaw(this, &BaseVideoGrabber::ApplicationHasReactivatedDelegate_Handler);
}

void BaseVideoGrabber::unRegisterDelegates() {
    FCoreDelegates::ApplicationWillEnterBackgroundDelegate.RemoveAll(this);
    FCoreDelegates::ApplicationHasEnteredForegroundDelegate.RemoveAll(this);
}

void BaseVideoGrabber::ApplicationWillDeactivateDelegate_Handler() {
    frwLock.WriteLock();
    pause();
    frwLock.WriteUnlock();
}

void BaseVideoGrabber::ApplicationHasReactivatedDelegate_Handler() {
    frwLock.WriteLock();
    resume();
    frwLock.WriteUnlock();
}



