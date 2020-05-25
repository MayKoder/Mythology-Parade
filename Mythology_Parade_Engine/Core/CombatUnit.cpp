#include "CombatUnit.h"
#include "j1ParticleManager.h"
#include "p2Log.h"
#include "j1TutorialScene.h"

CombatUnit::CombatUnit(UnitType type, iPoint pos) : Unit(type, pos), range(0), damage(0)
{
	//TODO 10: Change textures
	unitType = type;
	position = {(float)pos.x, (float)pos.y};
	canLevel = true;
	switch (unitType)
	{
	case UnitType::ASSASSIN:
		time_production = 90;
		name = "assassin";
		time_research = 0;
		researched = true;
		//Change texture
		LevelSystem::Init(3500, 6500, 9500);
		CombatUnit::Init(100, 15, 1, 80);
		collisionRect = { 0, 0, 30, -55 };
		break;
	case UnitType::PIKEMAN:
		time_production = 90;
		time_research = 70;
		researched = false;
		name = "pikeman";
		//Change Texture
		LevelSystem::Init(3000, 6000, 9500);
		CombatUnit::Init(110, 25, 1, 40);
		collisionRect = { 0, 0, 30, -55 };
		break;
	case UnitType::EXPLORER:
		name = "explorer";
		break;
	case UnitType::PRIEST:
		name = "priest";
		break;
	case UnitType::FOOTMAN:
		name = "footman";
		break;
	}

	realDamage = damage;

	combat_unit = true;
	show_bar_for_damage = false;

}

CombatUnit::~CombatUnit()
{
}

void CombatUnit::Action(Entity* entity)
{
	switch (unitType)
	{
	case UnitType::ASSASSIN:
	LOG("I'm a assassin unit!");
		break;
	case UnitType::PIKEMAN:
	LOG("I'm a pikeman unit!");
		break;

	}
	//Attack enemy
	Unit* target = (Unit*)entity;
	target->RecieveDamage(GetDamageValue());
}

void CombatUnit::LevelUp()
{
	LevelSystem::LevelUp();
	switch (unitType)
	{
	case UnitType::PIKEMAN:
		break;
	case UnitType::ASSASSIN:
		switch (GetLevel())
		{
		case 1:
			IncreaseDamage(5);
			break;
		case 2:
			IncreaseHealth(10);
			break;
		case 3:
			IncreaseSpeed(1);
		}
		break;
	case UnitType::EXPLORER:
		break;
	case UnitType::PRIEST:
		break;
	case UnitType::FOOTMAN:
		break;

	}
	App->particleManager->CreateParticle({ (int)position.x-20,(int)position.y-60 }, { 0,-1 }, 10, ParticleAnimation::Level_Up);
	if (App->scene->isInTutorial == true) {
		App->tutorialscene->level_up = true;
	}
}

void CombatUnit::SetDamage(int d)
{
	damage = realDamage + d;
}



void CombatUnit::Init(int maxHealth, int damage, int range, int speed)
{
	Unit::Init(maxHealth);
	this->damage = damage;
	this->range = range;
	this->speed = speed;
	SetMoveSpeed(speed);

	//TODO: Convert to a get funtions
	targetPosition.ResetAsPosition();
	ChangeState(targetPosition, state);
}

bool CombatUnit::Update(float dt)
{
	Unit::Update(dt);

	if (isSelected()) {
		switch (GetLevel())
		{
		case (1):
			App->render->Blit(level_tex, (position.x - 6), (position.y - 63), &level_rect, 1, 0, 0, 0, 1);
			break;
		case (2):
			App->render->Blit(level_tex, (position.x - 2), (position.y - 63), &level_rect, 1, 0, 0, 0, 1);
			App->render->Blit(level_tex, (position.x - 10), (position.y - 63), &level_rect, 1, 0, 0, 0, 1);
			break;
		case (3):
			App->render->Blit(level_tex, (position.x - 6), (position.y - 63), &level_rect, 1, 0, 0, 0, 1);
			App->render->Blit(level_tex, (position.x - 14), (position.y - 63), &level_rect, 1, 0, 0, 0, 1);
			App->render->Blit(level_tex, (position.x + 2), (position.y - 63), &level_rect, 1, 0, 0, 0, 1);
			break;
		}
	}
	return true;
}

int CombatUnit::GetDamageValue()
{
	return damage;
}

int CombatUnit::GetRangeValue()
{
	return range;
}


int CombatUnit::GetSpeedValue()
{
	return speed;
}

void CombatUnit::IncreaseHealth(int value)
{
	HealthSystem::IncreaseHealth(value);
}

void CombatUnit::IncreaseSpeed(int value)
{
	speed += value;
}

void CombatUnit::IncreaseDamage(int value)
{
	damage += value;
}


void CombatUnit::SetDefaultHealth()
{
	HealthSystem::SetDefaultHealth();
}
void CombatUnit::SetLevel(int i)
{
	for (int j = 0; j < i; j++)
	{
		LevelUp();
	}
}

void CombatUnit::SetHealth(int value)
{
	HealthSystem::SetHealth(value);
}
