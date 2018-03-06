#include "WebCameraDeviceIdCustomization.h"

#if WITH_EDITOR
#include "Framework/Application/SlateApplication.h"

#include "PropertyHandle.h"
#include "STextComboBox.h"
#include "DetailWidgetRow.h"

FWebCameraDeviceIdCustomization::FWebCameraDeviceIdCustomization() {
	
}

void FWebCameraDeviceIdCustomization::CustomizeHeader(TSharedRef<class IPropertyHandle> InStructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	StructPropertyHandle = InStructPropertyHandle;
	PropertyUtilities = StructCustomizationUtils.GetPropertyUtilities();

	uint32 NumChildren;
	StructPropertyHandle->GetNumChildren(NumChildren);

	for (uint32 ChildIndex = 0; ChildIndex < NumChildren; ++ChildIndex)
	{
		const TSharedRef< IPropertyHandle > ChildHandle = StructPropertyHandle->GetChildHandle(ChildIndex).ToSharedRef();

		if (ChildHandle->GetProperty()->GetName() == TEXT("selectedDevice"))
		{
			SelectedDeviceHandle = ChildHandle;
			break;
		}
	}

	if ( SelectedDeviceHandle.IsValid()) {
		devices = dummyGrabber.listDevices();
	
		options.Empty();
		for ( const FVideoDevice& videoDevice : devices  ) {
			options.Add(TSharedPtr<FString>(new FString(videoDevice.deviceName)));
		}

		if  (options.Num() == 0) {
			options.Add(TSharedPtr<FString>(new FString(TEXT("No camera device found"))));
			SelectedDeviceHandle->SetValue(-1);
		} else {
			SelectedDeviceHandle->SetValue(0);
		}

		HeaderRow.NameContent()
			[
				StructPropertyHandle->CreatePropertyNameWidget()
			]
		.ValueContent()
			.MaxDesiredWidth(512)
			[
				SNew(STextComboBox)
				.OptionsSource(&options)
				.OnSelectionChanged(this, &FWebCameraDeviceIdCustomization::OnSelectionChanged)
				.InitiallySelectedItem(options[0])
			];

		

	}
}

void FWebCameraDeviceIdCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle, class IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) {
	
}

void  FWebCameraDeviceIdCustomization::OnSelectionChanged(TSharedPtr<FString> value, ESelectInfo::Type type) {
	if ( SelectedDeviceHandle.IsValid()) {
		int i = 0;
		for ( const FVideoDevice& videoDevice : devices  ) {
			if ( videoDevice.deviceName == *value.Get()) {
				SelectedDeviceHandle->SetValue(i);
				break;
			}
			i++;
		}

		
	}
}
#endif