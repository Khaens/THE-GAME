#pragma once
#ifdef ABILITYCORE_EXPORTS
#define ABILITY_API __declspec(dllexport)
#else
#define ABILITY_API __declspec(dllimport)
#endif

enum class AbilityType {
	HarryPotter,
	Gambler,
	TaxEvader,
	Soothsayer,
	Peasant
};