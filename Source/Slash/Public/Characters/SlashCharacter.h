// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "CharacterTypes.h"
#include "Interfaces/PickupInterface.h"
#include "SlashCharacter.generated.h"

class USlashOverlay;
class AWeapon;
class AItem;
class UGroomComponent;
class UCameraComponent;
class USpringArmComponent;
struct FInputActionValue;
class UInputAction;
class UInputMappingContext;


UCLASS()
class SLASH_API ASlashCharacter : public ABaseCharacter, public IPickupInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASlashCharacter();
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void  SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void  Jump() override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void  GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
	virtual void  SetOverlappingItem(AItem* Item) override;
	virtual void  AddSouls(ASoul* Soul) override;
	virtual void  AddGold(ATreasure* Treasure) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	UPROPERTY(EditAnywhere, Category="Input")
	UInputMappingContext* SlashMappingContext;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* EKeyAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* DodgeAction;

	/**
	*  Callbacks for input actions
	*/
	void         Move(const FInputActionValue& Value);
	void         Look(const FInputActionValue& Value);
	void         EKeyPressed();
	virtual void Attack() override;
	void         Dodge();

	// Combat
	void         EquipWeapon(AWeapon* Weapon);
	virtual void AttackEnd() override;
	virtual void DodgeEnd() override;
	virtual bool CanAttack() override;
	bool         CanDisArm();
	bool         CanArm();
	void         DisArm();
	void         Arm();
	void         PlayEquipMontage(const FName& SectionName);
	virtual void Die() override;
	bool         HasEnoughStamina(float StaminaCost);


	UFUNCTION(BlueprintCallable)
	void AttachWeaponToBack();

	UFUNCTION(BlueprintCallable)
	void AttachWeaponToHand();

	UFUNCTION(BlueprintCallable)
	void FinishEquipping();

	UFUNCTION(BlueprintCallable)
	void HitReactEnd();

private:
	bool IsUnoccupied();
	void InitializeSlashOverlay();
	void SetHUDHealth();

	// Character Components
	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* ViewCamera;

	UPROPERTY(VisibleAnywhere, Category="Hair")
	UGroomComponent* Hair;

	UPROPERTY(VisibleAnywhere, Category="Hair")
	UGroomComponent* Eyebrows;

	UPROPERTY(VisibleInstanceOnly)
	AItem* OverlappingItem;

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* EquipMontage;

	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;

	UPROPERTY(BlueprintReadWrite, meta =(AllowPrivateAccess = "true"))
	EActionState ActionState = EActionState::EAS_Unoccupied;

	UPROPERTY()
	USlashOverlay* SlashOverlay;

public:
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
	FORCEINLINE EActionState    GetActionState() const { return ActionState; }
};