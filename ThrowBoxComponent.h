// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Components/ActorComponent.h"
#include "ThrowBoxComponent.generated.h"

enum class EPlayerStates : uint8;
class ABasePlayer;

USTRUCT(BlueprintType)
struct FThrowHit
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
		float HitStunFrames;
	UPROPERTY(BlueprintReadWrite)
		float Damage;
	UPROPERTY(BlueprintReadWrite)
		FVector HitboxScale;
	UPROPERTY(BlueprintReadWrite)
		FVector HitboxOffset;
	UPROPERTY(BlueprintReadWrite)
		int GroundedFrames;
	UPROPERTY(BlueprintReadWrite)
		FVector EnemyThrowPositionOffset;
	UPROPERTY(BlueprintReadWrite)
		FVector OwnLaunchPositionOffset;
	UPROPERTY(BlueprintReadWrite)
		FVector EnemyLaunchPositionOffset;
	UPROPERTY(BlueprintReadWrite)
		bool HasLaunch;
	UPROPERTY(BlueprintReadWrite)
		float LaunchDistance;
	UPROPERTY(BlueprintReadWrite)
		FVector OwnStartingLocation;
	UPROPERTY(BlueprintReadWrite)
		FVector LandOffset;
	//This is storing the original facing 	
	bool FacingRight;

};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TENSHIN_API UThrowBoxComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Sets default values for this component's properties
	UThrowBoxComponent();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ThrowBox")
		FVector ThrowBoxLocationOffset;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ThrowBox")
		FVector ThrowBoxScale;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ThrowBox")
		FRotator ThrowBoxRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ThrowBox")
		UBoxComponent* ThrowBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		ABasePlayer* MyOwner;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		ABasePlayer* MyEnemy;


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FThrowHit CurrentThrow;


	UFUNCTION(BlueprintCallable)
		void ThrowBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION(BlueprintCallable)
		void ThrowBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/* Call this at the start of the active frames of an attack
	* @param Hit - describe what kind of attack you're throwing
	*/
	UFUNCTION(BlueprintCallable)
		void StartActiveThrowBox(FThrowHit Hit);

	UFUNCTION(BlueprintCallable)
		void EndActiveThrowBox();

	UFUNCTION(BlueprintCallable)
		void SetThrowBoxScale(FVector aHitBoxLocationOffset, FVector aHitBoxScale, FRotator aHitBoxRotation/*, class ABasePlayer* aMyOwner, bool aIsChargeDash*/);

	/*This function is used to offset the enemy player so that the animation can look correct.
	@param PlayerPosition - the position of the player executing the throw.
	@param EnemyPlayerOffset - the offset which is passed in the Throw notify.
	*/
	FVector CalculateEnemyThrowPosition(FVector PlayerPosition, FVector EnemyPlayerOffset);

	/*
	*  Use this to clear the hit struct. It's called at the end of each hitbox and at the ctor of the class
	*/
	void ClearThrowStruct();

	float GetThrowFacingDirectionFloat();

	//@param get the vector value from the throw struct. 
	//This is where the enemy player must stand in order for the animation to play properly.
	float AdjustThrowPosition(float NewPositionY, bool& HasCollided);

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		bool IsEnemyInRange;

	bool IsForwardThrow;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FThrowHit DefaultThrowHitRange;

	bool HasHit;

private:
	float RightEdge;

	float LeftEdge;

	bool ShouldStart;


};
