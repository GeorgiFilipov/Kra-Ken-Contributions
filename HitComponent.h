// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/BoxComponent.h"
#include "HitComponent.generated.h"


class ABasePlayer;
class AProjectile;
class UBoxComponent;
class USoundCue;

//The type of the attack which is being thrown
UENUM(BlueprintType)
enum class EAttackType : uint8
{
	LightAttack,
	MediumAttack,
	HeavyAttack,
	Empty
};

//Describe which limb are you using to attack
UENUM(BlueprintType)
enum class EAttackingLimb : uint8
{
	Punch,
	Kick,
	Empty
};

UENUM(BlueprintType)
enum class EAttackTarget : uint8
{
	Overhead,	// Overhead
	High,		// Standing
	Low,		// Crouching
	Empty
};

USTRUCT(BlueprintType)
struct FAttackHit
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int HitStunFrames;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int SelfStunFrames;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float BlockStun;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Damage;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bCancelSelfKnockbackOnSuccesfulHit;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bIsSuper;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float SuperMeterOnHit;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float SuperMeterOnBlock;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float SuperMeterOnMiss;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float HitKnockbackDistance;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float BlockKnockbackDistance;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector KnockbackDirection;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int KnockbackFrames;

	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bIsLauncher;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float AirKnockbackXDistance;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float AirKnockbackYDistance;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float AirKnockbackDuration;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bJuggleOK;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EAttackType AttackType;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EAttackingLimb AttackingLimb;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EAttackTarget AttackTarget;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector HitboxScale;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector HitboxOffset;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool GroundTheEnemy;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int GroundedFrames;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float StunMeterPerAttack;	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bIsSpecial;	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	USoundCue* OnBlockedHitSound;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	USoundCue* OnHitSound;	
	UPROPERTY(EditAnywhere, Category = Parameters)
	float CornerPushbackDistance;
	UPROPERTY(EditAnywhere, Category = Parameters)
	int CornerPushbackDuration;
	UPROPERTY(EditAnywhere, Category = Parameters)
	bool bCancelKnockbackOnSuccesfulHit = false;
	UPROPERTY(EditAnywhere, Category = Misc)
	bool bHiddenHitbox = true;
};


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TENSHIN_API UHitComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UHitComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HitBox")
	FVector HitBoxLocationOffset;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HitBox")
	FVector HitBoxScale;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HitBox")
	FRotator HitBoxRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "HitBox")
	UBoxComponent* HitBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsChargeDash = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool hasHitOnce = false;

	bool bHasActiveHitbox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ABasePlayer* MyOwner;

	AProjectile* OwningProjectile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAttackHit CurrentHit;

	//Is the attack special (EX-move). Would it go through other attacks?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsSpecial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsSuper;

	// FUNCTIONS: -------------------

	/* Call this at the start of the active frames of an attack
	* @param Hit - describe what kind of attack you're throwing
	*/
	UFUNCTION(BlueprintCallable)
	void StartActiveHitBox(FAttackHit Hit);
	
	void SetHitboxTransform(FVector aHitBoxLocationOffset, FVector aHitBoxScale, FRotator aHitBoxRotation);	

	virtual void HitBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	/* Call this function when an overlap between the hitbox and an actor happens.
	   The function does the damage handling for normal/special attacks which are not projectiles.
		@param Enemy - Enemy player
		@param EnemyHurtBox - Enemy hurtbox
	*/
	void ProcessAttackNormal(ABasePlayer * Enemy, UBoxComponent * EnemyHurtBox);

	/* Call this function to handle the collision between a player with projectile or two projectiles.	   
		@param Enemy - Enemy player
		@param EnemyHurtBox - Enemy hurtbox
	*/
	void ProcessAttackProjectileToProjectile(ABasePlayer * Enemy, UBoxComponent * EnemyHurtBox, AProjectile * EnemyProjectile);
	/* Call this to deal damage on a successful hit.	    
	*/
	void ProcessDamage(ABasePlayer* Enemy, UBoxComponent * EnemyHurtBox);

	void EndActiveHitBox();

	UFUNCTION(BlueprintCallable)
	UBoxComponent* GetHitBox();

	/*
	*  Use this to clear the hit struct. It's called at the end of each hitbox and at the ctor of the class
	*/
	void ClearHitStruct();

private:
	// Keeps track of the interval time between super hit
	float SuperInternalTimer;
	// Reference to the enemy player hurtbox for the super attack
	UBoxComponent* EnemyHurtbox;

	bool bHitBlocked;

	void SpawnProjectileHitParticles(FVector ParticleSpawnLocation);

};
