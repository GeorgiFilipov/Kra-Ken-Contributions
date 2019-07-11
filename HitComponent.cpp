// Fill out your copyright notice in the Description page of Project Settings.

#include "HitComponent.h"
#include "Engine/World.h"
#include "Engine/GameEngine.h"
#include "Kismet/GameplayStatics.h"
#include "BasePlayer.h"
#include "Projectile.h"
#include "HitstunComponent.h"
#include "ComboComponent.h"


// Sets default values for this component's properties
UHitComponent::UHitComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	HitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox"));
	HitBox->SetCollisionProfileName("HitBox");
	HitBox->OnComponentBeginOverlap.AddDynamic(this, &UHitComponent::HitBoxBeginOverlap);
	HitBox->SetGenerateOverlapEvents(false);
	HitBox->SetHiddenInGame(true);
	HitBox->SetBoxExtent(FVector(0, 0, 0), false);
	ClearHitStruct();
}


void UHitComponent::ProcessAttackNormal(ABasePlayer * Enemy, UBoxComponent * EnemyHurtBox)
{
	if (hasHitOnce == false && Enemy != nullptr && Enemy != MyOwner && EnemyHurtBox != nullptr && EnemyHurtBox->GetCollisionProfileName() == "HurtBox")
	{
		hasHitOnce = true;
		if (!Enemy->HitstunComponent->GetIsInvulnerable())
		{
			//Check if the attack target is opposite to the guarding target
			if (Enemy->GetCurrentState() == EPlayerStates::Guarding && CurrentHit.AttackTarget == EAttackTarget::Low)
			{
				ProcessDamage(Enemy, EnemyHurtBox);
			}
			else if (Enemy->GetCurrentState() == EPlayerStates::CrouchGruarding && CurrentHit.AttackTarget == EAttackTarget::Overhead)
			{
				ProcessDamage(Enemy, EnemyHurtBox);
			}
			else if (Enemy->GetCurrentState() != EPlayerStates::CrouchGruarding && Enemy->GetCurrentState() != EPlayerStates::Guarding)
			{
				ProcessDamage(Enemy, EnemyHurtBox);
			}
			else
			{
				MyOwner->SetSuperMeter(CurrentHit.SuperMeterOnBlock);
				Enemy->bBlockedEnemyHit = true;
				bHitBlocked = true;
				Enemy->OnBlockedHit(CurrentHit, MyOwner, EnemyHurtBox);
			}

			if (CurrentHit.bCancelSelfKnockbackOnSuccesfulHit)
			{
				MyOwner->StopKnockBack();
			}
		}

	}
}

void UHitComponent::ProcessAttackProjectileToProjectile(ABasePlayer * Enemy, UBoxComponent * EnemyHurtBox, AProjectile * EnemyProjectile)
{
	if (EnemyProjectile  && OwningProjectile)
	{
		FVector MyProjectileLocation = OwningProjectile->GetActorLocation();
		FVector EnemyProjectileLocation = EnemyProjectile->GetActorLocation();
		FVector ParticleSpawnLoc = FVector(MyProjectileLocation.X, (MyProjectileLocation.Y + EnemyProjectileLocation.Y) / 2, MyProjectileLocation.Z);
		if (EnemyProjectile != OwningProjectile)
		{
			//Enemy projectile is special
			if (EnemyProjectile->Hitbox->bIsSpecial)
			{
				if (OwningProjectile->bFiredSuper)
				{
					// Super projectile destroys all other projectiles on impact
					SpawnProjectileHitParticles(ParticleSpawnLoc);
					EnemyProjectile->DestroyProjectile();
				}
				else if (OwningProjectile->Hitbox->bIsSpecial)
				{
					SpawnProjectileHitParticles(ParticleSpawnLoc);
					EnemyProjectile->DestroyProjectile();
					OwningProjectile->DestroyProjectile();

				}
				else
				{
					SpawnProjectileHitParticles(ParticleSpawnLoc);
					OwningProjectile->DestroyProjectile();

				}
			}
			else //Enemy projectile is NOT special
			{
				if (OwningProjectile->Hitbox->bIsSpecial)
				{
					SpawnProjectileHitParticles(ParticleSpawnLoc);
					EnemyProjectile->DestroyProjectile();
				}
				else
				{
					//both projectiles are normal
					EnemyProjectile->DestroyProjectile();
					OwningProjectile->DestroyProjectile();
				}
			}
		}
	}
}

void UHitComponent::ProcessDamage(ABasePlayer* Enemy, UBoxComponent * EnemyHurtBox)
{
	Enemy->bConfirmedEnemyHit = true;
	MyOwner->OnSuccessfulHit(CurrentHit, Enemy, EnemyHurtBox);
	MyOwner->HitstunComponent->Stun(0, CurrentHit.SelfStunFrames, 0.f, false, true);
	Enemy->OnReceiveHit_Implementation(CurrentHit, MyOwner);
}

