// Fill out your copyright notice in the Description page of Project Settings.


#include "SampleWidget.h"

#include "Components/TextBlock.h"

void USampleWidget::SetSampleCount(const FSampleBundle& sampleBundle)
{
    _commonSampleText->SetText(FText::AsNumber(0));
    _rareSampleText->SetText(FText::AsNumber(0));
    _superSampleText->SetText(FText::AsNumber(0));

    for (const auto& sample : sampleBundle._samples)
    {

        FString text = FString::Printf(TEXT("%d"), sample.Value);

        if (sample.Key == ESampleType::Common)
            _commonSampleText->SetText(FText::FromString(text));
        else if (sample.Key == ESampleType::Rare)
            _rareSampleText->SetText(FText::FromString(text));
        else if (sample.Key == ESampleType::Super)
            _superSampleText->SetText(FText::FromString(text));
        else
            continue;
    }
}
