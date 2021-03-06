#include "IA.h"
#include "j1App.h"
#include "j1TutorialScene.h"
#include "PugiXml/src/pugixml.hpp"
#include "CombatUnit.h"

IA::IA() : enemyFortress(nullptr)
{
	gamePhase = GameBehaviour::EARLY;
	early = EarlyGameBehaviour::CREATION;
	mid = MidGameBehaviour::ASSEMBLE;
	late = LateGameBehaviour::ATACK;
	numWaves = 1;

	civilization = CivilizationType::GREEK;

	positionViking.push_back({ -384,672 }); //Monastery
	positionViking.push_back({ 224,688 }); //Temple
	positionViking.push_back({ 160,816 }); //Cleric1
	positionViking.push_back({ 224,848 }); //Cleric2
	positionViking.push_back({ -224,624 }); //Monk1
	positionViking.push_back({ -384,736 }); //Monk2
	positionViking.push_back({ -352,752 }); //Monk3
	positionViking.push_back({ -3904,2464 }); //Cleric1Pos
	positionViking.push_back({ 3200,2304 }); //Cleric2Pos
	positionViking.push_back({ -192,4448 }); //Cleric3Pos
	positionViking.push_back({ 64,864 }); //Home

	positionGreek.push_back({ -576,4256 }); //Monastery
	positionGreek.push_back({ 64,4224 }); //Temple
	positionGreek.push_back({ -256,4416 }); //Cleric1
	positionGreek.push_back({ -864,4272 }); //Cleric2
	positionGreek.push_back({ -576,4192 }); //Monk1
	positionGreek.push_back({ -640,4288 }); //Monk2
	positionGreek.push_back({ -608,4304 }); //Monk3
	positionGreek.push_back({ -3904,2464 }); //Cleric1Pos
	positionGreek.push_back({ 3200,2304 }); //Cleric2Pos
	positionGreek.push_back({ -128,576 }); //Cleric3Pos
	positionGreek.push_back({ -192,4352 }); //Home


	positionViking.push_back({ -704,800 }); //encampment
	positionViking.push_back({ -800,848 }); //assassin1
	positionViking.push_back({ -672,912 }); //assassin2
	positionViking.push_back({ -96,976 }); //assassin3
	positionViking.push_back({ 32,976 }); //assassin4
	positionViking.push_back({ 0,1120 }); //assassin5
	positionViking.push_back({ 480,848 }); //assassin6
	positionViking.push_back({ 640,832 }); //assassin7

	positionGreek.push_back({ -32,4048 }); //encampment
	positionGreek.push_back({ -256,3840 }); //assassin1
	positionGreek.push_back({ -160,3792 }); //assassin2
	positionGreek.push_back({ -64,3840 }); //assassin3
	positionGreek.push_back({ 512,4256 }); //assassin4
	positionGreek.push_back({ -992,4240 }); //assassin5
	positionGreek.push_back({ -960,4128 }); //assassin6
	positionGreek.push_back({ -160,3888 }); //assassin7

	positionGreek.push_back({ -64,4416 }); //monastery 2
	positionGreek.push_back({ 32,4464 }); //monk4
	positionGreek.push_back({ 64,4448 }); //monk5

	positionViking.push_back({ -224,560 }); //monastery 2
	positionViking.push_back({ -256,576 }); //monk4
	positionViking.push_back({ -224,592 }); //monk5

	timer.Start();

	loading = false;
}

void IA::Init()
{
	active = false;
}

bool IA::Start()
{
	bool ret = true;

	timer_ia.Start();
	time_ia = 684;
	timer.Start();

	return ret;
}

bool IA::PreUpdate()
{
	bool ret = true;

	if (App->input->GetMouseButtonDown(1))
	{
		mouse = App->map->GetMousePositionOnMap();
		mouse = App->map->TileCenterPoint(mouse);
		//LOG("%i %i", mouse.x, mouse.y);
	}

	return ret;
}

bool IA::Update(float dt)
{
	bool ret = true;

	if (App->scene->paused_game == true && timer.isPaused() == false) {
		timer.Pause();
		timer_ia.Pause();
	}
	else if (App->scene->paused_game == false && timer.isPaused() == true) {
		timer.Resume();
		timer_ia.Resume();
	}

	switch (gamePhase)
	{
	case GameBehaviour::EARLY:
		EarlyGame();
		break;
	case GameBehaviour::MID:
		MidGame();
		break;
	case GameBehaviour::LATE:
		LateGame();
		break;
	default:
		break;
	}

	return ret;
}

