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
#include "PixelShaderDeclaration.h"
#include "RHIStaticStates.h"




FWebCameraMirrorShader::FWebCameraMirrorShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
: FGlobalShader(Initializer)
{
	
	TextureParameter.Bind(Initializer.ParameterMap, TEXT("TextureParameter"));  //The text parameter here is the name of the parameter in the shader
    TextureParameterSampler.Bind(Initializer.ParameterMap, TEXT("TextureParameterSampler"));
    mirrorParameter.Bind(Initializer.ParameterMap, TEXT("Mirror"));
}


//This is what will instantiate the shader into the engine from the engine/Shaders folder
//                      ShaderType               ShaderFileName     Shader function name            Type
IMPLEMENT_SHADER_TYPE(, FWebCameraMirrorVS, TEXT("/Plugin/WebCameraFeed/Private/WebCameraMirrorShader.usf"), TEXT("MainVertexShader"), SF_Vertex);
IMPLEMENT_SHADER_TYPE(, FWebCameraMirrorPS, TEXT("/Plugin/WebCameraFeed/Private/WebCameraMirrorShader.usf"), TEXT("MainPixelShader"), SF_Pixel);


