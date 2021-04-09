// Fill out your copyright notice in the Description page of Project Settings.


#include "Main.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"
#include "Enemy.h"


// Sets default values
AMain::AMain()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Create Camera Boom; pulls towards player if there's A COLLISION
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 600.f; //Camera follows at this distance
	CameraBoom->bUsePawnControlRotation = true; //Rotate arm based on controller

	//Set size for collision capsule
	GetCapsuleComponent()->SetCapsuleSize(48.f, 105.f);

	//Create Follow Camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	//Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false;

	//Hard coding turn rates for input
	BaseTurnRate = 65.f;
	BaseLookUpRate = 65.f;

	// don't rotate the character when the controller rotates
	// let that just affect the camera
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	//configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; //character moves in the direction of input...
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.f, 0.0f); //...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 650.f;
	GetCharacterMovement()->AirControl = 0.2f;

	MaxHealth = 100.f;
	Health = 65.f;
	MaxStamina = 150.f;
	Stamina = 120.f;
	Coins = 0;

	RunningSpeed = 650.f;
	SprintingSpeed = 950.f;

	bShiftKeyDown = false;
	bLMBDown = false;

	//Initialize enums
	MovementStatus = EMovementStatus::EMS_Normal;
	StaminaStatus = EStaminaStatus::ESS_Normal;

	StaminaDrainRate = 25.f;
	MinSprintStamina = 50.f;

	InterpSpeed = 15.f;
	bInterpToEnemy = false;
}

