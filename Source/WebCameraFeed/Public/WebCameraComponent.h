// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "WebCameraDevice.h"
#include <Engine/Texture2D.h>
#include "VideoGrabber.h"
#include "WebCameraComponent.generated.h"


UCLASS( ClassGroup=(WebCamera), meta=(BlueprintSpawnableComponent), Config=Game )
class WEBCAMERAFEED_API UWebCameraComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWebCameraComponent();

	UPROPERTY(EditAnywhere, Category = WebCamera, Config)
	FWebCameraDeviceId  DeviceId;

	UPROPERTY(EditAnywhere, Category = WebCamera, Config)
	int requestedWidth;

	UPROPERTY(EditAnywhere, Category = WebCamera, Config)
	int requestedHeight;

	UFUNCTION(Category = WebCamera, BlueprintCallable)
	UTexture* GetTexture();
    
    UPROPERTY(EditAnywhere, Category = WebCamera, Config)
    bool MirroredVideo;
	
    UFUNCTION(BlueprintCallable, Category="WebCamera")
    bool SaveAsImage(const FString& FileName);


	UFUNCTION(Category = "WebCamera", BlueprintCallable)
	static TArray<FString> ListDevices();

	UFUNCTION(Category = "WebCamera", BlueprintCallable)
	void SetDeviceId(int id);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	TSharedPtr<VideoGrabber> currentVideoGrabber;
	
};
