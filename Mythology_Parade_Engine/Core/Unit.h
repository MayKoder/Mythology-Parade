#ifndef __UNIT_H__
#define __UNIT_H__

#include "Entity.h"
#include "SDL/include/SDL_rect.h"
#include "HealthSystem.h"
#include"EntityManager.h"
#include "Animation.h"
#include "j1Timer.h"

struct SDL_Texture;

enum class ReligiousType
{
	MONK,
	CLERIC,
	MISSIONARY
};

enum class UnitType 
{
	NONE=-1,
	PIKEMAN,
	ASSASSIN,
	EXPLORER,
	PRIEST,
	FOOTMAN,
	MONK,
	CLERIC,
	MISSIONARY,
	UNKNOWN
};

class Unit : public Entity, public HealthSystem
{
private:

	//Move Speed
	int moveSpeed;

	//Description / Effect
	std::string description;

public: 
	//Unit Type
	UnitType unitType;
	int time_production;
	int time_research;
	bool researched;
	Unit* enemyTarget;
	bool toDelete = false;

public:
	Unit(UnitType, iPoint);
	virtual ~Unit();

	void Init(int maxHealth);

	bool Start() override;

	bool Update(float dt);

	void SetMoveSpeed(int);


	virtual bool Draw(float dt);
	virtual void Action(Entity*);
	void ChangeState(iPoint, AnimationType);

	void MoveToTarget();
	Direction getMovementDirection(iPoint);
	void SetPath(const std::vector<iPoint>);

	void StateMachineActions(float dt);
	void Kill(iPoint);

	void Draw_Life_Bar(bool enemy = false);

protected:
	//Animation
	Direction currentDirection;
	Animation_char currentAnim;

	iPoint targetPosition;
	iPoint directionToTarget;
	fPoint normalizedDirection;

	AnimationType state;

	std::vector<iPoint> entPath;

	float timeToDespawn;
	//void SetTarget();
	//void CheckState();
	j1Timer damage_timer;
	bool show_bar_for_damage;
	bool combat_unit;


	SDL_Texture* circle_unit_tex;
	SDL_Rect circle_unit_rect;
};

#endif // !__UNIT_H__
