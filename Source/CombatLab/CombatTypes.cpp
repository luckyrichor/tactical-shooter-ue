#include "CombatTypes.h"
#include "Engine/Engine.h"
#include "NativeGameplayTags.h"
#include "GameplayCueManager.h"
#include "GameplayCueSet.h"
#include "AbilitySystemGlobals.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "GameFramework/PlayerStart.h"
#include "HAL/PlatformMisc.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeManager.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Composites/BTComposite_Sequence.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "UObject/ConstructorHelpers.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
UE_DEFINE_GAMEPLAY_TAG(LabDashCue,"GameplayCue.Lab.Dash");
UE_DEFINE_GAMEPLAY_TAG(LabPulseCue,"GameplayCue.Lab.Pulse");
UCombatAttributes::UCombatAttributes(){InitHealth(100);InitStamina(100);}
void UCombatAttributes::OnRep_Health(const FGameplayAttributeData& Old){GAMEPLAYATTRIBUTE_REPNOTIFY(UCombatAttributes,Health,Old);}
void UCombatAttributes::OnRep_Stamina(const FGameplayAttributeData& Old){GAMEPLAYATTRIBUTE_REPNOTIFY(UCombatAttributes,Stamina,Old);}
void UCombatAttributes::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const{Super::GetLifetimeReplicatedProps(OutLifetimeProps);DOREPLIFETIME_CONDITION_NOTIFY(UCombatAttributes,Health,COND_None,REPNOTIFY_Always);DOREPLIFETIME_CONDITION_NOTIFY(UCombatAttributes,Stamina,COND_None,REPNOTIFY_Always);}
void UCombatAttributes::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data){Super::PostGameplayEffectExecute(Data);SetHealth(FMath::Clamp(GetHealth(),0.f,100.f));SetStamina(FMath::Clamp(GetStamina(),0.f,100.f));}
static void Modifier(UGameplayEffect* Effect,FGameplayAttribute Attribute,float Value){FGameplayModifierInfo M;M.Attribute=Attribute;M.ModifierOp=EGameplayModOp::Additive;M.ModifierMagnitude=FScalableFloat(Value);Effect->Modifiers.Add(M);}
static void Cue(UGameplayEffect* Effect,FGameplayTag Tag){FGameplayEffectCue C;C.GameplayCueTags.AddTag(Tag);Effect->GameplayCues.Add(C);}
UDamageEffect::UDamageEffect(){DurationPolicy=EGameplayEffectDurationType::Instant;Modifier(this,UCombatAttributes::GetHealthAttribute(),-20);}
UDashCost::UDashCost(){DurationPolicy=EGameplayEffectDurationType::Instant;Modifier(this,UCombatAttributes::GetStaminaAttribute(),-25);Cue(this,LabDashCue);}
UPulseCost::UPulseCost(){DurationPolicy=EGameplayEffectDurationType::Instant;Modifier(this,UCombatAttributes::GetStaminaAttribute(),-30);Cue(this,LabPulseCue);}
UDashAbility::UDashAbility(){InstancingPolicy=EGameplayAbilityInstancingPolicy::InstancedPerActor;NetExecutionPolicy=EGameplayAbilityNetExecutionPolicy::ServerOnly;CostGameplayEffectClass=UDashCost::StaticClass();}
void UDashAbility::ActivateAbility(const FGameplayAbilitySpecHandle H,const FGameplayAbilityActorInfo* Info,const FGameplayAbilityActivationInfo Activation,const FGameplayEventData*){TRACE_CPUPROFILER_EVENT_SCOPE(Lab_Dash);if(CommitAbility(H,Info,Activation)){if(auto* C=Cast<ACombatCharacter>(Info->AvatarActor.Get())){C->LaunchCharacter(C->GetActorForwardVector()*1200,true,false);UE_LOG(LogTemp,Display,TEXT("LAB_DASH %s stamina=%.0f"),*C->GetName(),C->Attributes->GetStamina());}}EndAbility(H,Info,Activation,true,false);}
UPulseAbility::UPulseAbility(){InstancingPolicy=EGameplayAbilityInstancingPolicy::InstancedPerActor;NetExecutionPolicy=EGameplayAbilityNetExecutionPolicy::ServerOnly;CostGameplayEffectClass=UPulseCost::StaticClass();}
void UPulseAbility::ActivateAbility(const FGameplayAbilitySpecHandle H,const FGameplayAbilityActorInfo* Info,const FGameplayAbilityActivationInfo Activation,const FGameplayEventData*){TRACE_CPUPROFILER_EVENT_SCOPE(Lab_Pulse);if(CommitAbility(H,Info,Activation)){if(auto* C=Cast<ACombatCharacter>(Info->AvatarActor.Get())){for(TActorIterator<ACombatCharacter> It(C->GetWorld());It;++It)if(*It!=C&&FVector::Dist(It->GetActorLocation(),C->GetActorLocation())<600)C->ApplyHit(*It);UE_LOG(LogTemp,Display,TEXT("LAB_PULSE %s stamina=%.0f"),*C->GetName(),C->Attributes->GetStamina());}}EndAbility(H,Info,Activation,true,false);}
UDashCue::UDashCue(){GameplayCueTag=LabDashCue;}
bool UDashCue::OnExecute_Implementation(AActor* Target,const FGameplayCueParameters&)const{if(Target){DrawDebugSphere(Target->GetWorld(),Target->GetActorLocation(),120,12,FColor::Cyan,false,.5);UE_LOG(LogTemp,Display,TEXT("LAB_CUE Dash %s"),*Target->GetName());}return true;}
UPulseCue::UPulseCue(){GameplayCueTag=LabPulseCue;}
bool UPulseCue::OnExecute_Implementation(AActor* Target,const FGameplayCueParameters&)const{if(Target){DrawDebugSphere(Target->GetWorld(),Target->GetActorLocation(),600,24,FColor::Yellow,false,.5);UE_LOG(LogTemp,Display,TEXT("LAB_CUE Pulse %s"),*Target->GetName());}return true;}
ACombatCharacter::ACombatCharacter(){PrimaryActorTick.bCanEverTick=true;bReplicates=true;Abilities=CreateDefaultSubobject<UAbilitySystemComponent>("Abilities");Abilities->SetIsReplicated(true);Abilities->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);Attributes=CreateDefaultSubobject<UCombatAttributes>("Attributes");GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);auto* Arm=CreateDefaultSubobject<USpringArmComponent>("CameraArm");Arm->SetupAttachment(RootComponent);Arm->TargetArmLength=400;Arm->bUsePawnControlRotation=true;auto* Camera=CreateDefaultSubobject<UCameraComponent>("Camera");Camera->SetupAttachment(Arm);auto* Body=CreateDefaultSubobject<UStaticMeshComponent>("Body");Body->SetupAttachment(RootComponent);static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));if(Cube.Succeeded())Body->SetStaticMesh(Cube.Object);Body->SetRelativeScale3D(FVector(.6,.6,1.5));Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);GetCharacterMovement()->MaxWalkSpeed=500;AIControllerClass=AEnemyController::StaticClass();AutoPossessAI=EAutoPossessAI::Disabled;}
void ACombatCharacter::BeginPlay(){Super::BeginPlay();auto* Set=UAbilitySystemGlobals::Get().GetGameplayCueManager()->GetRuntimeCueSet();Set->AddCues({FGameplayCueReferencePair(LabDashCue,FSoftObjectPath(UDashCue::StaticClass())),FGameplayCueReferencePair(LabPulseCue,FSoftObjectPath(UPulseCue::StaticClass()))});Abilities->InitAbilityActorInfo(this,this);if(HasAuthority()){static int32 Identity=0;LabId=++Identity;Abilities->GiveAbility(FGameplayAbilitySpec(UDashAbility::StaticClass()));Abilities->GiveAbility(FGameplayAbilitySpec(UPulseAbility::StaticClass()));}}
void ACombatCharacter::Tick(float Delta){Super::Tick(Delta);double Now=GetWorld()->GetTimeSeconds();if(FParse::Param(FCommandLine::Get(),TEXT("LabAutoPlayer"))&&IsPlayerControlled()&&IsLocallyControlled()){if(ProbeEnd==0)ProbeEnd=Now+5;if(Now<ProbeEnd)Forward(.4f);if(Now<ProbeEnd&&Now>NextProbe){NextProbe=Now+1;Fire();Pulse();Dash();}}if(Now>NextState){NextState=Now+1;UE_LOG(LogTemp,Display,TEXT("LAB_STATE net=%d id=%d actor=%s ammo=%d health=%.0f stamina=%.0f ai=%s"),(int)GetNetMode(),LabId,*GetName(),Ammo,Attributes->GetHealth(),Attributes->GetStamina(),*AIState.ToString());}float Exit=0;if(FParse::Value(FCommandLine::Get(),TEXT("LabExitSeconds="),Exit)&&Exit>0){if(HasAuthority()&&Now>Exit-2){static bool Stopped=false;if(!Stopped){Stopped=true;GEngine->Exec(GetWorld(),TEXT("csvprofile stop"));}}if(Now>Exit)FPlatformMisc::RequestExit(false);}}
void ACombatCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const{Super::GetLifetimeReplicatedProps(OutLifetimeProps);DOREPLIFETIME(ACombatCharacter,LabId);DOREPLIFETIME(ACombatCharacter,Ammo);DOREPLIFETIME(ACombatCharacter,Reloading);DOREPLIFETIME(ACombatCharacter,AIState);}
void ACombatCharacter::SetupPlayerInputComponent(UInputComponent* I){Super::SetupPlayerInputComponent(I);I->BindAxis("Forward",this,&ACombatCharacter::Forward);I->BindAxis("Right",this,&ACombatCharacter::Right);I->BindAxis("Look",this,&ACombatCharacter::Look);I->BindAxis("Pitch",this,&ACombatCharacter::Pitch);I->BindAction("Fire",IE_Pressed,this,&ACombatCharacter::Fire);I->BindAction("Reload",IE_Pressed,this,&ACombatCharacter::Reload);I->BindAction("Dash",IE_Pressed,this,&ACombatCharacter::Dash);I->BindAction("Pulse",IE_Pressed,this,&ACombatCharacter::Pulse);}
void ACombatCharacter::Forward(float V){if(Attributes->GetHealth()>0)AddMovementInput(FRotationMatrix(FRotator(0,GetControlRotation().Yaw,0)).GetUnitAxis(EAxis::X),V);}
void ACombatCharacter::Right(float V){if(Attributes->GetHealth()>0)AddMovementInput(FRotationMatrix(FRotator(0,GetControlRotation().Yaw,0)).GetUnitAxis(EAxis::Y),V);}
void ACombatCharacter::Look(float V){AddControllerYawInput(V);}void ACombatCharacter::Pitch(float V){AddControllerPitchInput(V);}
void ACombatCharacter::Fire(){ServerFire();}void ACombatCharacter::Reload(){ServerReload();}void ACombatCharacter::Dash(){ServerSkill(false);}void ACombatCharacter::Pulse(){ServerSkill(true);}
void ACombatCharacter::ApplyHit(ACombatCharacter* Target){if(!HasAuthority()||!Target)return;auto Spec=Abilities->MakeOutgoingSpec(UDamageEffect::StaticClass(),1,Abilities->MakeEffectContext());if(Spec.IsValid())Abilities->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(),Target->Abilities);}
void ACombatCharacter::ServerFire_Implementation(){TRACE_CPUPROFILER_EVENT_SCOPE(Lab_Fire);double Now=GetWorld()->GetTimeSeconds();if(Ammo<=0||Reloading||Attributes->GetHealth()<=0||Now<NextFire)return;NextFire=Now+.15;Ammo--;FVector Start;FRotator Aim;GetActorEyesViewPoint(Start,Aim);FVector End=Start+Aim.Vector()*3000;FHitResult Hit;FCollisionQueryParams Query;Query.AddIgnoredActor(this);if(GetWorld()->LineTraceSingleByChannel(Hit,Start,End,ECC_Visibility,Query)){End=Hit.ImpactPoint;if(auto* Target=Cast<ACombatCharacter>(Hit.GetActor()))ApplyHit(Target);}ShotFX(End);}
void ACombatCharacter::ShotFX_Implementation(FVector End){DrawDebugLine(GetWorld(),GetActorLocation()+FVector(0,0,50),End,FColor::Red,false,.2);}
void ACombatCharacter::ServerReload_Implementation(){if(Reloading||Ammo>=30||Attributes->GetHealth()<=0)return;Reloading=true;FTimerHandle Timer;TWeakObjectPtr<ACombatCharacter> Weak(this);GetWorldTimerManager().SetTimer(Timer,[Weak]{if(Weak.IsValid()){Weak->Ammo=30;Weak->Reloading=false;}},1,false);}
void ACombatCharacter::ServerSkill_Implementation(bool IsPulse){double Now=GetWorld()->GetTimeSeconds();double& Next=IsPulse?NextPulse:NextDash;if(Now<Next||Attributes->GetHealth()<=0)return;if(Abilities->TryActivateAbilityByClass(IsPulse?UPulseAbility::StaticClass():UDashAbility::StaticClass()))Next=Now+(IsPulse?3:2);}
AEnemyController::AEnemyController(){auto* Perception=CreateDefaultSubobject<UAIPerceptionComponent>("Perception");SetPerceptionComponent(*Perception);auto* Sight=CreateDefaultSubobject<UAISenseConfig_Sight>("Sight");Sight->SightRadius=1500;Sight->LoseSightRadius=1800;Sight->PeripheralVisionAngleDegrees=80;Sight->DetectionByAffiliation.bDetectEnemies=true;Sight->DetectionByAffiliation.bDetectNeutrals=true;Sight->DetectionByAffiliation.bDetectFriendlies=true;Perception->ConfigureSense(*Sight);Perception->SetDominantSense(Sight->GetSenseImplementation());Perception->OnTargetPerceptionUpdated.AddDynamic(this,&AEnemyController::Sensed);}
void AEnemyController::OnPossess(APawn* InPawn){Super::OnPossess(InPawn);auto* Data=NewObject<UBlackboardData>(this);FBlackboardEntry Entry;Entry.EntryName="Target";Entry.KeyType=NewObject<UBlackboardKeyType_Object>(Data);Data->Keys.Add(Entry);UBlackboardComponent* BB=nullptr;UseBlackboard(Data,BB);auto* Tree=NewObject<UBehaviorTree>(this);Tree->BlackboardAsset=Data;auto* Root=NewObject<UBTComposite_Sequence>(Tree);Tree->RootNode=Root;FBTCompositeChild Child;Child.ChildTask=NewObject<UEnemyTask>(Tree);Root->Children.Add(Child);bool Started=RunBehaviorTree(Tree);auto* BT=Cast<UBehaviorTreeComponent>(BrainComponent);UE_LOG(LogTemp,Display,TEXT("LAB_BT started=%d manager=%d running=%d ticking=%d"),Started,UBehaviorTreeManager::GetCurrent(GetWorld())!=nullptr,BT?BT->IsRunning():false,BT?BT->IsComponentTickEnabled():false);}
void AEnemyController::Sensed(AActor* Actor,FAIStimulus S){auto* TargetCharacter=Cast<ACombatCharacter>(Actor);if(!GetBlackboardComponent()||!TargetCharacter||!TargetCharacter->IsPlayerControlled())return;if(S.WasSuccessfullySensed())GetBlackboardComponent()->SetValueAsObject("Target",Actor);else if(GetBlackboardComponent()->GetValueAsObject("Target")==Actor)GetBlackboardComponent()->ClearValue("Target");}
UEnemyTask::UEnemyTask(){NodeName="Patrol Chase Attack";bNotifyTick=true;}
EBTNodeResult::Type UEnemyTask::ExecuteTask(UBehaviorTreeComponent&,uint8*){return EBTNodeResult::InProgress;}
void UEnemyTask::TickTask(UBehaviorTreeComponent& Owner,uint8*,float){TRACE_CPUPROFILER_EVENT_SCOPE(Lab_AI);auto* AI=Cast<AEnemyController>(Owner.GetAIOwner());auto* C=AI?Cast<ACombatCharacter>(AI->GetPawn()):nullptr;if(!C)return;auto* Target=Cast<ACombatCharacter>(AI->GetBlackboardComponent()->GetValueAsObject("Target"));if(!Target||Target->Attributes->GetHealth()<=0){C->AIState="Patrol";C->AddMovementInput(FVector(0,FMath::Sin(C->GetWorld()->GetTimeSeconds()),0));return;}FVector Direction=Target->GetActorLocation()-C->GetActorLocation();AI->SetControlRotation(Direction.Rotation());if(Direction.Size()>350){C->AIState="Chase";C->AddMovementInput(Direction.GetSafeNormal());}else{C->AIState="Attack";C->ServerFire();}}
ACombatGameState::ACombatGameState(){
 SetActorHiddenInGame(false);
 RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
 auto* Sun=CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("Sun"));
 Sun->SetupAttachment(RootComponent);
 Sun->SetMobility(EComponentMobility::Movable);
 Sun->SetRelativeRotation(FRotator(-45,-35,0));
 Sun->SetIntensity(3.f);
 auto* Fill=CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("Fill"));
 Fill->SetupAttachment(RootComponent);
 Fill->SetMobility(EComponentMobility::Movable);
 Fill->SetRelativeRotation(FRotator(-30,145,0));
 Fill->SetIntensity(.6f);
 Fill->SetCastShadows(false);
}
ACombatMode::ACombatMode(){DefaultPawnClass=ACombatCharacter::StaticClass();GameStateClass=ACombatGameState::StaticClass();}
void ACombatMode::StartPlay(){Super::StartPlay();auto* Set=UAbilitySystemGlobals::Get().GetGameplayCueManager()->GetRuntimeCueSet();Set->AddCues({FGameplayCueReferencePair(LabDashCue,FSoftObjectPath(UDashCue::StaticClass())),FGameplayCueReferencePair(LabPulseCue,FSoftObjectPath(UPulseCue::StaticClass()))});auto* Floor=GetWorld()->SpawnActor<AStaticMeshActor>(FVector(0,0,-80),FRotator::ZeroRotator);Floor->SetReplicates(true);Floor->SetReplicateMovement(true);Floor->GetStaticMeshComponent()->SetIsReplicated(true);Floor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);Floor->GetStaticMeshComponent()->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));Floor->SetActorScale3D(FVector(40,40,1));Floor->GetStaticMeshComponent()->SetCollisionProfileName("BlockAll");auto* Enemy=GetWorld()->SpawnActor<ACombatCharacter>(FVector(FParse::Param(FCommandLine::Get(),TEXT("LabNetworkProbe"))?5000:700,0,120),FRotator(0,180,0));if(Enemy){Enemy->AIState="Patrol";auto* AI=GetWorld()->SpawnActor<AEnemyController>();AI->Possess(Enemy);}UE_LOG(LogTemp,Display,TEXT("LAB_STARTED authority=%d"),HasAuthority());}

AActor* ACombatMode::ChoosePlayerStart_Implementation(AController*){return GetWorld()->SpawnActor<APlayerStart>(FVector(-500,SpawnSlot++*180,120),FRotator::ZeroRotator);}
