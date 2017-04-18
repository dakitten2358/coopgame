// Fill out your copyright notice in the Description page of Project Settings.

#include "coopgame.h"
#include "NativeCoopCharacter.h"
#include "items/NativeWeaponBase.h"

void ncc_output(const char* fmt, ...)
{
	static const auto displayDuration = 3.0f;
	static const auto forceAlwaysDisplay = -1;
	
	if (GEngine != nullptr)
	{
		char buffer[256];
		
		va_list args;
		va_start(args, fmt);
		vsprintf(buffer, fmt, args);
		va_end(args);
		
		GEngine->AddOnScreenDebugMessage(forceAlwaysDisplay, displayDuration, FColor::Yellow, buffer);
	}
}

ANativeCoopCharacter::ANativeCoopCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// we want Tick() to be called every frame
	PrimaryActorTick.bCanEverTick = true;

	// setup the defaults for movement
	SetupMovementComponentDefaults(GetCharacterMovement());

	// create and set up the camera
	CreateAndSetupCamera(ObjectInitializer);

	// set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// need to ignore the weapon channel for collisions, otherwise impacts will hit this instead of the character mesh
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;
}

void ANativeCoopCharacter::Tick(float deltaTime)
{
	Super::Tick(deltaTime);

	// if we want to be sprinting, but we're not, try to
	if (bWantsToSprint && !IsSprinting())
		SetSprinting(true);
}

// APawn IMPLEMENTATION
// -----------------------------------------------------------------------------
void ANativeCoopCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	// movement
	PlayerInputComponent->BindAxis("MoveForward", this, &ANativeCoopCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ANativeCoopCharacter::MoveRight);
	// Turn and LookUp are for devices that return an absolute delta, like a mouse
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	// TurnRate and LookUpRate are for devices that use a rate of change, like an analog joystick	
	PlayerInputComponent->BindAxis("TurnRate", this, &ANativeCoopCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ANativeCoopCharacter::LookUpAtRate);
	
	// movement actions
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ANativeCoopCharacter::OnStartCrouching);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ANativeCoopCharacter::OnStopCrouching);
	PlayerInputComponent->BindAction("CrouchToggle", IE_Released, this, &ANativeCoopCharacter::OnToggleCrouch);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ANativeCoopCharacter::OnStartSprinting);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ANativeCoopCharacter::OnStopSprinting);

	// aiming
	PlayerInputComponent->BindAction("AimingDownSights", IE_Pressed, this, &ANativeCoopCharacter::OnStartAimingDownSights);
	PlayerInputComponent->BindAction("AimingDownSights", IE_Released, this, &ANativeCoopCharacter::OnStopAimingDownSights);
}

// MOVEMENT
// -----------------------------------------------------------------------------
void ANativeCoopCharacter::MoveForward(float Val)
{
	if (Controller != nullptr && Val != 0.0f)
	{
		// we need ot limit the pitch if we're on the ground or falling
		auto character_movement = GetCharacterMovement();
		auto need_to_limit_pitch = character_movement->IsMovingOnGround() || character_movement->IsFalling();

		// figure out the forward direction
		const auto rotation = need_to_limit_pitch ? GetActorRotation() : Controller->GetControlRotation();
		const auto direction = FRotationMatrix(rotation).GetScaledAxis(EAxis::X);

		AddMovementInput(direction, Val);
	}
}

void ANativeCoopCharacter::MoveRight(float Val)
{
	if (Controller != nullptr && Val != 0.0f)
	{
		const auto rotation = GetActorRotation();
		const auto direction = FRotationMatrix(rotation).GetScaledAxis(EAxis::Y);

		AddMovementInput(direction, Val);
	}
}

void ANativeCoopCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ANativeCoopCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ANativeCoopCharacter::OnStartCrouching()
{
	if (IsSprinting())
		SetSprinting(false);

	if (CanCrouch())
		Crouch();
}

void ANativeCoopCharacter::OnStopCrouching()
{
	UnCrouch();
}

void ANativeCoopCharacter::OnToggleCrouch()
{
	if (CanCrouch())
		Crouch();
	else
		UnCrouch();
}

void ANativeCoopCharacter::OnStartSprinting()
{
	SetSprinting(true);
}

void ANativeCoopCharacter::OnStopSprinting()
{
	SetSprinting(false);
}

void ANativeCoopCharacter::SetSprinting(bool isSprinting)
{
	Super::SetSprinting(isSprinting);
}

void ANativeCoopCharacter::SetupMovementComponentDefaults(UCharacterMovementComponent* movementComponent)
{
	check(movementComponent);

	// adjust jump to make it less floaty
	movementComponent->GravityScale = 1.5f;
	movementComponent->JumpZVelocity = 620;
	movementComponent->bCanWalkOffLedgesWhenCrouching = true;
	movementComponent->MaxWalkSpeedCrouched = 200;

	// enable crouching
	movementComponent->GetNavAgentPropertiesRef().bCanCrouch = true;

	// disable jumping for now
	movementComponent->GetNavAgentPropertiesRef().bCanJump = false;
}

// AIMING
// -----------------------------------------------------------------------------
void ANativeCoopCharacter::OnStartAimingDownSights()
{
	SetAimingDownSights(true);
}

void ANativeCoopCharacter::OnStopAimingDownSights()
{
	SetAimingDownSights(false);
}

// CAMERA
// -----------------------------------------------------------------------------
void ANativeCoopCharacter::CreateAndSetupCamera(const FObjectInitializer& objectInitializer)
{
	// create and setup the spring arm for the camera, and attach it to the root component
	SpringArmForCamera = objectInitializer.CreateDefaultSubobject<USpringArmComponent>(this, TEXT("CameraBoom"));
	SpringArmForCamera->TargetArmLength = 200.0f;
	SpringArmForCamera->SocketOffset = FVector(0, 0, 0);
	SpringArmForCamera->TargetOffset = FVector(0, 0, 0);
	SpringArmForCamera->bUsePawnControlRotation = true;
	SpringArmForCamera->SetupAttachment(GetRootComponent());

	// create and attach the camera to the spring arm
	CameraComponent = objectInitializer.CreateDefaultSubobject<UCameraComponent>(this, TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArmForCamera, USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false;
}

// WEAPONS
// -----------------------------------------------------------------------------
void ANativeCoopCharacter::AddWeapon(ANativeWeaponBase* weapon)
{
	// only the server can tell us to add a weapon
	if (weapon != nullptr && Role == ROLE_Authority)
	{
		ncc_output("equipping weapon");
		weapon->OnEnterInventory(this);
	}
}
