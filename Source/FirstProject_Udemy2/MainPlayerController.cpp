// Fill out your copyright notice in the Description page of Project Settings.


#include "MainPlayerController.h"
#include "Blueprint/UserWidget.h"

void AMainPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (HUDOverlayAsset)
	{
		HUDOverlay = CreateWidget<UUserWidget>(this, HUDOverlayAsset);
	}
	HUDOverlay->AddToViewport();
	HUDOverlay->SetVisibility(ESlateVisibility::Visible);

	if (WEnemyHealthBar)
	{
		EnemyHealthBar = CreateWidget<UUserWidget>(this, WEnemyHealthBar);
		if (EnemyHealthBar)
		{
			EnemyHealthBar->AddToViewport();
			EnemyHealthBar->SetVisibility(ESlateVisibility::Hidden);
		}
		//Align in viewport
		FVector2D Alignment(0.f, 0.f);
		EnemyHealthBar->SetAlignmentInViewport(Alignment);
	}

	if (WPauseMenu)
	{
		PauseMenu = CreateWidget<UUserWidget>(this, WPauseMenu);
		if (PauseMenu)
		{
			PauseMenu->AddToViewport();
			PauseMenu->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void AMainPlayerController::DisplayEnemyHealthBar()
{
	if (EnemyHealthBar)
	{
		bEnemyHealthBarVisible = true;
		//Set health bar to visible
		EnemyHealthBar->SetVisibility(ESlateVisibility::Visible);
	}
}

void AMainPlayerController::RemoveEnemyHealthBar()
{
	if (EnemyHealthBar)
	{
		bEnemyHealthBarVisible = false;
		//Set health bar to hidden
		EnemyHealthBar->SetVisibility(ESlateVisibility::Hidden);
	}
}

void AMainPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (EnemyHealthBar)
	{
		//Get world location of enemy and project to viewport
		FVector2D PositionInViewport;
		ProjectWorldLocationToScreen(EnemyLocation, PositionInViewport);
		//Decrease Y to raise health bar above enemy
		PositionInViewport.Y -= 85.f;

		//hard code the size of health bar
		FVector2D SizeInViewport(200.f, 25.f);

		//Project EnemyHealthBar to position determined in PositionInViewport
		EnemyHealthBar->SetPositionInViewport(PositionInViewport);
		EnemyHealthBar->SetDesiredSizeInViewport(SizeInViewport);
	}
	/*
DisplayEnemyHealthBar on MainPlayerController
0
Mushfiqur � Lecture 78
� 2 years ago

Hi,

Any reason we put DisplayEnemyHealthBar on MainPlayerController instead of say Enemy.cpp? Are there any guidelines/documentation/Unreal standards on where to place what?

Stephen Ulibarri
Stephen � Instructor
2 years ago
Hi Mushfiqur,

It is just a choice made for this particular game. Since there is only one Health Bar being displayed at a time, it's fine to put it on the MainPlayerController. You could put it in any class you want though.

I have mainly seen HUD code in the PlayerController class, but I've also seen it in Character classes or HUD classes.

Stephen
*/
}

void AMainPlayerController::DisplayPauseMenu_Implementation()
{
	if (PauseMenu)
	{
		bPauseMenuVisible = true;
		//Set pause menu to visible
		PauseMenu->SetVisibility(ESlateVisibility::Visible);

		//Create struct of type FInputModeUIOnly called the same, and pass in to SetInputMode()
		FInputModeGameAndUI InputModeGameAndUI; 
		SetInputMode(InputModeGameAndUI);
		bShowMouseCursor = true;
	}
}

void AMainPlayerController::RemovePauseMenu_Implementation()
{
	if (PauseMenu)
	{
		//Create struct of type FInputModeGameOnly called the same, and pass in to SetInputMode()
		FInputModeGameOnly InputModeGameOnly;
		SetInputMode(InputModeGameOnly);
		bShowMouseCursor = false;

		bPauseMenuVisible = false;
	}
}

void AMainPlayerController::TogglePauseMenu()
{
	if (bPauseMenuVisible)
	{
		RemovePauseMenu(); //remove the _Implementation so that the BP function can be called (for animation in this case)
	}
	else
	{
		DisplayPauseMenu(); //remove the _Implementation so that the BP function can be called (for animation in this case)
	}
}

