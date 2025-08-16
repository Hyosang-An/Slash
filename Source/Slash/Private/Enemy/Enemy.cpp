// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"

#include "Components/AttributeComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HUD/HealthBarComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Navigation/PathFollowingComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "Runtime/AIModule/Classes/AIController.h"
#include "Slash/DebugMacros.h"

// Sets default values
AEnemy::AEnemy()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetGenerateOverlapEvents(true);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	HealthBarWidget = CreateDefaultSubobject<UHealthBarComponent>(TEXT("HealthBarWidget"));
	HealthBarWidget->SetupAttachment(GetRootComponent());

	PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensing"));
	PawnSensing->SightRadius = 4000.f;
	PawnSensing->SetPeripheralVisionAngle(45.f);

	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationRoll = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (Attributes && HealthBarWidget)
	{
		HealthBarWidget->SetHealthPercent(Attributes->GetHealthPercent());

		HealthBarWidget->SetVisibility(false);
	}

	EnemyController = Cast<AAIController>(GetController());

	// Start the timer to check when enemy can begin patrolling 
	// (depends on NavMesh being ready)
	GetWorldTimerManager().SetTimer(BeginPatrolTimer, this, &AEnemy::BeginPatrolling, 1.0f, true);

	if (PawnSensing)
	{
		PawnSensing->OnSeePawn.AddDynamic(this, &AEnemy::PawnSeen);
	}
}

/**
 * CombatTarget와의 거리를 확인하여, CombatTarget이 범위를 벗어나면 CombatTarget을 nullptr로 설정하고 HealthBarWidget을 숨깁니다.
 */
void AEnemy::CheckCombatTarget()
{
	if (!InTargetRange(CombatTarget, CombatRange))
	{
		// Outside combat radius, lose interest
		CombatTarget = nullptr;
		if (HealthBarWidget)
		{
			HealthBarWidget->SetVisibility(false);
		}

		EnemyState = EEnemyState::EES_Patrolling;
		GetCharacterMovement()->MaxWalkSpeed = 125.f; // Reset speed to patrol speed
		MoveToTarget(CurrentPatrolTarget);
	}
	else if (!InTargetRange(CombatTarget, AttackRange) && EnemyState != EEnemyState::EES_Chasing)
	{
		// Outside attack radius, chase the target
		EnemyState = EEnemyState::EES_Chasing;
		GetCharacterMovement()->MaxWalkSpeed = 300.f; // Set speed to chase speed
		MoveToTarget(CombatTarget);
	}
	else if (InTargetRange(CombatTarget, AttackRange) && EnemyState != EEnemyState::EES_Attacking)
	{
		// Inside Attack range, attack character
		EnemyState = EEnemyState::EES_Attacking;
		// TODO: Attack Montage
	}
}

/**
 * 순찰 대상과의 거리를 확인하여 도달했을 때 새로운 순찰 지점을 선택하고,
 * 대기 시간을 랜덤으로 설정한 후 타이머를 시작합니다.
 */
void AEnemy::CheckPatrolTarget()
{
	if (InTargetRange(CurrentPatrolTarget, PatrolRadius))
	{
		CurrentPatrolTarget = ChoosePatrolTarget();
		const float WaitTime = FMath::RandRange(WaitMin, WaitMax);
		GetWorldTimerManager().SetTimer(PatrolTimer, this, &AEnemy::PatrolTimerFinished, WaitTime);
	}
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (EnemyState > EEnemyState::EES_Patrolling)
	{
		CheckCombatTarget();
	}
	else
	{
		CheckPatrolTarget();
	}
}

void AEnemy::Die()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && DeathMontage)
	{
		AnimInstance->Montage_Play(DeathMontage);

		const int32 Selection = FMath::RandRange(0, 5);
		FName       SectionName = FName();
		switch (Selection)
		{
			case 0:
				SectionName = FName("Death1");
				DeathPose = EDeathPose::EDP_Death1;
				break;
			case 1:
				SectionName = FName("Death2");
				DeathPose = EDeathPose::EDP_Death2;
				break;
			case 2:
				SectionName = FName("Death3");
				DeathPose = EDeathPose::EDP_Death3;
				break;
			case 3:
				SectionName = FName("Death4");
				DeathPose = EDeathPose::EDP_Death4;
				break;
			case 4:
				SectionName = FName("Death5");
				DeathPose = EDeathPose::EDP_Death5;
				break;
			case 5:
				SectionName = FName("Death6");
				DeathPose = EDeathPose::EDP_Death6;
				break;
			default:
				break;
		}

		AnimInstance->Montage_JumpToSection(SectionName, DeathMontage);
	}

	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(false);
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetLifeSpan(7.f); // Destroy after 3 seconds
}