// Called when the game starts or when spawned
void AMain::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AMain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float DeltaStamina = StaminaDrainRate * DeltaTime; //create float variable called DeltaStamina that is equal to stamina drain rate times difference in time

	switch (StaminaStatus) //switch statement for all the cases of stamina statuses
	{
	case EStaminaStatus::ESS_Normal: //stamina status that we can sprint in
		if (bShiftKeyDown) //inside normal stamina status case, and shift key is down, do the following
		{
			if (Stamina - DeltaStamina <= MinSprintStamina) //if stamina - difference in stamina is less than or equal to minimum sprintable stamina,
			{
				SetStaminaStatus(EStaminaStatus::ESS_BelowMinimum); //then set stamina status to below the minimum sprintable state
				Stamina -= DeltaStamina; //and continue to drain stamina
				UE_LOG(LogTemp, Warning, TEXT("Stamina status below min"));
			}
			else //else stamina - difference in stamina is greater than minimum sprintable stanima
			{
				Stamina -= DeltaStamina; //or drain stamina
			}
			SetMovementStatus(EMovementStatus::EMS_Sprinting); // if we are in normal stanima state and shift key is down, change movement status to sprinting
			UE_LOG(LogTemp, Warning, TEXT("Movement status sprinting"));
		}
		else //else we are in normal stamina status and shift key is up, do the following
		{
			if (Stamina + DeltaStamina >= MaxStamina) //if stamina plus regenerated is greater than or equal to maxstamina,
			{
				Stamina = MaxStamina; //then set stamina to max stamina
			}
			else //else stamina plus regenerated stamina is less than max
			{
				Stamina += DeltaStamina; //regenerate stamina
			}
			SetMovementStatus(EMovementStatus::EMS_Normal); // we are in normal stamina state and shift key is up so set movement status to normal
			UE_LOG(LogTemp, Warning, TEXT("Movement status normal"));
		}
		break;
	case EStaminaStatus::ESS_BelowMinimum: //case stamina status where we are at 0 stamina and/or below minimum sprintable stamina
		if (bShiftKeyDown) //shift key down
		{
			if (Stamina - DeltaStamina <= 0.f) //If stamina minus drained stamina is less than or equal to 0,
			{
				SetStaminaStatus(EStaminaStatus::ESS_Exhausted); //set stamina status to exhausted
				Stamina = 0.f; //and set stamina to 0
				SetMovementStatus(EMovementStatus::EMS_Normal); //and set movement status to normal
				UE_LOG(LogTemp, Warning, TEXT("stamina status exhausted"));
				UE_LOG(LogTemp, Warning, TEXT("movement status normal"));
			}
			else //else stamina minus drained stamina is greater than 0
			{
				Stamina -= DeltaStamina; //then continue to drain stamina
				SetMovementStatus(EMovementStatus::EMS_Sprinting); //and set movement status to sprinting
				UE_LOG(LogTemp, Warning, TEXT("movement status sprinting"));
			}
		}
		else  //shift key up
		{
			if (Stamina + DeltaStamina >= MinSprintStamina) //check if stamina + regenerated stamina became greater than or equal to minimum sprintable stamina (case normal)
			{
				SetStaminaStatus(EStaminaStatus::ESS_Normal); //then set stamina status to normal state
				Stamina += DeltaStamina; //and regenerate stamina
				UE_LOG(LogTemp, Warning, TEXT("stamina status normal"));
			}
			else //else stamina + regenrated stamina is still less than minimum sprintable stamina (case normal)
			{
				Stamina += DeltaStamina; //regenerate stamina
			}
			SetMovementStatus(EMovementStatus::EMS_Normal); //set movement status to normal because shift key is up
			UE_LOG(LogTemp, Warning, TEXT("movement status normal"));
		}
		break;
	case EStaminaStatus::ESS_Exhausted: //case stamina status where stamina has reached 0 and we are unable to sprint
		if (bShiftKeyDown) //check if shift key is down
		{
			Stamina = 0.f; //stamina is 0 and cannot regenerate
		}
		else //shift key is up
		{
			SetStaminaStatus(EStaminaStatus::ESS_ExhaustedRecovering); //set stamina status to recovering while still below minimum sprintable (case ExhaustedRecovering)
			Stamina += DeltaStamina; //regenerate stamina
			UE_LOG(LogTemp, Warning, TEXT("stamina status ExhaustedRecovering"));
		}
		SetMovementStatus(EMovementStatus::EMS_Normal); //whether shift key is up or down movement status gets set to normal
		break;
	case EStaminaStatus::ESS_ExhaustedRecovering: //case stamina status after reaching 0 and needs to regenerate to minimum sprintable stamina status (stamina status case normal)
		if (Stamina + DeltaStamina >= MinSprintStamina) //check if stamina plus regenerated stamina is greater than or equal to minimum sprintable stamina
		{
			SetStaminaStatus(EStaminaStatus::ESS_Normal); //then set stamina status to normal
			Stamina += DeltaStamina; //and continue to regenerate stamina
			UE_LOG(LogTemp, Warning, TEXT("stamina status normal"));
		}
		else //else stamina plus regenerated stamina is still less than minimum sprintable
		{
			Stamina += DeltaStamina; //continue to regenerate
		}
		SetMovementStatus(EMovementStatus::EMS_Normal); //whether shift key is up or down movement status remnains in normal
		UE_LOG(LogTemp, Warning, TEXT("movement status normal"));
		break;
	default:
		;
	}

	if (bInterpToEnemy && CombatTarget)
	{
		FRotator LookAtYaw = GetLookAtRotationYaw(CombatTarget->GetActorLocation());
		//Use RInterpTo so that Main doesn't just snap immediately, will be a more smooth turn
		FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), LookAtYaw, DeltaTime, InterpSpeed);

		SetActorRotation(InterpRotation);
	}

}

FRotator AMain::GetLookAtRotationYaw(FVector Target)
{
	//Create variable of type FRotator, which is the function of UkismetMathLibrary with input paramaters of Main's ActorLocation and Target's location
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target);
	//Isolate yaw from LookAtRotation
	FRotator LookAtRotationYaw(0.f, LookAtRotation.Yaw, 0.f);
	return LookAtRotationYaw;
}

// Called to bind functionality to input
void AMain::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent); //check is playerinputcomponent is valid

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMain::ShiftKeyDown);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMain::ShiftKeyUp);

	PlayerInputComponent->BindAction("LMB", IE_Pressed, this, &AMain::LMBDown);
	PlayerInputComponent->BindAction("LMB", IE_Released, this, &AMain::LMBUp);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMain::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMain::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMain::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMain::LookUpAtRate);


}

