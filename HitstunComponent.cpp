// Fill out your copyright notice in the Description page of Project Settings.

#include "HitstunComponent.h"
#include "BasePlayer.h"
#include "Runtime/Engine/Classes/Components/SkeletalMeshComponent.h"
#include "Runtime/Engine/Classes/GameFramework/CharacterMovementComponent.h"


// Sets default values for this component's properties
UHitstunComponent::UHitstunComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	OwningPlayer = nullptr;
	CurrentHitstunFrame = 0.f;
	MaxHitstunFrames = 0.f;
	bIsInHitstun = false;
	bIsInvulnerable = false;
	MaxInvulnerableFrames = 0.f;
	CurrentInvulnerableFrames = 0.f;
	bIsFrozen = false;


	FullStunMeterStunFrames = 30;
	MaxStunMeter = 100.f;
	StunMeter = 0.f;
	CurrentStunMeterRemainingFrames = 0.f;
	StunMeterDecreasePerFrame = 5.f;
	StunMeterDecreaseDelayInFrames = 20;

	CurrentShakeFrame = 0.f;
	ShakeFrameCount = -1.f;

}

// Called when the game starts
void UHitstunComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningPlayer = dynamic_cast<ABasePlayer*>(GetOwner());
	check(OwningPlayer);
}


// Called every frame
void UHitstunComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateFreeze(DeltaTime);

	UpdateHitstun(DeltaTime);

	UpdateInvulnerability(DeltaTime);

	UpdatePlayerShake(DeltaTime);

	UpdateStunMeter(DeltaTime);
}

void UHitstunComponent::Stun(float HitstunFrames, float FreezeFrames, float StunMeterPerAttack, bool bShouldChangeStateToStunned, bool bFreezeCharacter)
{
	MaxHitstunFrames = HitstunFrames;
	CurrentHitstunFrame = 0.f;
	bIsInHitstun = true;
	if (bShouldChangeStateToStunned)
	{
		MaxHitstunFrames += FreezeFrames;
		if (OwningPlayer->IsInAir() || GetOwningPlayer()->EnemyHit.bIsLauncher)
		{
			OwningPlayer->SetState(EPlayerStates::AirKnockback);
		}
		else
		{
			OwningPlayer->SetState(EPlayerStates::Stunned);
		}
	}

	CurrentStunMeterFrame = 0.f;
	StunMeter += StunMeterPerAttack;
	

	if (bFreezeCharacter == true)
	{
		MaxFreezeFrames = FreezeFrames;
		CurrentFreezeFrame = 0.f;
		HitStunFreeze(MaxHitstunFrames - 1.f);
	}

	if (FreezeFrames > 0.f || HitstunFrames > 0.f)
	{
		OwningPlayer->SetCanAttack(false);
	}
}

void UHitstunComponent::StunWithInvulnerability(int HitstunFrames, int FreezeFrames, int InvulnerabilityFrames, float StunMeterPerAttack, bool bShouldChangeStateToStunned, bool bFreezeCharacter)
{
	CurrentInvulnerableFrames = 0.f;
	MaxInvulnerableFrames = InvulnerabilityFrames;
	bIsInvulnerable = true;

	Stun(HitstunFrames, FreezeFrames, StunMeter, bShouldChangeStateToStunned);
}

void UHitstunComponent::UpdateInvulnerability(float DeltaTime)
{
	if (bIsInvulnerable)
	{
		//Don't start the invulnerable frames if we're still in the air
		if (!OwningPlayer->IsInAir())
		{
			CurrentInvulnerableFrames += (DeltaTime * 60.f);
			if (CurrentInvulnerableFrames >= MaxInvulnerableFrames)
			{
				EndInvulnerability();
			}
		}
	}
}

void UHitstunComponent::EndInvulnerability()
{
	bIsInHitstun = false;
	bIsInvulnerable = false;
}

void UHitstunComponent::FreezeCharacter()
{
	bIsFrozen = true;
	OwningPlayer->GetMesh()->bPauseAnims = true;
	OwningPlayer->SetCanAttack(false);
}

void UHitstunComponent::UnfreezeCharacter()
{
	bIsFrozen = false;
	OwningPlayer->GetMesh()->bPauseAnims = false;
}

void UHitstunComponent::EndStun()
{
	bIsInHitstun = false;
}

