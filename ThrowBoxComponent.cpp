// Fill out your copyright notice in the Description page of Project Settings.

#include "ThrowBoxComponent.h"
#include "BasePlayer.h"
#include "HitstunComponent.h"
#include "Runtime/Engine/Classes/Components/CapsuleComponent.h"
#include "Runtime/Engine/Classes/Engine/World.h"



UThrowBoxComponent::UThrowBoxComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	ThrowBox = CreateDefaultSubobject <UBoxComponent>(TEXT("ThrowBox"));
	ThrowBox->SetCollisionProfileName("ThrowBox");
	ThrowBox->OnComponentBeginOverlap.AddDynamic(this, &UThrowBoxComponent::ThrowBoxBeginOverlap);
	ThrowBox->OnComponentEndOverlap.AddDynamic(this, &UThrowBoxComponent::ThrowBoxEndOverlap);
	ThrowBox->SetGenerateOverlapEvents(false);
	ThrowBox->SetHiddenInGame(true);
	//ThrowBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	IsEnemyInRange = false;
	ShouldStart = false;
	HasHit = false;

	ClearThrowStruct();
}


// Called when the game starts
void UThrowBoxComponent::BeginPlay()
{
	Super::BeginPlay();

	IsEnemyInRange = false;

	//Get the player in begin play otherwise the character is nullptr
	auto* OwnerTry = dynamic_cast<ABasePlayer*>(GetOwner());
	check(OwnerTry)
	
	MyOwner = OwnerTry;

	ThrowBox->IgnoreActorWhenMoving(MyOwner, true);
	ThrowBox->IgnoreComponentWhenMoving(MyOwner->GrabBoxManager->GrabBox, true);

	ThrowBox->SetRelativeLocation(ThrowBoxLocationOffset, false, nullptr, ETeleportType::TeleportPhysics);
	ThrowBox->SetWorldRotation(ThrowBoxRotation, false, nullptr, ETeleportType::TeleportPhysics);
	ThrowBox->SetBoxExtent(ThrowBoxScale, true);
	
}


void UThrowBoxComponent::ThrowBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{	
	if ((OtherActor != nullptr) && (OtherActor != this->MyOwner) && (OtherComp != nullptr))
	{
		if (auto * Enemy = dynamic_cast<ABasePlayer*>(OtherActor))
		{
			if (MyOwner)
			{
				if (Enemy &&
					Enemy != MyOwner &&
					Enemy->GetCurrentState() != EPlayerStates::Grounded &&
					Enemy->GetCurrentState() != EPlayerStates::WakeUp &&
					!Enemy->IsInAir()
					)
				{
					if (Enemy->GetCurrentState() == EPlayerStates::ForwardThrow &&
						Enemy->GetCurrentState() == EPlayerStates::BackThrow)
					{
						MyOwner->TechThrow();
					}
					else
					{
						//Call that so that the enemy player don't fly off when dashing.					
						MyEnemy = Enemy;
						//Store throw data in each player
						MyOwner->MyThrow = CurrentThrow;
						MyEnemy->EnemyThrow = CurrentThrow;

						LeftEdge = -(MyOwner->GameplayCamera->GetBoundWidth() + MyOwner->GameplayCamera->GetPlayerBoundWidth());
						RightEdge = MyOwner->GameplayCamera->GetBoundWidth() + MyOwner->GameplayCamera->GetPlayerBoundWidth();

						//store the initial positions
						float OldPlayerY = MyOwner->GetActorLocation().Y;
						float OldEnemyY = Enemy->GetActorLocation().Y;

						//Check if we're hitting a wall
						bool WallCollide = false;

						//Figure out what's the enemy player's final location
						float NewEnemyLandY;
						if (IsForwardThrow)
						{
							NewEnemyLandY = AdjustThrowPosition(OldEnemyY + CurrentThrow.EnemyThrowPositionOffset.Y, WallCollide);
						}
						else
						{
							NewEnemyLandY = AdjustThrowPosition(OldEnemyY - CurrentThrow.EnemyThrowPositionOffset.Y, WallCollide);
						}

						//Adjust player positions on wall hit
						if (WallCollide)
						{
							float EnemyPlayerPosNew = 0.f;
							float PlayerPos = 0.f;

							//Calculate where the player should stay in regards to his throw when we're close to a wall
							if (IsForwardThrow)
							{
								PlayerPos = NewEnemyLandY - CurrentThrow.EnemyThrowPositionOffset.Y;
							}
							else
							{
								PlayerPos = NewEnemyLandY + CurrentThrow.EnemyThrowPositionOffset.Y;
							}

							//Start the new being thrown state
							MyEnemy->OnReceiveThrow_Implementation(CurrentThrow, MyOwner);

							//Get the new enemyposition
							EnemyPlayerPosNew = PlayerPos + CurrentThrow.EnemyThrowPositionOffset.Y;

							Enemy->SetActorLocation(FVector(Enemy->GetActorLocation().X, EnemyPlayerPosNew, Enemy->GetActorLocation().Z));
							MyOwner->SetActorLocation(FVector(MyOwner->GetActorLocation().X, PlayerPos, MyOwner->GetActorLocation().Z));

						}
						else
						{
							//Adjust locations so that the player is offset enough from the enemy so that the throw animation can play as intended.
							MyEnemy->SetActorLocation(CurrentThrow.OwnStartingLocation + CurrentThrow.EnemyThrowPositionOffset);
						}

						ThrowBox->SetGenerateOverlapEvents(false);
						HasHit = true;
						MyEnemy->OnReceiveThrow_Implementation(CurrentThrow, MyOwner);
					}
				}
			}
		}
	}
}

