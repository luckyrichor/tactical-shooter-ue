#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayEffect.h"
#include "GameplayCueNotify_Static.h"
#include "AIController.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Perception/AIPerceptionTypes.h"
#include "CombatTypes.generated.h"
#define LAB_ATTRIBUTE(Name) GAMEPLAYATTRIBUTE_PROPERTY_GETTER(UCombatAttributes,Name) GAMEPLAYATTRIBUTE_VALUE_GETTER(Name) GAMEPLAYATTRIBUTE_VALUE_SETTER(Name) GAMEPLAYATTRIBUTE_VALUE_INITTER(Name)
UCLASS() class COMBATLAB_API UCombatAttributes:public UAttributeSet {
 GENERATED_BODY()
public:
 UCombatAttributes();
 UPROPERTY(ReplicatedUsing=OnRep_Health) FGameplayAttributeData Health;
 UPROPERTY(ReplicatedUsing=OnRep_Stamina) FGameplayAttributeData Stamina;
 LAB_ATTRIBUTE(Health) LAB_ATTRIBUTE(Stamina)
 UFUNCTION() void OnRep_Health(const FGameplayAttributeData& Old);
 UFUNCTION() void OnRep_Stamina(const FGameplayAttributeData& Old);
 virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out)const override;
 virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
};
UCLASS() class COMBATLAB_API UDamageEffect:public UGameplayEffect {GENERATED_BODY() public:UDamageEffect();};
UCLASS() class COMBATLAB_API UDashCost:public UGameplayEffect {GENERATED_BODY() public:UDashCost();};
UCLASS() class COMBATLAB_API UPulseCost:public UGameplayEffect {GENERATED_BODY() public:UPulseCost();};
UCLASS() class COMBATLAB_API UDashAbility:public UGameplayAbility {GENERATED_BODY() public:UDashAbility();virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* Info,const FGameplayAbilityActivationInfo Activation,const FGameplayEventData* Event)override;};
UCLASS() class COMBATLAB_API UPulseAbility:public UGameplayAbility {GENERATED_BODY() public:UPulseAbility();virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* Info,const FGameplayAbilityActivationInfo Activation,const FGameplayEventData* Event)override;};
UCLASS() class COMBATLAB_API UDashCue:public UGameplayCueNotify_Static {GENERATED_BODY() public:UDashCue();virtual bool OnExecute_Implementation(AActor* Target,const FGameplayCueParameters& Parameters)const override;};
UCLASS() class COMBATLAB_API UPulseCue:public UGameplayCueNotify_Static {GENERATED_BODY() public:UPulseCue();virtual bool OnExecute_Implementation(AActor* Target,const FGameplayCueParameters& Parameters)const override;};
UCLASS() class COMBATLAB_API ACombatCharacter:public ACharacter,public IAbilitySystemInterface {
 GENERATED_BODY()
public:
 ACombatCharacter();
 UPROPERTY(VisibleAnywhere) TObjectPtr<UAbilitySystemComponent> Abilities;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UCombatAttributes> Attributes;
 UPROPERTY(Replicated,VisibleAnywhere) int32 LabId=0;
 UPROPERTY(Replicated,VisibleAnywhere) int32 Ammo=30;
 UPROPERTY(Replicated,VisibleAnywhere) bool Reloading=false;
 UPROPERTY(Replicated,VisibleAnywhere) FName AIState="Player";
 virtual UAbilitySystemComponent* GetAbilitySystemComponent()const override{return Abilities;}
 virtual void BeginPlay()override;
 virtual void Tick(float Delta)override;
 virtual void SetupPlayerInputComponent(UInputComponent* Input)override;
 virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out)const override;
 void ApplyHit(ACombatCharacter* Target);
 void Forward(float Value);void Right(float Value);void Look(float Value);void Pitch(float Value);
 void Fire();void Reload();void Dash();void Pulse();
 UFUNCTION(Server,Reliable) void ServerFire();
 UFUNCTION(Server,Reliable) void ServerReload();
 UFUNCTION(Server,Reliable) void ServerSkill(bool IsPulse);
 UFUNCTION(NetMulticast,Unreliable) void ShotFX(FVector End);
 double NextFire=0,NextDash=0,NextPulse=0,NextProbe=2,NextState=0,ProbeEnd=0;
};
UCLASS() class COMBATLAB_API AEnemyController:public AAIController {
 GENERATED_BODY()
public:
 AEnemyController();virtual void OnPossess(APawn* Pawn)override;
 UFUNCTION() void Sensed(AActor* Actor,FAIStimulus Stimulus);
};
UCLASS() class COMBATLAB_API UEnemyTask:public UBTTaskNode {
 GENERATED_BODY()
public:
 UEnemyTask();virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& Owner,uint8* Memory)override;
 virtual void TickTask(UBehaviorTreeComponent& Owner,uint8* Memory,float Delta)override;
};
// GameState exists on the server and each client; its lights are created locally
// on every peer, so rendering never depends on the server's light components.
UCLASS() class COMBATLAB_API ACombatGameState:public AGameStateBase {
 GENERATED_BODY()
public:
 ACombatGameState();
};
UCLASS() class COMBATLAB_API ACombatMode:public AGameModeBase {GENERATED_BODY() public:int32 SpawnSlot=0;ACombatMode();virtual void StartPlay()override;virtual AActor* ChoosePlayerStart_Implementation(AController* Player)override;};
