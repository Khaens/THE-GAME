#pragma once

class Game;

class Harry_Potter
{
public:
	void UseAbility(Game* game);
	
private:
	bool m_canUse = true;
};

