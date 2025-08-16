// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "Characters/CharacterTypes.h"
#include "Enemy.generated.h"

class UPawnSensingComponent;
class UHealthBarComponent;

UCLASS()
class SLASH_API AEnemy : public ABaseCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void  SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void  GetHit_Implementation(const FVector& ImpactPoint) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Die() override;
	bool InTargetRange(AActor* Target, double Radius);

	UFUNCTION()
	void PawnSeen(APawn* Pawn);

	UPROPERTY(BlueprintReadOnly)
	EDeathPose DeathPose = EDeathPose::EDP_Alive;

private:
	/**
	 *  Components
	 */
	UPROPERTY(VisibleAnywhere)
	UHealthBarComponent* HealthBarWidget;

	UPROPERTY(VisibleAnywhere)
	UPawnSensingComponent* PawnSensing;
	

	UPROPERTY()
	AActor* CombatTarget;

	UPROPERTY(EditAnywhere)
	double CombatRange = 500.f;

	UPROPERTY(EditAnywhere)
	double AttackRange = 150.f;
	
	/**
	 * Navigation
	 */
	FTimerHandle BeginPatrolTimer;
	void         BeginPatrolling();
	void         MoveToTarget(AActor* Target);
	AActor*      ChoosePatrolTarget();
	void         CheckCombatTarget();
	void         CheckPatrolTarget();
	FTimerHandle PatrolTimer;
	void         PatrolTimerFinished();

	UPROPERTY()
	class AAIController* EnemyController;

	UPROPERTY(EditInstanceOnly, Category="AI Navigation")
	AActor* CurrentPatrolTarget;

	UPROPERTY(EditInstanceOnly, Category="AI Navigation")
	TArray<AActor*> PatrolTargets;

	UPROPERTY(EditAnywhere)
	double PatrolRadius = 200.f;

	UPROPERTY(EditAnywhere, Category="AI Navigation")
	float WaitMin = 5.f;

	UPROPERTY(EditAnywhere, Category="AI Navigation")
	float WaitMax = 10.f;

	EEnemyState EnemyState = EEnemyState::EES_Patrolling;
};