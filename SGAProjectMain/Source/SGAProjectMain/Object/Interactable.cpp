// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactable.h"

#include "Components/WidgetComponent.h"
#include "../UI/InteractionWidget.h"

// Sets default values
AInteractable::AInteractable()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	_mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	_mesh->SetSimulatePhysics(true);
	RootComponent = _mesh;

	_interactionMark = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractionMark"));
	_interactionMark->SetupAttachment(RootComponent);
	_interactionMark->SetWidgetSpace(EWidgetSpace::Screen);
	_interactionMark->SetRelativeLocation(FVector::ZeroVector);

	static ConstructorHelpers::FClassFinder<UUserWidget> widgetBP(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/Blueprints/UI/BP_InteractionWidget.BP_InteractionWidget_C'"));
	if (widgetBP.Succeeded())
	{
		_interactionMark->SetWidgetClass(widgetBP.Class);
	}
}

// Called when the game starts or when spawned
void AInteractable::BeginPlay()
{
	Super::BeginPlay();
	
	_interactionMark->SetVisibility(false);
}

// Called every frame
void AInteractable::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AInteractable::ShowDefaultMark()
{
	if (auto widget = Cast<UInteractionWidget>(_interactionMark->GetUserWidgetObject()))
	{
		widget->ShowDefaultMark();
	}
	_interactionMark->SetVisibility(true);
}

void AInteractable::ShowKeyButtonMark()
{
	if (auto widget = Cast<UInteractionWidget>(_interactionMark->GetUserWidgetObject()))
	{
		widget->ShowKeyButtonMark();
	}
	_interactionMark->SetVisibility(true);
}

void AInteractable::HideMark()
{
	_interactionMark->SetVisibility(false);
}

