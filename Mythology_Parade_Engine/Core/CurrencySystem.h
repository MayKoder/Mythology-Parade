#ifndef _CURRENCYSYSTEM_
#define _CURRENCYSYSTEM_


class CurrencySystem
{
public:
	//ECONOMY BASICS
	enum class ActionSacrifice {
		RivalKilled,
		EliteKilled,
		BeastKilled,
		TempleDestroyed,
		EncampmentDestroyed,
		MonasteriesSacrifice
	};
	enum class ActionPrayers {
		UnitConverted,
		TempleConverted,
		EncampmentConverted,
		MonasteryConverted,
		BeastConverted,
		MonasteriesPrayers
	};
	//MIRACLES AND DISASTRES
	enum class Miracles {
		ClericEnlightment,
		BordersPressure,
		BeastTame,
		MassConversion,
		ForcedTraition,
		WeakenedBorders,
		CallToArms,
		GoldenExpansion,
		Victory
	};
	enum class Disasters {
		MilitaryPressure,
		TroopsInspiration,
		HolyMeteor,
		StrikingFear,
		BloodFlow,
		TheresRoom,
		ColdAssassination,
		AnimalShelter,
		Victory
	};

public:
	CurrencySystem();
	~CurrencySystem();

	//BASIC ECONOMY FUNCTONS
	void IncreaseFaith(int number);
	void DecreaseFaith(int number);
	void IncreaseSacrifice(ActionSacrifice action);
	void IncreasePrayers(ActionPrayers action);
	void IncreaseAll(int number);

	//MIRACLES AND DISASTER
	void Miracle(Miracles action);
	void Disaster(Disasters action);

public:
	//ECONOMY
	int sacrifices;
	int faith;
	int prayers;
};

#endif // !_J1CURRENCYSYSTEM_