void AMain:: MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f) && (!bAttacking)) //check for null pointer and need to make sure key is pressed so that we want to move forward (value != 0.0f). can't be attacking
	{
		//find out which way is forward 
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		//create rotation matrix from YawRotation and this has Unit Axes that we can access and one of these is X axis
		// in other words we are accessing the local axis rotation and want to move forward based on that (usually X is forward)
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X); 
		AddMovementInput(Direction, Value);
	}
}

void AMain::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f) && (!bAttacking)) //check for null pointer and need to make sure key is pressed so that we want to move forward (value != 0.0f). can't be attacking
	{
		//find out which way is forward 
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		//doing same as above but just for Y axis instead of X	
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}

void AMain::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds()); //need to include Engine/World/h to access GetWorld->GetDeltaSeconds
}

void AMain::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

//Use this function to check that the player wants to equip the new weapon
void AMain::LMBDown()
{
	bLMBDown = true;

	if (ActiveOverlappingItem)
	{
		AWeapon* Weapon = Cast<AWeapon>(ActiveOverlappingItem); //Cast in to AWeapon, pass in OverLappingItem
		if (Weapon) //check weapon is valid
		{
			Weapon->Equip(this); //Call Equip inside Weapon
			SetActiveOverlappingItem(nullptr);
		}
	}
	else if (EquippedWeapon) //if LMB is down and we have a weapon equipped, it means we want to attack so call Attack function
	{
		Attack();
	}
}


void AMain::LMBUp()
{
	bLMBDown = false;
}


void AMain::DecrementHealth(float Amount)
{
	if (Health - Amount <= 0.f)
	{
		Health -= Amount;
		Die();
	}
	else
	{
		Health -= Amount;
	}
}

void AMain::IncrementCoins(int32 Amount)
{
	Coins += Amount;
}

void AMain::Die()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && CombatMontage) //is valid
	{
		AnimInstance->Montage_Play(CombatMontage, 1.0f);
		AnimInstance->Montage_JumpToSection(FName("Death"));
	}
}

void AMain::SetMovementStatus(EMovementStatus Status)
{
	MovementStatus = Status;
	if (MovementStatus == EMovementStatus::EMS_Sprinting)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintingSpeed;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = RunningSpeed;
	}
}

void AMain::ShiftKeyDown()
{
	bShiftKeyDown = true;
}


void AMain::ShiftKeyUp()
{
	bShiftKeyDown = false;

}

void AMain::ShowPickupLocations()
{

	for (FVector Location : PickupLocations)
	{
		UKismetSystemLibrary::DrawDebugSphere(this, Location, 25.f, 8, FLinearColor::Red, 10.f, 0.5f);
	}
	
}

void AMain::SetEquippedWeapon(AWeapon* WeaponToSet)
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Destroy();
	}

	EquippedWeapon = WeaponToSet;
}

void AMain::Attack()
{
	if (!bAttacking) //ensure we are not in already Attacking state before next attack
	{
	
		bAttacking = true;
		SetInterpToEnemy(true);

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && CombatMontage) //is valid
		{
			int32 Section = FMath::RandRange(0, 1); //Randomize the attack animation played from Combat Montage using int32 variable called Section and switch statement
			switch (Section)
			{
			case 0:
				//Play the Anim Instance's montage Combat Montage at 2.2x speed, and skip directly to Attack_1
				AnimInstance->Montage_Play(CombatMontage, 2.2f);
				AnimInstance->Montage_JumpToSection(FName("Attack_1"), CombatMontage);
				break;
			case 1:
				AnimInstance->Montage_Play(CombatMontage, 1.8f);
				AnimInstance->Montage_JumpToSection(FName("Attack_2"), CombatMontage);
				break;

			default:
				;
			}
		}
	}
}

void AMain::AttackEnd()
{
	bAttacking = false;
	SetInterpToEnemy(false);
	if (bLMBDown)
	{
		Attack();
	}
}

void AMain::PlaySwingSound()
{
	if (EquippedWeapon->SwingSound)
	{
		UGameplayStatics::PlaySound2D(this, EquippedWeapon->SwingSound);
	}
	
}

void AMain::SetInterpToEnemy(bool Interp)
{
	bInterpToEnemy = Interp;
}

float AMain::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	DecrementHealth(DamageAmount);

	return DamageAmount;
}
