// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "WebCameraDevice.h"
#include "WebCameraComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WEBCAMERAFEED_API UWebCameraComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWebCameraComponent();

	UPROPERTY(EditAnywhere, Category = WebCamera)
	FWebCameraDeviceId  DeviceId;

	UPROPERTY(EditAnywhere, Category = WebCamera)
	int requestedWidth;

	UPROPERTY(EditAnywhere, Category = WebCamera)
	int requestedHeight;


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
	
};
