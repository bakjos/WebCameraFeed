// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseVideoGrabber.h"
#include "ImageUtility.h"
#include <UnrealEngine.h>
#include <Engine/World.h>
#include <SceneInterface.h>
#include <GlobalShader.h>
#include <PipelineStateCache.h>
#include <RHIStaticStates.h>
#include <RHIUtilities.h>
#include <Rendering/Texture2DResource.h>
#include <Engine/TextureRenderTarget2D.h>


TGlobalResource<FTextureVertexDeclaration> GTextureVertexDeclaration;


DEFINE_LOG_CATEGORY(LogVideoGrabber)

FVertexBufferRHIRef CreateQuadVertexBuffer()
{
    FRHIResourceCreateInfo CreateInfo;
    FVertexBufferRHIRef VertexBufferRHI = RHICreateVertexBuffer(sizeof(FTextureVertex) * 4, BUF_Volatile, CreateInfo);
    void* VoidPtr = RHILockVertexBuffer(VertexBufferRHI, 0, sizeof(FTextureVertex) * 4, RLM_WriteOnly);
    
    FTextureVertex* Vertices = (FTextureVertex*)VoidPtr;
    Vertices[0].Position = FVector4(-1.0f, 1.0f, 0, 1.0f);
    Vertices[1].Position = FVector4(1.0f, 1.0f, 0, 1.0f);
    Vertices[2].Position = FVector4(-1.0f, -1.0f, 0, 1.0f);
    Vertices[3].Position = FVector4(1.0f, -1.0f, 0, 1.0f);
    Vertices[0].UV = FVector2D(0, 0);
    Vertices[1].UV = FVector2D(1, 0);
    Vertices[2].UV = FVector2D(0, 1);
    Vertices[3].UV = FVector2D(1, 1);
    RHIUnlockVertexBuffer(VertexBufferRHI);
    
    return VertexBufferRHI;
}

BaseVideoGrabber::BaseVideoGrabber()
{
    _running = false;
    runnableThread = NULL;
    deviceID  = 0;
    mirrored = false;
    paused = false;
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
    
    if (_running && runnableThread) {
        _running = false;
        runnableThread->Kill();
        runnableThread= NULL;
    }
}

uint32 BaseVideoGrabber::Run() {
    paused = false;
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
        
        frwLock.WriteUnlock();
        
    }
    
}