UBoxComponent* UHitComponent::GetHitBox()
{
	return HitBox;
}

// Called when the game starts
void UHitComponent::BeginPlay()
{
	Super::BeginPlay();

	//Get the player in begin play otherwise the character is nullptr
	auto* Owner = GetOwner();
	auto* TryPlayer = dynamic_cast<ABasePlayer*>(Owner);
	check(TryPlayer)

	MyOwner = TryPlayer;
	HitBox->IgnoreActorWhenMoving(MyOwner, true);
	HitBox->AttachToComponent(Cast<USceneComponent>(MyOwner->GetMesh()), FAttachmentTransformRules::KeepRelativeTransform);

	HitBox->SetRelativeLocation(HitBoxLocationOffset, false, nullptr, ETeleportType::TeleportPhysics);
	HitBox->SetRelativeRotation(HitBoxRotation, false, nullptr, ETeleportType::TeleportPhysics);
	HitBox->SetBoxExtent(HitBoxScale, true);
}

void UHitComponent::HitBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	auto * EnemyHurtBox = dynamic_cast<UBoxComponent*>(OtherComp);
	auto * Enemy = dynamic_cast<ABasePlayer*>(OtherActor);
	auto * EnemyProjectile = dynamic_cast<AProjectile*>(OtherActor);

	ProcessAttackProjectileToProjectile(Enemy, EnemyHurtBox, EnemyProjectile);
	ProcessAttackNormal(Enemy, EnemyHurtBox);
}

void UHitComponent::StartActiveHitBox(FAttackHit Hit)
{
	HitBox->SetHiddenInGame(Hit.bHiddenHitbox);
	HitBox->SetGenerateOverlapEvents(true);
	CurrentHit = Hit;
	SetHitboxTransform(Hit.HitboxOffset, (Hit.HitboxScale), FRotator(0.f, 0.f, 0.f));	
	MyOwner->MyAttackState = SAttackState::ACTIVE;//AI relevant
}

void UHitComponent::EndActiveHitBox()
{
	HitBox->SetGenerateOverlapEvents(false);
	HitBox->SetBoxExtent(FVector(0, 0, 0), false);
	HitBox->SetRelativeLocation(FVector(0, 0, 0), false, nullptr, ETeleportType::TeleportPhysics);
	hasHitOnce = false;
	bHasActiveHitbox = false;
	ClearHitStruct();
	MyOwner->MyAttackState = SAttackState::RECOVERY; //AI relevant
}

void UHitComponent::SetHitboxTransform(FVector aHitBoxLocationOffset, FVector aHitBoxScale, FRotator aHitBoxRotation/*, ABasePlayer* aMyOwner, bool aIsChargeDash*/)
{
	HitBox->SetRelativeLocation(aHitBoxLocationOffset, false, nullptr, ETeleportType::TeleportPhysics);	
	HitBox->SetBoxExtent(aHitBoxScale, true);
	HitBox->SetRelativeRotation(aHitBoxRotation, false, nullptr, ETeleportType::TeleportPhysics);
	bHasActiveHitbox = true;
}

void UHitComponent::ClearHitStruct()
{
	CurrentHit.AttackingLimb = EAttackingLimb::Empty;
	CurrentHit.AttackTarget = EAttackTarget::Empty;
	CurrentHit.AttackType = EAttackType::Empty;
	CurrentHit.HitStunFrames = 0;
	CurrentHit.Damage = 0.f;
	CurrentHit.HitKnockbackDistance = 0.f;
	CurrentHit.KnockbackDirection = FVector(0.f);
	CurrentHit.BlockKnockbackDistance = 0.f;
	CurrentHit.BlockStun = 0.f;
	CurrentHit.KnockbackFrames = 0;
	CurrentHit.GroundedFrames = 0;
	CurrentHit.GroundTheEnemy = false;
	CurrentHit.StunMeterPerAttack = 0.f;
	CurrentHit.bIsSpecial = false;
	CurrentHit.bHiddenHitbox = true;
	CurrentHit.bCancelSelfKnockbackOnSuccesfulHit = false;
}

void UHitComponent::SpawnProjectileHitParticles(FVector ParticleSpawnLocation)
{
	FRotator ProjectileParticleRotation = MyOwner->FireballParticleRotation * MyOwner->GetInverseFacingDirectionF(); //Calculate the rotation in regards to the player facing direction
	UGameplayStatics::SpawnEmitterAtLocation(this, MyOwner->FireballParticle, ParticleSpawnLocation, MyOwner->GetActorRotation() + ProjectileParticleRotation);
}
