// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "WebCameraDevice.h"
#include "VideoGrabber.h"
#include "Styling/SlateBrush.h"
#include "SWebCameraImage.h"
#include "WebCameraWidget.generated.h"


class SImage;

/**
 * 
 */
UCLASS(Config=Game)
class WEBCAMERAFEED_API UWebCameraWidget : public UWidget
{
	GENERATED_UCLASS_BODY()

public:
	~UWebCameraWidget();
	
    virtual void BeginDestroy() override;
    
    virtual void ReleaseSlateResources(bool bReleaseChildren) override;
   

	UPROPERTY(EditAnywhere, Category = WebCamera, Config)
	FWebCameraDeviceId  DeviceId;

	UPROPERTY(EditAnywhere, Category = WebCamera, Config)
	int requestedWidth;

	UPROPERTY(EditAnywhere, Category = WebCamera, Config)
	int requestedHeight;
    
    UPROPERTY(EditAnywhere, Category = WebCamera, Config)
    bool MirroredVideo;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Appearance, meta = (sRGB = "true"))
	FLinearColor ColorAndOpacity;

	/** A bindable delegate for the ColorAndOpacity. */
	UPROPERTY()
	FGetLinearColor ColorAndOpacityDelegate;

public:
	UFUNCTION(BlueprintCallable, Category="Appearance")
	void SetColorAndOpacity(FLinearColor InColorAndOpacity);
    
    UFUNCTION(BlueprintCallable, Category="WebCamera")
    bool SaveAsImage(const FString& FileName);

	UFUNCTION(Category = "WebCamera", BlueprintCallable)
	void SetDeviceId(int id);

#if WITH_EDITOR
    virtual const FText GetPaletteCategory() override;
#endif
    
protected:
	//~ Begin UWidget Interface
	virtual TSharedRef<SWidget> RebuildWidget() override;

	virtual void SynchronizeProperties() override;

	const FSlateBrush* ConvertImage(TAttribute<FSlateBrush> InImageAsset) const;

protected:
	FSlateBrush			CameraBrush;
	TSharedPtr<SWebCameraImage>	MyImage;
	TSharedPtr<VideoGrabber> currentVideoGrabber;
	
	PROPERTY_BINDING_IMPLEMENTATION(FSlateColor, ColorAndOpacity);
};