void UThrowBoxComponent::ThrowBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	auto * Enemy = dynamic_cast<ABasePlayer*>(OtherActor);

	if (Enemy)
	{
		IsEnemyInRange = false;
		Enemy = nullptr;
	}

}

void UThrowBoxComponent::StartActiveThrowBox(FThrowHit Throw)
{
	CurrentThrow = Throw;

	//Enable collision
	ThrowBox->SetGenerateOverlapEvents(true);
	SetThrowBoxScale(FVector(Throw.HitboxOffset.X, Throw.HitboxOffset.Y, Throw.HitboxOffset.Z), (Throw.HitboxScale*30.f), FRotator(0.f, 0.f, 0.f)); //Multiplying the scale since scaling between 1-10 almost yield no result
	ShouldStart = true;
}

void UThrowBoxComponent::EndActiveThrowBox()
{
	//Disable collision after we've 
	ThrowBox->SetGenerateOverlapEvents(false);
	ThrowBox->SetBoxExtent(FVector(0.f));

	ShouldStart = false;

	//Check if the player missed with his grab
	if (HasHit == false)
	{
		//Set the throw data
		MyOwner->MyThrow = CurrentThrow;
		//Play the whiff animation
		MyOwner->Whiff();
	}
}

void UThrowBoxComponent::SetThrowBoxScale(FVector aThrowBoxLocationOffset, FVector aThrowBoxScale, FRotator aThrowBoxRotation/*, ABasePlayer* aMyOwner, bool aIsChargeDash*/)
{
	ThrowBoxLocationOffset = aThrowBoxLocationOffset;
	ThrowBoxScale = aThrowBoxScale;
	ThrowBoxRotation = aThrowBoxRotation;
}

FVector UThrowBoxComponent::CalculateEnemyThrowPosition(FVector PlayerPosition, FVector EnemyPlayerOffset)
{
	return FVector(PlayerPosition + EnemyPlayerOffset);
}


void UThrowBoxComponent::ClearThrowStruct()
{
	CurrentThrow.HitStunFrames = 0.f;
	CurrentThrow.Damage = 0.f;
}

float UThrowBoxComponent::GetThrowFacingDirectionFloat()
{
	float EnemyFacingDir = (float)(CurrentThrow.FacingRight);
	if (!EnemyFacingDir)
	{
		EnemyFacingDir = -1.f;
	}

	return EnemyFacingDir;
}

float UThrowBoxComponent::AdjustThrowPosition(float NewPositionY, bool& HasCollided)
{
	float NewY = NewPositionY;

	if (NewY > RightEdge)
	{
		HasCollided = true;
		return	RightEdge;
	}
	else if (NewY < LeftEdge)
	{
		HasCollided = true;
		return	 LeftEdge;
	}
	HasCollided = false;
	return NewY;
}