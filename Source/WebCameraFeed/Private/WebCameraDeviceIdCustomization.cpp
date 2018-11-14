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

        int sel = -1;
        FPropertyAccess::Result result = SelectedDeviceHandle->GetValue(sel);

		if  (options.Num() == 0) {
			options.Add(TSharedPtr<FString>(new FString(TEXT("No camera device found"))));
			SelectedDeviceHandle->SetValue(-1);
		} else {
            if (result != FPropertyAccess::Success) {
                SelectedDeviceHandle->SetValue(0);
                sel = 0;
			}
			else if (sel >= options.Num()) {
				sel = options.Num() - 1;
				if (sel < 0) sel = 0;
				SelectedDeviceHandle->SetValue(sel);
			}
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
				.InitiallySelectedItem(sel>=0 && sel < options.Num()?options[sel]: options[0])
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
                FPropertyAccess::Result result = SelectedDeviceHandle->SetValue(i);
                if (result == FPropertyAccess::Success) {
                    break;
                }
			}
			i++;
		}

		
	}
}
#endif