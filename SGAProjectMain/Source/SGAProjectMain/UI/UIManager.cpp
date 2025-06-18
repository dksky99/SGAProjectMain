// Fill out your copyright notice in the Description page of Project Settings.


#include "UIManager.h"

UUIManager::UUIManager()
{
	AddUIConstructor("GunSetting", TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/Blueprints/UI/BP_GunSettingWidget.BP_GunSettingWidget_C'"));
}

void UUIManager::Initialize(FSubsystemCollectionBase& collection)
{
	Super::Initialize(collection);

	UE_LOG(LogTemp, Warning, TEXT("Widget Classes %d"), _widgetMap.Num());
}

bool UUIManager::IsOpen(FString name)
{
	TSubclassOf<UUserWidget> widgetClass = GetUIClass(name);
	if (!widgetClass)
		return false;

	for (auto& widget : _widgetStack)
	{
		if (widget->GetClass() == widgetClass)
			return true;
	}

	return false;
}

UUserWidget* UUIManager::GetOrShowPopUp(FString name)
{
	TSubclassOf<UUserWidget> widgetClass = GetUIClass(name);
	
	if (!widgetClass)
		return nullptr;
	
	// 현재 켜져있는 UI인지 확인
	for (auto& widget : _widgetStack)
	{
		if (widget->GetClass() == widgetClass)
			return widget;
	}

	// 켜져있지 않다면 켜기
	UUserWidget* widget = CreateWidget(GetWorld(), widgetClass);
	
	if (widget)
	{
		widget->AddToViewport(_zOrder);
		_zOrder++;
		_widgetStack.Push(widget);
	}

	return widget;
}

void UUIManager::ClosePopUp(FString name)
{
	TSubclassOf<UUserWidget> widgetClass = GetUIClass(name);
	if (!widgetClass)
		return;

	UUserWidget* target = nullptr;
	for (auto& widget : _widgetStack)
	{
		if (widget->GetClass() == widgetClass)
		{
			widget->RemoveFromParent();
			target = widget;
			break;
		}
	}

	if (target)
	{
		_widgetStack.Remove(target);
	}
}

void UUIManager::ClosePopUp()
{
	if (_widgetStack.IsEmpty()) return;

	_widgetStack.Last()->RemoveFromParent();
	_widgetStack.Pop();
	_zOrder--;
}

void UUIManager::CloseAll()
{
	for (auto& widget : _widgetStack)
	{
		widget->RemoveFromParent();
	}
	
	_widgetStack.Empty();
	_zOrder = 1;
}

UUserWidget* UUIManager::GetOrShowSceneUI(TSubclassOf<UUserWidget> widgetClass)
{
	if (_sceneUI && _sceneUI->GetClass() == widgetClass)
	{
		return _sceneUI;
	}

	if (!_sceneUI)
	{
		CloseSceneUI();
	}

	_sceneUI = CreateWidget(GetWorld(), widgetClass);
	_sceneUIClass = widgetClass;

	_sceneUI->AddToViewport();

	return _sceneUI;
}

void UUIManager::CloseSceneUI()
{
	_sceneUIClass = nullptr;

	_sceneUI->RemoveFromParent();
	_sceneUI = nullptr;
}

void UUIManager::AddUIConstructor(FString name, FString path)
{
	ConstructorHelpers::FClassFinder<UUserWidget> bp(*path);

	if (bp.Succeeded())
	{
		_widgetMap.Add(name, bp.Class);
	}
}

TSubclassOf<UUserWidget> UUIManager::GetUIClass(FString name)
{
	if (_widgetMap.Find(name) == nullptr)
		return nullptr;

	return _widgetMap[name];
}