void  BaseVideoGrabber::mirrorTexture_RenderThread(FRWLock& frwLock, FRHICommandList& RHICmdList, FTextureResource* TextureResource, FTextureRenderTargetResource* MirrorTextureRef, FRHIDepthStencilState* DepthStencilState) {
    
    frwLock.ReadLock();
    
    if (MirrorTextureRef) {
        
        
        const FTexture2DRHIRef& RenderTargetTexture = MirrorTextureRef->GetRenderTargetTexture();
        
        RHICmdList.Transition(FRHITransitionInfo(RenderTargetTexture, ERHIAccess::SRVMask, ERHIAccess::RTV));
        
        FRHIRenderPassInfo RPInfo(RenderTargetTexture, ERenderTargetActions::DontLoad_Store, MirrorTextureRef->TextureRHI);
        RHICmdList.BeginRenderPass(RPInfo, TEXT("DrawMirrorTexture"));
        {
            // Update viewport.
            RHICmdList.SetViewport(
                                   0, 0, 0.f,
                                   MirrorTextureRef->GetSizeX(), MirrorTextureRef->GetSizeY(), 1.f);
            
            
            ERHIFeatureLevel::Type FeatureLevel = GMaxRHIFeatureLevel;
            
            UWorld * world = GEngine->GetWorld();
            
            if (world && world->Scene) {
                FeatureLevel = world->Scene->GetFeatureLevel();
            }
            
            // Get shaders.
            FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(FeatureLevel);
            TShaderMapRef<FWebCameraMirrorVS> VertexShader(GlobalShaderMap);
            TShaderMapRef<FWebCameraMirrorPS> PixelShader(GlobalShaderMap);
            
            // Set the graphic pipeline state.
            FGraphicsPipelineStateInitializer GraphicsPSOInit;
            RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
            GraphicsPSOInit.DepthStencilState = DepthStencilState;
            GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
            GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
            GraphicsPSOInit.PrimitiveType = PT_TriangleStrip;
            GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GTextureVertexDeclaration.VertexDeclarationRHI;
            GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
            GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
            SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);
            
            // Update shader uniform parameters.
            PixelShader->SetParameters(RHICmdList, PixelShader.GetPixelShader(), TextureResource, true);
            
            
            // Draw a fullscreen quad that we can run our pixel shader on
#if (ENGINE_MINOR_VERSION >= 21)
            FVertexBufferRHIRef VertexBufferRHI = CreateQuadVertexBuffer();
            
            RHICmdList.SetStreamSource(0, VertexBufferRHI, 0);
#if (ENGINE_MINOR_VERSION >= 23)
            RHICmdList.DrawPrimitive(0, 2, 1);
#else
            RHICmdList.DrawPrimitive(PT_TriangleStrip, 0, 2, 1);
#endif
            VertexBufferRHI.SafeRelease();
            
#else
            FTextureVertex Vertices[4];
            Vertices[0].Position = FVector4(-1.0f, 1.0f, 0, 1.0f);
            Vertices[1].Position = FVector4(1.0f, 1.0f, 0, 1.0f);
            Vertices[2].Position = FVector4(-1.0f, -1.0f, 0, 1.0f);
            Vertices[3].Position = FVector4(1.0f, -1.0f, 0, 1.0f);
            Vertices[0].UV = FVector2D(0, 0);
            Vertices[1].UV = FVector2D(1, 0);
            Vertices[2].UV = FVector2D(0, 1);
            Vertices[3].UV = FVector2D(1, 1);
            
            // Deprecated in 4.21
            DrawPrimitiveUP(RHICmdList, PT_TriangleStrip, 2, Vertices, sizeof(Vertices[0]));
            
#endif
            
            
        }
        RHICmdList.EndRenderPass();
        RHICmdList.Transition(FRHITransitionInfo(RenderTargetTexture, ERHIAccess::RTV, ERHIAccess::SRVMask));
    }
    
    frwLock.ReadUnlock();
}

void BaseVideoGrabber::copyDataToTexture(unsigned char * pData, int TextureWidth, int TextureHeight, int numColors) {
    try {
        if (cameraTexture.IsValid()) {
            
            FUpdateTextureRegion2D region(0, 0, 0, 0, TextureWidth, TextureHeight);
            
            if ( cameraTexture->Resource) {
                
                struct FUpdateTextureRegionsData
                {
                    TWeakObjectPtr<UTexture2D> cameraTexture;
                    int32 MipIndex;
                    FUpdateTextureRegion2D Region;
                    uint32 SrcPitch;
                    uint32 SrcBpp;
                    uint8* SrcData;
                    FRWLock* frwLock;
                };
                
                FUpdateTextureRegionsData* RegionData = new FUpdateTextureRegionsData;
                
                RegionData->cameraTexture = cameraTexture;
                RegionData->MipIndex = 0;
                RegionData->Region = FUpdateTextureRegion2D(0, 0, 0, 0, TextureWidth, TextureHeight);;
                RegionData->SrcPitch = (uint32)(numColors * TextureWidth);
                RegionData->SrcBpp = (uint32)numColors;
                RegionData->SrcData = pData;
                RegionData->frwLock = &frwLock;
                
                ENQUEUE_RENDER_COMMAND(UpdateTextureRegionsData)
                 ([RegionData](FRHICommandListImmediate& RHICmdList) {
                    if(RegionData->cameraTexture.IsValid()) {
                        FTexture2DResource* Texture2DResource = (FTexture2DResource*)RegionData->cameraTexture->Resource;
                        int32 CurrentFirstMip = Texture2DResource->GetCurrentFirstMip();
                        if (RegionData->MipIndex >= CurrentFirstMip)
                        {
                            RegionData->frwLock->WriteLock();
                            
                            RHIUpdateTexture2D(Texture2DResource->GetTexture2DRHI(),
                                               RegionData->MipIndex - CurrentFirstMip,
                                               RegionData->Region,
                                               RegionData->SrcPitch,
                                               RegionData->SrcData + RegionData->Region.SrcY * RegionData->SrcPitch + RegionData->Region.SrcX * RegionData->SrcBpp);
                            RegionData->frwLock->WriteUnlock();
                        }
                    }
                    
                    delete RegionData;
                });
                
            }
            
            if ( mirrored && mirroredTexture.IsValid()) {
                
                struct FUpdateTextureRegionsData
                {
                    FTextureRenderTargetResource* MirrorTextureResource;
                    FTextureResource*	TextureResource;
                    FRWLock* frwLock;
                };
                FUpdateTextureRegionsData* RegionData = new FUpdateTextureRegionsData;
                RegionData->MirrorTextureResource = static_cast<FTextureRenderTarget2DResource*>( mirroredTexture->Resource );
                RegionData->TextureResource = static_cast<FTexture2DResource*>(cameraTexture->Resource);
                RegionData->frwLock = &frwLock;

                ENQUEUE_RENDER_COMMAND(UpdateTextureRegionsData)
                    ([RegionData](FRHICommandListImmediate& RHICmdList) {
                    mirrorTexture_RenderThread(*RegionData->frwLock, RHICmdList, RegionData->TextureResource, RegionData->MirrorTextureResource, TStaticDepthStencilState<false, CF_Always>::GetRHI());
                    delete RegionData;
                });
                
            }
        }
    } catch(...) {
        
    }
}

