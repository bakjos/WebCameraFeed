/******************************************************************************
* The MIT License (MIT)
*
* Copyright (c) 2015 Fredrik Lindh
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
******************************************************************************/

#pragma once

#include "GlobalShader.h"
#include "UniformBuffer.h"
#include "TextureResource.h"
#include "RHICommandList.h"
#include "RHIStaticStates.h"
#include "ShaderParameterUtils.h"
#include <Runtime/Launch/Resources/Version.h>



/************************************************************************/
/* This is the type we use as vertices for our fullscreen quad.         */
/************************************************************************/
struct FTextureVertex
{
	FVector4 Position;
	FVector2D UV;
};

/************************************************************************/
/* We define our vertex declaration to let us get our UV coords into    */
/* the shader                                                           */
/************************************************************************/
class FTextureVertexDeclaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;

	virtual void InitRHI() override
	{
		FVertexDeclarationElementList Elements;
		uint32 Stride = sizeof(FTextureVertex);
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FTextureVertex, Position), VET_Float4, 0, Stride));
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FTextureVertex, UV), VET_Float2, 1, Stride));
		VertexDeclarationRHI = RHICreateVertexDeclaration(Elements);
	}

	virtual void ReleaseRHI() override
	{
		VertexDeclarationRHI.SafeRelease();
	}
};

/************************************************************************/
/* A simple passthrough vertexshader that we will use.                  */
/************************************************************************/
class FWebCameraMirrorVS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FWebCameraMirrorVS, Global);

public:

	static bool ShouldCache(EShaderPlatform Platform) { return true; }
#if (ENGINE_MAJOR_VERSION > 4 || (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION > 18))  
    /** Should the shader be cached? Always. */
    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        //return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM4) && !IsConsolePlatform(Parameters.Platform);
        return true;
    }
#endif

	FWebCameraMirrorVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer) :
		FGlobalShader(Initializer)
	{}
	FWebCameraMirrorVS() {}
};


/***************************************************************************/
/* This class is what encapsulates the shader in the engine.               */
/* It is the main bridge between the HLSL located in the engine directory  */
/* and the engine itself.                                                  */
/***************************************************************************/
class FWebCameraMirrorShader : public FGlobalShader
{
	
	DECLARE_INLINE_TYPE_LAYOUT(FWebCameraMirrorShader, NonVirtual);

public:

	FWebCameraMirrorShader() {}

	FWebCameraMirrorShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer);

	static bool ShouldCache(EShaderPlatform Platform) { return IsFeatureLevelSupported(Platform, ERHIFeatureLevel::SM5); }
 
#if (ENGINE_MAJOR_VERSION > 4 || (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION > 18))   
    /** Should the shader be cached? Always. */
    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        //return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM4) && !IsConsolePlatform(Parameters.Platform);
        return true;
    }
#endif

	
	template<typename TShaderRHIParamRef>
	void SetParameters(FRHICommandList& RHICmdList, const TShaderRHIParamRef ShaderRHI, FTextureResource* UITextureRHI, bool mirror) {
		FRHISamplerState* samplerState = UITextureRHI->SamplerStateRHI;
		if (!samplerState) {
			samplerState = TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
		}


		SetShaderValue(RHICmdList, ShaderRHI, mirrorParameter, (int)mirror);
		SetTextureParameter(RHICmdList, ShaderRHI, TextureParameter, TextureParameterSampler, samplerState, UITextureRHI->TextureRHI);
	}

private:
	//This is how you declare resources that are going to be made available in the HLSL
	LAYOUT_FIELD(FShaderResourceParameter,TextureParameter);
	LAYOUT_FIELD(FShaderResourceParameter, TextureParameterSampler);
	LAYOUT_FIELD(FShaderParameter,mirrorParameter);
};

class FWebCameraMirrorPS : public FWebCameraMirrorShader {
	DECLARE_SHADER_TYPE(FWebCameraMirrorPS, Global);
public:
	/** Default constructor. */
	FWebCameraMirrorPS() {}

	/** Initialization constructor. */
	FWebCameraMirrorPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FWebCameraMirrorShader(Initializer)
	{ }
};
