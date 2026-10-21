#if WITH_DEV_AUTOMATION_TESTS
#include "CombatTypes.h"
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/WorldSettings.h"
#include "GameFramework/GameStateBase.h"
#include "Components/DirectionalLightComponent.h"
struct FLabWorld {
 UWorld* World;
 FLabWorld(){World=UWorld::CreateWorld(EWorldType::Game,false);GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);World->CreateAISystem();World->InitializeActorsForPlay(FURL());World->GetWorldSettings()->NotifyBeginPlay();World->GetWorldSettings()->NotifyMatchStarted();}
 ~FLabWorld(){World->EndPlay(EEndPlayReason::Quit);GEngine->DestroyWorldContext(World);World->DestroyWorld(false);}
 ACombatCharacter* Character(FVector Location=FVector(0,0,200)){FActorSpawnParameters Params;Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;return World->SpawnActor<ACombatCharacter>(Location,FRotator::ZeroRotator,Params);}
};
struct FLabLatent:IAutomationLatentCommand{TFunction<bool()> Callback;explicit FLabLatent(TFunction<bool()> In):Callback(MoveTemp(In)){}virtual bool Update()override{return Callback();}};
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWeaponTest,"CombatLab.Weapon.CadenceReload",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FWeaponTest::RunTest(const FString&){auto Fixture=MakeShared<FLabWorld>();auto* C=Fixture->Character();C->ServerFire();TestEqual("shot consumes ammo",C->Ammo,29);C->ServerFire();TestEqual("cadence blocks duplicate",C->Ammo,29);C->ServerReload();TestTrue("reload state",C->Reloading);C->NextFire=0;C->ServerFire();TestEqual("reload blocks fire",C->Ammo,29);auto Started=FPlatformTime::Seconds();FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FLabLatent([this,Fixture,C,Started](){Fixture->World->Tick(LEVELTICK_All,.05f);if(FPlatformTime::Seconds()-Started<1.2)return false;TestEqual("reload refills",C->Ammo,30);TestFalse("reload exits",C->Reloading);return true;})));return true;}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAbilityTest,"CombatLab.GAS.CostDamageCue",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAbilityTest::RunTest(const FString&){FLabWorld Fixture;auto* C=Fixture.Character();auto* Target=Fixture.Character(FVector(150,0,200));TestTrue("dash activated",C->Abilities->TryActivateAbilityByClass(UDashAbility::StaticClass()));TestEqual("dash GE cost",C->Attributes->GetStamina(),75.f);TestTrue("pulse activated",C->Abilities->TryActivateAbilityByClass(UPulseAbility::StaticClass()));TestEqual("pulse GE cost",C->Attributes->GetStamina(),45.f);TestEqual("pulse GE damage",Target->Attributes->GetHealth(),80.f);C->Attributes->SetStamina(0);TestFalse("insufficient stamina rejects",C->Abilities->TryActivateAbilityByClass(UPulseAbility::StaticClass()));return true;}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAITest,"CombatLab.AI.BehaviorTransitions",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAITest::RunTest(const FString&){auto Fixture=MakeShared<FLabWorld>();auto* Enemy=Fixture->Character();auto* Target=Fixture->Character(FVector(1000,0,200));auto* AI=Fixture->World->SpawnActor<AEnemyController>();AI->Possess(Enemy);auto Step=MakeShared<int32>(0);FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FLabLatent([this,Fixture,Enemy,Target,AI,Step](){Fixture->World->Tick(LEVELTICK_All,.05f);++*Step;if(*Step==5){TestEqual("no sensed target patrols",Enemy->AIState,FName("Patrol"));AI->GetBlackboardComponent()->SetValueAsObject("Target",Target);}if(*Step==10){TestEqual("distant target chased",Enemy->AIState,FName("Chase"));Target->SetActorLocation(Enemy->GetActorLocation()+FVector(150,0,0));}if(*Step==15){TestEqual("near target attacked",Enemy->AIState,FName("Attack"));return true;}return false;})));return true;}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHealthTest,"CombatLab.GAS.HealthClamp",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FHealthTest::RunTest(const FString&){FLabWorld Fixture;auto* C=Fixture.Character();auto* T=Fixture.Character(FVector(150,0,200));for(int i=0;i<10;i++)C->ApplyHit(T);TestEqual("damage clamps at zero",T->Attributes->GetHealth(),0.f);T->ServerFire();TestEqual("dead cannot fire",T->Ammo,30);return true;}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSceneLightingTest,"CombatLab.Scene.Lighting",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FSceneLightingTest::RunTest(const FString&){
 FLabWorld Fixture;
 auto* Mode=Fixture.World->SpawnActor<ACombatMode>();
 auto* State=Fixture.World->SpawnActor<AGameStateBase>(Mode->GameStateClass);
 TArray<UDirectionalLightComponent*> Lights; State->GetComponents(Lights);
 bool HasVisibleLighting=false;
 for(auto* Light:Lights) if(Light->IsVisible()&&Light->Intensity>0) HasVisibleLighting=!State->IsHidden();
 TestTrue("normal rendered scene has visible nonzero lighting on its replicated game state",HasVisibleLighting);
 TestTrue("lighting owner replicates to network clients",State->GetIsReplicated());
 return true;
}
#endif