void UHitstunComponent::HitStunFreeze(int FreezeFrames)
{
	CurrentFreezeFrame = 0.f;
	FreezeCharacter();
}

void UHitstunComponent::ShakePlayer(float Direction, float Offset, float FrameCount)
{
	ShakeDirection = Direction;
	ShakeOffset = Offset;
	ShakeFrameCount = FrameCount;
	MeshPlayerOffset = OwningPlayer->GetMesh()->RelativeLocation;

	if (CurrentShakeFrame <= 0.f)
	{
		MeshOriginalLocation = OwningPlayer->GetMesh()->GetComponentToWorld().GetTranslation();
	}
	else
	{
		CurrentShakeFrame = 0.f;
	}
}

void UHitstunComponent::UpdatePlayerShake(float DeltaTime)
{
	if (CurrentShakeFrame <= ShakeFrameCount)
	{
		CurrentShakeFrame += DeltaTime * 60.f;  //Update the shake frame		

		FVector NewMeshLocation = OwningPlayer->GetMesh()->GetComponentToWorld().GetTranslation();

		OwningPlayer->GetMesh()->SetWorldLocation(FVector(NewMeshLocation.X, NewMeshLocation.Y + (ShakeDirection*ShakeOffset), NewMeshLocation.Z));
		ShakeDirection *= -1.f; //Switch up the shake direction
		if (CurrentShakeFrame >= ShakeFrameCount)
		{
			CurrentShakeFrame = 0.f; //Reset
			ShakeFrameCount = -1.f; //Make sure we don't enter the if statement
			OwningPlayer->GetMesh()->SetRelativeLocation(MeshPlayerOffset); //Reset the location of the player mesh
		}
	}
}

int UHitstunComponent::GetRemainingHitstunDuration()
{
	return CurrentHitstunFrame;
}

bool UHitstunComponent::GetIsInHitstun()
{
	return bIsInHitstun;
}

ABasePlayer * UHitstunComponent::GetOwningPlayer()
{
	return OwningPlayer;
}

bool UHitstunComponent::GetIsInvulnerable()
{
	return bIsInvulnerable;
}

void UHitstunComponent::MakeInvulnrable(float InvulnrabilityFrames)
{
	bIsInvulnerable = true;
	MaxInvulnerableFrames = InvulnrabilityFrames;
	CurrentInvulnerableFrames = 0.f;
}

float UHitstunComponent::GetCurrentStunMeterFrame()
{
	return CurrentStunMeterFrame;
}

void UHitstunComponent::UpdateStunMeter(float DeltaTime)
{
	//check if we have any hitstun accumulated
	if (StunMeter > 0.f)
	{

		if (StunMeter >= MaxStunMeter)
		{
			CurrentStunMeterFrame = 0;
			Stun(FullStunMeterStunFrames, 0.f, true);
			StunMeter = 0.f;
		}

		//check if the hitstun's lifetime is more than it should, if it's more start decreasing it
		if (CurrentStunMeterFrame >= StunMeterDecreaseDelayInFrames)
		{
			StunMeter = StunMeter - StunMeterDecreasePerFrame;

			if (StunMeter < 0.f)
			{
				StunMeter = 0.f;
			}
		}
		else
		{
			//else update the hitstun frame
			++CurrentStunMeterFrame;
		}
	}
}

bool UHitstunComponent::IsInHitstun()
{
	return bIsInHitstun;
}

void UHitstunComponent::SetIsInvulnrable(bool value)
{
	bIsInvulnerable = value;
}

void UHitstunComponent::UpdateFreeze(float DeltaTime)
{
	if (bIsFrozen)
	{
		//Use delta to calculate the current freeze frame
		CurrentFreezeFrame += DeltaTime;
		if (CurrentFreezeFrame >= (MaxFreezeFrames / 60.f))
		{
			UnfreezeCharacter();
			OwningPlayer->OnEndFreezeFrames();
		}
	}
}

void UHitstunComponent::UpdateHitstun(float DeltaTime)
{
	//Update hitstun
	if (bIsInHitstun)
	{
		CurrentHitstunFrame += DeltaTime;
		if (CurrentHitstunFrame >= (MaxHitstunFrames / 60.f))
		{
			EndStun();
		}
	}
}