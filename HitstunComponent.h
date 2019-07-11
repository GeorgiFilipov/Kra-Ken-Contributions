// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HitstunComponent.generated.h"

class ABasePlayer;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TENSHIN_API UHitstunComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHitstunComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/* Stun the character for x amount of time
	*@Hitstun - how long should the player be stunned for
	*/
	UFUNCTION(BlueprintCallable)
	void Stun(float Hitstun, float FreezeFrames, float StunMeterPerAttack, bool bShouldChangeStateToStunned = true, bool bFreezeCharacter = true);
		
	/* Stun the character for x amount of time
	*@Hitstun - how long should the player be stunned for
	*@FreezeFrames - how long should we be frozen for
	*@bFreezeCharacter - should we stop the animations of the character
	*@bool bShouldChangeStateToStunned - should we change the player's state to stunned (used when you're applying self-freeze)
	*@InvulnerabilityFrames - how many frames of invulnerability do we have
	*Note - Invulnerability is called Grounded inside the BasePlayer
	*/
	UFUNCTION(BlueprintCallable)
	void StunWithInvulnerability(int HitstunFrames, int FreezeFrames, int InvulnerabilityFrames, float StunMeterPerAttack,bool bShouldChangeStateToStunned = true, bool bFreezeCharacter = false);

	//Updates the current invulnerability frames
	void UpdateInvulnerability(float DeltaTime);
	
	//Call this when the invulnerability is over
	void EndInvulnerability();

	UFUNCTION(BlueprintCallable)
	void FreezeCharacter();

	UFUNCTION(BlueprintCallable)
	void UnfreezeCharacter();

	UFUNCTION(BlueprintCallable)
	void EndStun();

	void HitStunFreeze(int FreezeFrames);
	
	//Shakes the players mesh in place 
	//@param direction - direction in which the player's mesh would be shook 
	//@param offset - how many units from origin would we move the player
	//@param how many frames should we do that for
	UFUNCTION(BlueprintCallable)
	void ShakePlayer(float Direction, float Offset, float FrameCount);		


	UFUNCTION(BlueprintCallable)
	int GetRemainingHitstunDuration();

	UFUNCTION(BlueprintCallable)
	bool GetIsInHitstun();

	UFUNCTION(BlueprintCallable)
	ABasePlayer* GetOwningPlayer();

	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<EMovementMode> LastMovementMode;

	UFUNCTION(BlueprintCallable)
	bool GetIsInvulnerable();

	//Make the player invulnerable for x amount of frames
	void MakeInvulnrable(float InvulnrabilityFrames);

	float GetCurrentStunMeterFrame();

	//return if the character is in hitstun
	bool IsInHitstun();
	   
	//Stun meter
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "StunMeter")
	float StunMeter;	
	
	//Hitstun decrease after this frame delay
	//The delay is in FRAMES!
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "StunMeter")
	int StunMeterDecreaseDelayInFrames;
	
	//How much stunmeter to lose per frame
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "StunMeter")
	float StunMeterDecreasePerFrame;

	//How long should you be stunned if the stun meter gets full
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "StunMeter")
	int FullStunMeterStunFrames;

	//Make the player invulnerable
	void SetIsInvulnrable(bool value);

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bIsFrozen;

private:
	ABasePlayer* OwningPlayer;

	//How much stun meter automatically stuns the player
	float MaxStunMeter;
	//Stun frames left until the player can attack again
	float MaxHitstunFrames;
	//Current amount of stun that has passed
	float CurrentHitstunFrame;		
	float CurrentStunMeterFrame;
	float CurrentStunMeterRemainingFrames;
	
	float MaxFreezeFrames;
	float CurrentFreezeFrame;
	
	bool bIsInHitstun;	
	bool bIsInvulnerable;

	float MaxInvulnerableFrames;
	float CurrentInvulnerableFrames;

	float ShakeOffset;
	float ShakeDirection;
	float ShakeFrameCount;
	float CurrentShakeFrame;
	FVector MeshOriginalLocation;
	FVector MeshPlayerOffset;

	void UpdateFreeze(float DeltaTime);
	void UpdateHitstun(float DeltaTime);	
	void UpdatePlayerShake(float DeltaTime);
	void UpdateStunMeter(float DeltaTime);
};