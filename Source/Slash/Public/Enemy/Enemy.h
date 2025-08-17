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

	virtual void  GetHit_Implementation(const FVector& ImpactPoint) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void  Destroyed() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Die() override;
	bool         InTargetRange(AActor* Target, double Radius);

	UFUNCTION()
	void PawnSeen(APawn* Pawn);

	virtual void  Attack() override;
	virtual bool  CanAttack() override;
	virtual void  HandleDamage(float DamageAmount) override;
	virtual int32 PlayDeathMontage() override;

	UPROPERTY(EditAnywhere, Category=Combat)
	float DeathLifeSpan = 8.f;

	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<EDeathPose> DeathPose;

	UPROPERTY(BlueprintReadOnly)
	EEnemyState EnemyState = EEnemyState::EES_Patrolling;

private:
	/**
	 *  Components
	 */
	UPROPERTY(VisibleAnywhere)
	UHealthBarComponent* HealthBarWidget;

	UPROPERTY(VisibleAnywhere)
	UPawnSensingComponent* PawnSensing;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AWeapon> WeaponClass;

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

	// Combat
	void StartAttackTimer();

	FTimerHandle AttackTimer;

	UPROPERTY(EditAnywhere, Category=Combat)
	float AttackMin = 0.5f;
	float AttackMax = 1.f;

	UPROPERTY(EditAnywhere, Category="Combat")
	float PatrollingSpeed = 125.f;

	UPROPERTY(EditAnywhere, Category="Combat")
	float ChasingSpeed = 300.f;

	// AI Behavior
	void HideHealthBar();
	void ShowHealthBar();
	void LoseInterest();
	void StartPatrolling();
	void ChaseTarget();

	bool IsOutsideCombatRadius();
	bool IsOutsideAttackRadius();
	bool IsInsideAttackRadius();
	bool IsChasing();
	bool IsAttacking();
	bool IsDead();
	bool IsEngaged();

	void ClearPatrolTimer();
	void ClearAttackTimer();
};