UTexture* BaseVideoGrabber::getTexture() {
    if ( mirrored && mirroredTexture.IsValid()) {
        return mirroredTexture.Get();
    }
    
    if (cameraTexture.IsValid()) {
        return cameraTexture.Get();
    }
    return NULL;
}

bool BaseVideoGrabber::isVideoMirrored() {
    return mirrored;
}

void BaseVideoGrabber::setVideoMirrored( bool m ) {
    this->mirrored = m;
}


bool BaseVideoGrabber::saveTextureAsFile ( const FString& fileName ) {
    bool retVal = false;
    frwLock.ReadLock();
    UTexture* texture = getTexture();
    if ( texture ) {
        
        FTexture2DRHIRef texture_ref;
        
        if (mirrored) {
            texture_ref =  static_cast<FTextureRenderTarget2DResource*>(texture->Resource)->GetTextureRHI();
        } else {
            texture_ref =  static_cast<FTexture2DResource*>(cameraTexture->Resource)->GetTexture2DRHI();
        }
        
        retVal = ImageUtility::SaveTextureAsFile(texture_ref, fileName);
    }
    frwLock.ReadUnlock();
    return retVal;
}


void BaseVideoGrabber::registerDelegates() {
    FCoreDelegates::ApplicationWillEnterBackgroundDelegate.AddRaw(this, &BaseVideoGrabber::ApplicationWillDeactivateDelegate_Handler);
    FCoreDelegates::ApplicationHasEnteredForegroundDelegate.AddRaw(this, &BaseVideoGrabber::ApplicationHasReactivatedDelegate_Handler);
#if PLATFORM_IOS
    FCoreDelegates::ApplicationWillDeactivateDelegate.AddRaw(this, &BaseVideoGrabber::ApplicationWillDeactivateDelegate_Handler);
    FCoreDelegates::ApplicationHasReactivatedDelegate.AddRaw(this, &BaseVideoGrabber::ApplicationHasReactivatedDelegate_Handler);
#endif
}

void BaseVideoGrabber::unRegisterDelegates() {
    FCoreDelegates::ApplicationWillEnterBackgroundDelegate.RemoveAll(this);
    FCoreDelegates::ApplicationHasEnteredForegroundDelegate.RemoveAll(this);
#if PLATFORM_IOS
    FCoreDelegates::ApplicationWillDeactivateDelegate.RemoveAll(this);
    FCoreDelegates::ApplicationHasReactivatedDelegate.RemoveAll(this);
#endif
}

void BaseVideoGrabber::ApplicationWillDeactivateDelegate_Handler() {
#if PLATFORM_IOS
    if ( !paused ) {
        paused = true;
#endif
        frwLock.WriteLock();
        pause();
        frwLock.WriteUnlock();
        
#if PLATFORM_IOS
    }
#endif
}

void BaseVideoGrabber::ApplicationHasReactivatedDelegate_Handler() {
#if PLATFORM_IOS
    if ( paused) {
        paused = false;
#endif
        frwLock.WriteLock();
        resume();
        frwLock.WriteUnlock();
        
#if PLATFORM_IOS
    }
#endif
}