bool IA::PostUpdate()
{
	bool ret = true;
	SetPathAgain();
	return ret;
}

bool IA::CleanUp()
{
	bool ret = true;
	listEntities.clear();
	gamePhase = GameBehaviour::EARLY;
	early = EarlyGameBehaviour::CREATION;
	mid = MidGameBehaviour::ASSEMBLE;
	late = LateGameBehaviour::ATACK;
	return ret;
}

bool IA::Load(pugi::xml_node& s)
{
	listEntities.clear();
	loading = true;
	pugi::xml_node node = s.child("IA").child("Game_Phase");
	int macroState = node.attribute("macroState").as_int();
	int state = node.attribute("state").as_int();
	DoThingsBefore(macroState, state);
	switch (macroState)
	{
	case 0:
		gamePhase = GameBehaviour::EARLY;
		switch (state) {
		case 1:
			early = EarlyGameBehaviour::BASIC_BUILDINGS_CREATION;
			break;
		case 2:
			early = EarlyGameBehaviour::RESEARCH_CLERIC;
			break;
		case 3:
			early = EarlyGameBehaviour::BASIC_UNITS_CREATION;
			break;
		case 4:
			early = EarlyGameBehaviour::EXPLORE1;
			break;
		case 5:
			early = EarlyGameBehaviour::CHECKEXPLORER1;
			Explore2();
			break;
		case 6:
			early = EarlyGameBehaviour::FIND;
			break;
		default:
			break;
		}
		break;
	case 1:
		gamePhase = GameBehaviour::MID;
		switch (state) {
		case 0:
			mid = MidGameBehaviour::ASSEMBLE;
			break;
		case 1:
			mid = MidGameBehaviour::CREATE_ECONOMY;
			break;
		case 2:
			mid = MidGameBehaviour::RESEARCH_ASSASSIN;
			break;
		case 3:
			mid = MidGameBehaviour::CREATE_ASSASSIN;
			break;
		default:
			break;
		}
		break;
	case 2:
		gamePhase = GameBehaviour::LATE;
		switch (state) {
		case 0:
			late = LateGameBehaviour::ATACK;
			break;
		case 1:
			late = LateGameBehaviour::ECONOMY_FOCUS;
			break;
		case 2:
			late = LateGameBehaviour::DEFENSE;
			break;
		case 3:
			late = LateGameBehaviour::ATACK2;
			break;
		case 4:
			late = LateGameBehaviour::WIN;
			break;
		case 5:
			late = LateGameBehaviour::FINISH;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	timer.StartAt(node.attribute("time").as_int());

	loading = false;
	return true;
}

bool IA::Save(pugi::xml_node& s) const
{

	pugi::xml_node node = s.append_child("IA");
	pugi::xml_node gameph = s.child("IA").append_child("Game_Phase");
	gameph.append_attribute("macroState").set_value((int)gamePhase);
	if (gamePhase == GameBehaviour::EARLY)
	{
		gameph.append_attribute("state").set_value((int)early);
	}
	else if (gamePhase == GameBehaviour::MID)
	{
		gameph.append_attribute("state").set_value((int)mid);
	}
	else if (gamePhase == GameBehaviour::LATE)
	{
		gameph.append_attribute("state").set_value((int)late);
	}

	gameph.append_attribute("time").set_value(timer_ia.ReadSec());

	return true;
}

void IA::EarlyGame()
{
	switch (early)
	{
	case EarlyGameBehaviour::CREATION:
		InitCiv();
		break;
	case EarlyGameBehaviour::BASIC_BUILDINGS_CREATION:
		if (timer.ReadSec() >= 5 || loading)
		{
			if (civilization == CivilizationType::VIKING)
			{
				CreateBuilding(BuildingType::MONASTERY, positionViking.at((int)EarlyMovements::MONASTERY));
				CreateBuilding(BuildingType::TEMPLE, positionViking.at((int)EarlyMovements::TEMPLE));
			}
			else
			{
				CreateBuilding(BuildingType::MONASTERY, positionGreek.at((int)EarlyMovements::MONASTERY));
				CreateBuilding(BuildingType::TEMPLE, positionGreek.at((int)EarlyMovements::TEMPLE));
			}
			early = EarlyGameBehaviour::RESEARCH_CLERIC;
			timer.Start();
		}
		break;
	case EarlyGameBehaviour::RESEARCH_CLERIC:
		if (timer.ReadSec() >= 5 || loading)
		{
			if (civilization == CivilizationType::VIKING)
			{
				if (!loading) MoveUnit(positionViking.at((int)EarlyMovements::MONK1), "monk");
				listEntities.push_back(static_cast<Entity*>(CreateUnit(UnitType::MONK, positionViking.at((int)EarlyMovements::MONK2))));
				listEntities.push_back(static_cast<Entity*>(CreateUnit(UnitType::MONK, positionViking.at((int)EarlyMovements::MONK3))));
			}
			else
			{
				if (!loading) MoveUnit(positionGreek.at((int)EarlyMovements::MONK1), "monk");
				listEntities.push_back(static_cast<Entity*>(CreateUnit(UnitType::MONK, positionGreek.at((int)EarlyMovements::MONK2))));
				listEntities.push_back(static_cast<Entity*>(CreateUnit(UnitType::MONK, positionGreek.at((int)EarlyMovements::MONK3))));
			}
			early = EarlyGameBehaviour::BASIC_UNITS_CREATION;
			timer.Start();
		}
		break;
	case EarlyGameBehaviour::BASIC_UNITS_CREATION:
		if (loading)
		{
			mid = MidGameBehaviour::CREATE_ECONOMY; 
			gamePhase = GameBehaviour::MID; 
			break;
		}
		if (timer.ReadSec() >= 10 || loading)
		{
			if (civilization == CivilizationType::VIKING)
			{
				listEntities.push_back(static_cast<Entity*>(CreateUnit(UnitType::CLERIC, positionViking.at((int)EarlyMovements::CLERIC2))));
				listEntities.push_back(static_cast<Entity*>(CreateUnit(UnitType::CLERIC, positionViking.at((int)EarlyMovements::CLERIC1))));
			}
			else
			{
				listEntities.push_back(static_cast<Entity*>(CreateUnit(UnitType::CLERIC, positionGreek.at((int)EarlyMovements::CLERIC1))));
				listEntities.push_back(static_cast<Entity*>(CreateUnit(UnitType::CLERIC, positionGreek.at((int)EarlyMovements::CLERIC2))));
			}
			early = EarlyGameBehaviour::EXPLORE1;
			timer.Start();
		}
		break;
	case EarlyGameBehaviour::EXPLORE1:
		if (loading)
		{
			mid = MidGameBehaviour::CREATE_ECONOMY; 
			gamePhase = GameBehaviour::MID; 
			break;
		}
		Explore1();
		early = EarlyGameBehaviour::CHECKEXPLORER1;
		break;
	case EarlyGameBehaviour::CHECKEXPLORER1:
		if (loading) {
			mid = MidGameBehaviour::CREATE_ECONOMY; 
			gamePhase = GameBehaviour::MID; 
			break;
		}
		if (CheckExplore())
		{
			Explore2();
			early = EarlyGameBehaviour::FIND;
			timer.Start();
		}
		break;
	case EarlyGameBehaviour::FIND:
		if (loading)
		{
			mid = MidGameBehaviour::CREATE_ECONOMY; 
			gamePhase = GameBehaviour::MID; 
			break;
		}
		if (Find())
			gamePhase = GameBehaviour::MID;
		break;
	default:
		break;
	}
}

void IA::MidGame()
{
	switch (mid)
	{
	case MidGameBehaviour::ASSEMBLE:
		if (loading)
		{
			mid = MidGameBehaviour::CREATE_ECONOMY; 
			break;
		}
		AssembleClerics();
		mid = MidGameBehaviour::CREATE_ECONOMY;
		timer.Start();
		break;
	case MidGameBehaviour::CREATE_ECONOMY:
		if (timer.ReadSec() >= 10 || loading)
		{
			if (civilization == CivilizationType::VIKING) {
				CreateBuilding(BuildingType::MONASTERY, positionViking.at((int)EarlyMovements::MONASTERY2));
				listEntities.push_back(static_cast<Entity*>(CreateUnit(UnitType::MONK, positionViking.at((int)EarlyMovements::MONK4))));
				listEntities.push_back(static_cast<Entity*>(CreateUnit(UnitType::MONK, positionViking.at((int)EarlyMovements::MONK5))));

			}
			else {
				CreateBuilding(BuildingType::MONASTERY, positionGreek.at((int)EarlyMovements::MONASTERY2));
				listEntities.push_back(static_cast<Entity*>(CreateUnit(UnitType::MONK, positionGreek.at((int)EarlyMovements::MONK4))));
				listEntities.push_back(static_cast<Entity*>(CreateUnit(UnitType::MONK, positionGreek.at((int)EarlyMovements::MONK5))));
			}
			timer.Start();
			mid = MidGameBehaviour::RESEARCH_ASSASSIN;
		}
		break;
	case MidGameBehaviour::RESEARCH_ASSASSIN:
		if (timer.ReadSec() >= 5 || loading)
		{
			if (civilization == CivilizationType::VIKING)
				CreateBuilding(BuildingType::ENCAMPMENT, positionViking.at((int)EarlyMovements::ENCAMPMENT));
			else
				CreateBuilding(BuildingType::ENCAMPMENT, positionGreek.at((int)EarlyMovements::ENCAMPMENT));

			mid = MidGameBehaviour::CREATE_ASSASSIN;
			timer.Start();
		}
		break;
	case MidGameBehaviour::CREATE_ASSASSIN:
		if (timer.ReadSec() >= 5 || loading)
		{
			if (civilization == CivilizationType::VIKING) {
				listEntities.push_back(static_cast<Entity*>(CreateUnit(UnitType::ASSASSIN, positionViking.at((int)EarlyMovements::ASSASSIN1))));
				listEntities.push_back(static_cast<Entity*>(CreateUnit(UnitType::ASSASSIN, positionViking.at((int)EarlyMovements::ASSASSIN1))));
				listEntities.push_back(static_cast<Entity*>(CreateUnit(UnitType::ASSASSIN, positionViking.at((int)EarlyMovements::ASSASSIN2))));
				listEntities.push_back(static_cast<Entity*>(CreateUnit(UnitType::ASSASSIN, positionViking.at((int)EarlyMovements::ASSASSIN2))));
				listEntities.push_back(static_cast<Entity*>(CreateUnit(UnitType::ASSASSIN, positionViking.at((int)EarlyMovements::ASSASSIN3))));
				listEntities.push_back(static_cast<Entity*>(CreateUnit(UnitType::ASSASSIN, positionViking.at((int)EarlyMovements::ASSASSIN3))));
				listEntities.push_back(static_cast<Entity*>(CreateUnit(UnitType::ASSASSIN, positionViking.at((int)EarlyMovements::ASSASSIN4))));
				listEntities.push_back(static_cast<Entity*>(CreateUnit(UnitType::ASSASSIN, positionViking.at((int)EarlyMovements::ASSASSIN4))));
				listEntities.push_back(static_cast<Entity*>(CreateUnit(UnitType::ASSASSIN, positionViking.at((int)EarlyMovements::ASSASSIN5))));
				listEntities.push_back(static_cast<Entity*>(CreateUnit(UnitType::ASSASSIN, positionViking.at((int)EarlyMovements::ASSASSIN5))));
				listEntities.push_back(static_cast<Entity*>(CreateUnit(UnitType::ASSASSIN, positionViking.at((int)EarlyMovements::ASSASSIN6))));
				listEntities.push_back(static_cast<Entity*>(CreateUnit(UnitType::ASSASSIN, positionViking.at((int)EarlyMovements::ASSASSIN6))));
				listEntities.push_back(static_cast<Entity*>(CreateUnit(UnitType::ASSASSIN, positionViking.at((int)EarlyMovements::ASSASSIN7))));
				listEntities.push_back(static_cast<Entity*>(CreateUnit(UnitType::ASSASSIN, positionViking.at((int)EarlyMovements::ASSASSIN7))));
			}
			else {
				listEntities.push_back(static_cast<Entity*>(CreateUnit(UnitType::ASSASSIN, positionGreek.at((int)EarlyMovements::ASSASSIN1))));
				listEntities.push_back(static_cast<Entity*>(CreateUnit(UnitType::ASSASSIN, positionGreek.at((int)EarlyMovements::ASSASSIN1))));
				listEntities.push_back(static_cast<Entity*>(CreateUnit(UnitType::ASSASSIN, positionGreek.at((int)EarlyMovements::ASSASSIN2))));
				listEntities.push_back(static_cast<Entity*>(CreateUnit(UnitType::ASSASSIN, positionGreek.at((int)EarlyMovements::ASSASSIN2))));
				listEntities.push_back(static_cast<Entity*>(CreateUnit(UnitType::ASSASSIN, positionGreek.at((int)EarlyMovements::ASSASSIN3))));
				listEntities.push_back(static_cast<Entity*>(CreateUnit(UnitType::ASSASSIN, positionGreek.at((int)EarlyMovements::ASSASSIN3))));
				listEntities.push_back(static_cast<Entity*>(CreateUnit(UnitType::ASSASSIN, positionGreek.at((int)EarlyMovements::ASSASSIN4))));
				listEntities.push_back(static_cast<Entity*>(CreateUnit(UnitType::ASSASSIN, positionGreek.at((int)EarlyMovements::ASSASSIN4))));
				listEntities.push_back(static_cast<Entity*>(CreateUnit(UnitType::ASSASSIN, positionGreek.at((int)EarlyMovements::ASSASSIN5))));
				listEntities.push_back(static_cast<Entity*>(CreateUnit(UnitType::ASSASSIN, positionGreek.at((int)EarlyMovements::ASSASSIN5))));
				listEntities.push_back(static_cast<Entity*>(CreateUnit(UnitType::ASSASSIN, positionGreek.at((int)EarlyMovements::ASSASSIN6))));
				listEntities.push_back(static_cast<Entity*>(CreateUnit(UnitType::ASSASSIN, positionGreek.at((int)EarlyMovements::ASSASSIN6))));
				listEntities.push_back(static_cast<Entity*>(CreateUnit(UnitType::ASSASSIN, positionGreek.at((int)EarlyMovements::ASSASSIN7))));
				listEntities.push_back(static_cast<Entity*>(CreateUnit(UnitType::ASSASSIN, positionGreek.at((int)EarlyMovements::ASSASSIN7))));
			}
			gamePhase = GameBehaviour::LATE;
			timer.Start();
		}
		break;
	default:
		break;
	}
}

void IA::LateGame()
{
	switch (late)
	{
	case LateGameBehaviour::ATACK:
		Atack();
		late = LateGameBehaviour::ECONOMY_FOCUS;
		timer.Start();
		break;
	case LateGameBehaviour::ECONOMY_FOCUS:
		if (timer.ReadSec() >= 20 || loading)
		{
			late = LateGameBehaviour::DEFENSE;
		}
		break;
	case LateGameBehaviour::DEFENSE:
		Defense();
		timer.Start();
		late = LateGameBehaviour::ATACK2;
		break;
	case LateGameBehaviour::ATACK2:
		if (timer.ReadSec() >= 40)
		{
			CreateAtack();
			timer.Start();
			if (numWaves == 7)
			{
				late = LateGameBehaviour::WIN;
				timerWaves_win.Start();
			}
		}
		break;
	case LateGameBehaviour::WIN:
		if (timer.ReadSec() >= 400)
		{
			Win();
		}
		if (timerWaves_win.ReadSec() >= 40)
		{
			CreateAtack();
			timerWaves_win.Start();
		}
		break;
	case LateGameBehaviour::FINISH:
		break;
	case LateGameBehaviour::CREATE_MONKS:
		CreateMonks();
		break;
	default:
		break;
	}
}



bool IA::InitCiv()
{
	if (App->entityManager->playerCreated)
	{
		//CIVILIZATION
		if (App->entityManager->getPlayer()->civilization == CivilizationType::VIKING)
		{
			civilization = CivilizationType::GREEK;
		}
		else
		{
			civilization = CivilizationType::VIKING;
		}

		App->entityManager->BuildCivilizations(civilization);

		std::list<Entity*>::iterator it = App->entityManager->entities[EntityType::UNIT].begin();
		for (it; it != App->entityManager->entities[EntityType::UNIT].end(); ++it)
		{
			if (it._Ptr->_Myval->civilization == civilization)
			{
				listEntities.push_back(it._Ptr->_Myval);
			}
		}

		std::list<Entity*>::iterator it2 = App->entityManager->entities[EntityType::BUILDING].begin();
		for (it2; it2 != App->entityManager->entities[EntityType::BUILDING].end(); ++it2)
		{
			if (it2._Ptr->_Myval->name == "fortress")
			{
				if (it2._Ptr->_Myval->civilization != civilization) enemyFortress = (Building*)it2._Ptr->_Myval;
			}
		}


		timer.Start();
		if (!App->tutorialscene->active)
		{
			early = EarlyGameBehaviour::BASIC_BUILDINGS_CREATION;
			gamePhase = GameBehaviour::EARLY;
		}
		else
		{
			gamePhase = GameBehaviour::LATE;
			late = LateGameBehaviour::CREATE_MONKS;
		}
	}

	return true;
}

bool IA::CreateBuilding(BuildingType type, iPoint pos)
{
	if (type == BuildingType::MONASTERY)
	{
		if(civilization==CivilizationType::GREEK)
			App->entityManager->CreateBuildingEntity(pos, type, App->entityManager->buildingsData[5], civilization);
		else
			App->entityManager->CreateBuildingEntity(pos, type, App->entityManager->buildingsData[1], civilization);
	}
	else if (type == BuildingType::TEMPLE)
	{
		if (civilization == CivilizationType::GREEK)
			App->entityManager->CreateBuildingEntity(pos, type, App->entityManager->buildingsData[6], civilization);
		else
			App->entityManager->CreateBuildingEntity(pos, type, App->entityManager->buildingsData[2], civilization);
	}
	else if (type == BuildingType::ENCAMPMENT)
	{
		if (civilization == CivilizationType::GREEK)
			App->entityManager->CreateBuildingEntity(pos, type, App->entityManager->buildingsData[7], civilization);
		else
			App->entityManager->CreateBuildingEntity(pos, type, App->entityManager->buildingsData[3], civilization);
	}

	return true;
}


Unit* IA::CreateUnit(UnitType type, iPoint pos)
{
	Entity* entity = App->entityManager->CreateUnitEntity(type, pos, civilization);

	return static_cast<Unit*>(entity);
}

bool IA::CheckExplore()
{
	bool ret = false;

	int units = 0;

	int i = 0;
	std::list<Entity*>::iterator it = listEntities.begin();
	for (it; it != listEntities.end(); ++it)
	{
		if (it._Ptr->_Myval->name == "cleric")
		{
			if (it._Ptr->_Myval->position.DistanceManhattan(positionViking.at((int)EarlyMovements::CLERIC1POS + i)))
			{
				units++;
			}
			i++;
		}
	}
	if (units == 2) {
		ret = true;
	}

	return ret;
}

bool IA::Find()
{
	bool ret = false;

	std::list<Entity*>::iterator it = listEntities.begin();
	for (it; it != listEntities.end(); ++it)
	{
		if (it._Ptr->_Myval->name == "cleric")
		{
			if (it._Ptr->_Myval->position.DistanceManhattan(enemyFortress->position)<=800)
			{
				ret = true;
			}
		}
	}
	if (timer.ReadSec() <= 60)
		ret = true;

	return ret;
}

bool IA::Defense()
{
	Entity* assassin1;
	Entity* assassin2;
	Entity* assassin3;
	if (civilization == CivilizationType::VIKING) {
		assassin1 = CreateUnit(UnitType::ASSASSIN, positionViking.at((int)EarlyMovements::ASSASSIN1));
		assassin2 = CreateUnit(UnitType::ASSASSIN, positionViking.at((int)EarlyMovements::ASSASSIN1));
		assassin3 = CreateUnit(UnitType::ASSASSIN, positionViking.at((int)EarlyMovements::ASSASSIN1));
		listEntities.push_back(assassin1);
		listEntities.push_back(assassin2);
		listEntities.push_back(assassin3);
		//atackUnits.push_back(assassin1);
		//atackUnits.push_back(assassin2);
		//atackUnits.push_back(assassin3);
		listEntities.push_back(static_cast<Entity*>(CreateUnit(UnitType::ASSASSIN, positionViking.at((int)EarlyMovements::ASSASSIN2))));

	}
	else {
		assassin1 = CreateUnit(UnitType::ASSASSIN, positionGreek.at((int)EarlyMovements::ASSASSIN1));
		assassin2 = CreateUnit(UnitType::ASSASSIN, positionGreek.at((int)EarlyMovements::ASSASSIN1));
		assassin3 = CreateUnit(UnitType::ASSASSIN, positionGreek.at((int)EarlyMovements::ASSASSIN1));
		listEntities.push_back(assassin1);
		listEntities.push_back(assassin2);
		listEntities.push_back(assassin3);
		//atackUnits.push_back(assassin1);
		//atackUnits.push_back(assassin2);
		//atackUnits.push_back(assassin3);
		listEntities.push_back(static_cast<Entity*>(CreateUnit(UnitType::ASSASSIN, positionGreek.at((int)EarlyMovements::ASSASSIN2))));
	}
	return true;
}

bool IA::Atack()
{
	iPoint pos = { (int)enemyFortress->position.x, (int)enemyFortress->position.y };

	MoveUnit(pos, "assassin");
	MoveUnit(pos, "assassin", nullptr, 1);
	MoveUnit(pos, "assassin", nullptr, 2);
	return true;
}

bool IA::CreateAtack()
{
	iPoint pos = { (int)enemyFortress->position.x, (int)enemyFortress->position.y };
	Unit* assassin1;
	Unit* assassin2;
	Unit* assassin3;

	if (civilization == CivilizationType::VIKING)
	{
		assassin1 = CreateUnit(UnitType::ASSASSIN, positionViking.at((int)EarlyMovements::ASSASSIN3));
		assassin2 = CreateUnit(UnitType::ASSASSIN, positionViking.at((int)EarlyMovements::ASSASSIN3));
		assassin3 = CreateUnit(UnitType::ASSASSIN, positionViking.at((int)EarlyMovements::ASSASSIN4));
	}
	else
	{
		assassin1 = CreateUnit(UnitType::ASSASSIN, positionGreek.at((int)EarlyMovements::ASSASSIN1));
		assassin2 = CreateUnit(UnitType::ASSASSIN, positionGreek.at((int)EarlyMovements::ASSASSIN2));
		assassin3 = CreateUnit(UnitType::ASSASSIN, positionGreek.at((int)EarlyMovements::ASSASSIN2));
	}
	listEntities.push_back(assassin1);
	listEntities.push_back(assassin2);
	listEntities.push_back(assassin3);
	atackUnits.push_back(assassin1);
	atackUnits.push_back(assassin2);
	atackUnits.push_back(assassin3);

	MoveUnit(pos, "assassin", assassin1);
	MoveUnit(pos, "assassin", assassin2);
	MoveUnit(pos, "assassin", assassin3);
	numWaves++;
	return true;
}

bool IA::Win()
{
	App->entityManager->getPlayer()->player_lose = true;
	CleanUp();
	return true;
}

bool IA::MoveUnit(iPoint pos, std::string name, Unit* u, int number)
{
	std::list<Entity*>::iterator it = listEntities.begin();
	std::list<Entity*> entities;
	int n = number;
	if (u == nullptr)
	{
		for (it; it != listEntities.end(); ++it)
		{
			if (it._Ptr->_Myval->name == name)
			{
				if (n > 0) {
					--n;
					continue;
				}
				iPoint origin = App->map->WorldToMap((int)it._Ptr->_Myval->position.x, (int)it._Ptr->_Myval->position.y);
				iPoint ending = App->map->WorldToMap(pos.x, pos.y);
				entities.push_back(it._Ptr->_Myval);
				Unit* unit = static_cast<Unit*>(it._Ptr->_Myval);
				if (!loading)
					App->pathfinding->RequestPath(origin, ending, entities);
				else {
					App->entityManager->CreateUnitEntity(unit->unitType, ending, unit->civilization);
					App->entityManager->DeleteEntity(it._Ptr->_Myval);
				}
				break;
			}
			entities.clear();
		}
	}
	else
	{
		for (it; it != listEntities.end(); ++it)
		{
			if (it._Ptr->_Myval == u)
			{
				iPoint origin = App->map->WorldToMap((int)it._Ptr->_Myval->position.x, (int)it._Ptr->_Myval->position.y);
				iPoint ending = App->map->WorldToMap(pos.x, pos.y);
				entities.push_back(it._Ptr->_Myval);
				Unit* unit = static_cast<Unit*>(it._Ptr->_Myval);
				if (!loading)
					App->pathfinding->RequestPath(origin, ending, entities);
				else {
					App->entityManager->CreateUnitEntity(unit->unitType, ending, unit->civilization);
					App->entityManager->DeleteEntity(it._Ptr->_Myval);
				}
				break;
			}
			entities.clear();
		}
	}

	return true;
}

void IA::DoThingsBefore(int macro, int state)
{
	int maxStates[3] = { 7,4,6 };
	for (int i = 0; i < macro; i++)
	{
		for (int j = 0; j <= maxStates[i]; ++j)
		{
			if (i == 0)
			{
				early = (EarlyGameBehaviour)j;
				EarlyGame();
			}
			else if (i == 1)
			{
				mid = (MidGameBehaviour)j;
				MidGame();
			}
		}
	}

	for (int i = 0; i < state; i++)
	{
		if (macro == 0)
		{
			early = (EarlyGameBehaviour)i;
			EarlyGame();
		}
		else if (macro == 1)
		{
			mid = (MidGameBehaviour)i;
			MidGame();
		}
		else if (macro == 2)
		{
			late = (LateGameBehaviour)i;
			LateGame();
		}
	}
}


void IA::Explore1()
{
	int i = 0;
	std::list<Entity*>::iterator it = listEntities.begin();
	for (it; it != listEntities.end(); ++it)
	{
		if (it._Ptr->_Myval->name == "cleric")
		{
			Unit* u = static_cast<Unit*>(it._Ptr->_Myval);
			if (civilization == CivilizationType::VIKING)
				MoveUnit(positionViking.at((int)EarlyMovements::CLERIC1POS + i), "cleric", u);
			else
				MoveUnit(positionGreek.at((int)EarlyMovements::CLERIC1POS + i), "cleric", u);
			i++;
		}
	}
}

void IA::Explore2()
{
	std::list<Entity*>::iterator it = listEntities.begin();
	for (it; it != listEntities.end(); ++it)
	{
		if (it._Ptr->_Myval->name == "cleric")
		{
			Unit* u = static_cast<Unit*>(it._Ptr->_Myval);
			if (civilization == CivilizationType::VIKING)
				MoveUnit(positionViking.at((int)EarlyMovements::CLERIC3POS), "cleric", u);
			else
				MoveUnit(positionGreek.at((int)EarlyMovements::CLERIC3POS), "cleric", u);
		}
	}
}

void IA::AssembleClerics()
{
	std::list<Entity*>::iterator it = listEntities.begin();
	for (it; it != listEntities.end(); ++it)
	{
		if (it._Ptr->_Myval->name == "cleric")
		{
			Unit* u = static_cast<Unit*>(it._Ptr->_Myval);
			if (civilization == CivilizationType::VIKING)
				MoveUnit(positionViking.at((int)EarlyMovements::HOME), "cleric", u);
			else
				MoveUnit(positionGreek.at((int)EarlyMovements::HOME), "cleric", u);
		}
	}
}

void IA::CreateMonks()
{
	if (timer.ReadSec() >= 20)
	{
		timer.Start();
		App->entityManager->CreateUnitEntity(UnitType::MONK, {32,2672}, civilization);
	}
}

void IA::SetPathAgain()
{
	iPoint fortress = { (int)enemyFortress->position.x, (int)enemyFortress->position.y };
	std::list<Entity*>::iterator it = atackUnits.begin();
	for (it; it != atackUnits.end(); ++it)
	{
		CombatUnit* unit = static_cast<CombatUnit*>(it._Ptr->_Myval);
		if (unit->enemyTarget == nullptr && unit->entPath.size() == 0 && unit->GetState()==AnimationType::IDLE)
		{
			MoveUnit(fortress, "assassin", (Unit*)unit);
		}
	}
}

