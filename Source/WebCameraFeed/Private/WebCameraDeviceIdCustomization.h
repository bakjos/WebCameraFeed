#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWidget.h"
#include "Editor/PropertyEditor/Public/IPropertyTypeCustomization.h"
#include "SGraphPin.h"
#include "WebCameraDevice.h"
#include "VideoGrabber.h"


class IPropertyHandle;



class FWebCameraDeviceIdCustomization:  public IPropertyTypeCustomization {
public:

	FWebCameraDeviceIdCustomization();

	static TSharedRef<IPropertyTypeCustomization> MakeInstance()
	{
		return MakeShareable(new FWebCameraDeviceIdCustomization);
	}

	virtual void CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle, class IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

protected:

	void OnSelectionChanged(TSharedPtr<FString> value, ESelectInfo::Type type);

	/** Handle to the struct property being customized */
	TSharedPtr<IPropertyHandle> StructPropertyHandle;
	TSharedPtr<IPropertyHandle> SelectedDeviceHandle;
	TSharedPtr<IPropertyUtilities> PropertyUtilities;

	VideoGrabber  dummyGrabber;
	TArray<FVideoDevice> devices;
	TArray< TSharedPtr<FString>> options;
};