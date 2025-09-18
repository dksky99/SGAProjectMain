// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryWheelWidget.generated.h"

/**
 * 
 */
struct FWheelEntry
{
	bool _isAvailable = false;
	FString _label = TEXT("Cancel");
	class UImage* _section;
	class UImage* _icon;
};

UCLASS()
class SGAPROJECTMAIN_API UInventoryWheelWidget : public UUserWidget
{
	GENERATED_BODY()

	void NativeConstruct() override;
	void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	void InitializeWheel(class UHellDiverInvenComponent* invenComp);

	int32 GetCurIndex() { return _curIndex; }

protected:
	void UpdateWheel();
	void ResetWheel();

	void HighlightSection(int32 index);


	UPROPERTY(meta = (BindWidget))
	class UCanvasPanel* _canvasPanel;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _indexText;

	UPROPERTY(meta = (BindWidget))
	class UImage* _cursorImage;

	UPROPERTY(meta = (BindWidget))
	class UImage* _gunSection;
	UPROPERTY(meta = (BindWidget))
	class UImage* _itemSection;
	UPROPERTY(meta = (BindWidget))
	class UImage* _backpackSection;
	UPROPERTY(meta = (BindWidget))
	class UImage* _sampleSection;

	UPROPERTY(meta = (BindWidget))
	class UImage* _cancelCircle;

	UPROPERTY(meta = (BindWidget))
	class UImage* _gunIcon;
	UPROPERTY(meta = (BindWidget))
	class UImage* _itemIcon;
	UPROPERTY(meta = (BindWidget))
	class UImage* _backpackIcon;
	UPROPERTY(meta = (BindWidget))
	class UImage* _sampleIcon;

	FVector2D _center; // 중앙 위치
	FVector2D _cursorPos; // 커서 위치
	float _centerRadius = 115.f; // 중앙 취소 영역 반지름
	float _maxRadius = 250.f; // 최대 반지름

	TArray<FWheelEntry> _wheelEntries;
	TArray<class UImage*> _sections;
	TArray<class UImage*> _icons;

	FLinearColor _defaultSectionColor = FLinearColor(0.03f, 0.03f, 0.03f, 0.5f);
	FLinearColor _highlightSectionColor = FLinearColor(1.f, 1.f, 1.f, 0.5f);
	FLinearColor _defaultIconColor = FLinearColor(1.f, 1.f, 1.f, 0.5f);
	FLinearColor _highlightIconColor = FLinearColor(1.f, 1.f, 1.f, 1.f);
	FLinearColor _disabledIconColor = FLinearColor(1.f, 1.f, 1.f, 0.2f);

	UPROPERTY()
	APlayerController* _playerController;

	int32 _curIndex = -1;

};