bool AEnemy::InTargetRange(AActor* Target, double Radius)
{
	if (Target == nullptr)
		return false;

	const double DistanceToTarget = (GetActorLocation() - Target->GetActorLocation()).Size();

	return DistanceToTarget <= Radius;
}


/**
 * 감지된 Pawn이 "SlashCharacter" 태그를 가지고 있는 경우, 추적 상태로 전환하고 해당 Pawn을 목표로 설정하며 추적을 시작합니다.
 * 추적 상태 전환 시, 전역 타이머를 정리하고 이동 속도를 300.f로 설정합니다.
 *
 * @param SeenPawn 감지된 Pawn 객체
 */
void AEnemy::PawnSeen(APawn* SeenPawn)
{
	if (EnemyState == EEnemyState::EES_Chasing)
		return;

	if (SeenPawn->ActorHasTag(FName("SlashCharacter")))
	{
		GetWorldTimerManager().ClearTimer(PatrolTimer);
		GetCharacterMovement()->MaxWalkSpeed = 300.f;
		CombatTarget = SeenPawn;

		if (EnemyState != EEnemyState::EES_Attacking)
		{
			EnemyState = EEnemyState::EES_Chasing;
			MoveToTarget(CombatTarget);
		}
	}
}

void AEnemy::MoveToTarget(AActor* Target)
{
	if (EnemyController == nullptr || Target == nullptr)
		return;

	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(Target);
	MoveRequest.SetAcceptanceRadius(15.f);

	EnemyController->MoveTo(MoveRequest);
}

AActor* AEnemy::ChoosePatrolTarget()
{
	TArray<AActor*> ValidTargets;
	for (auto Target : PatrolTargets)
	{
		if (Target != CurrentPatrolTarget)
		{
			ValidTargets.AddUnique(Target);
		}
	}

	const int32 NumPatrolTargets = ValidTargets.Num();
	if (NumPatrolTargets > 0)
	{
		const int32 TargetSelection = FMath::RandRange(0, NumPatrolTargets - 1);
		return ValidTargets[TargetSelection];
	}

	return nullptr;
}

void AEnemy::BeginPatrolling()
{
	// Set up patrolling AI Navigation
	if (EnemyController == nullptr)
		return;

	CurrentPatrolTarget = ChoosePatrolTarget();

	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(CurrentPatrolTarget);
	MoveRequest.SetAcceptanceRadius(15.f);
	FNavPathSharedPtr NavPath;
	EnemyController->MoveTo(MoveRequest, &NavPath);
	if (NavPath)
	{
		// stop timer now NavMesh is there			
		GetWorldTimerManager().ClearTimer(BeginPatrolTimer);
	}

}

void AEnemy::PatrolTimerFinished()
{
	MoveToTarget(CurrentPatrolTarget);
}


// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::GetHit_Implementation(const FVector& ImpactPoint)
{
	// DRAW_SPHERE_COLOR(ImpactPoint, FColor::Orange);

	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(true);
	}

	if (Attributes && Attributes->IsAlive())
	{
		DirectionalHitReact(ImpactPoint);
	}
	else
	{
		Die();
	}

	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, ImpactPoint);
	}
	if (HitParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticles, ImpactPoint);
	}
}

float AEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	if (Attributes && HealthBarWidget)
	{
		Attributes->ReceiveDamage(DamageAmount);
		HealthBarWidget->SetHealthPercent(Attributes->GetHealthPercent());
	}

	CombatTarget = EventInstigator->GetPawn();
	EnemyState = EEnemyState::EES_Chasing;
	MoveToTarget(CombatTarget);
	GetCharacterMovement()->MaxWalkSpeed = 300.f; // Set speed to chase speed

	return DamageAmount;
}