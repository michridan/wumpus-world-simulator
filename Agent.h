// Agent.h

#ifndef AGENT_H
#define AGENT_H

#include "Action.h"
#include "Percept.h"

enum Direction{NORTH, EAST, SOUTH, WEST};

class Agent
{
public:
	Agent ();
	~Agent ();
	void Initialize ();
	Action Process (Percept& percept);
	void GameOver (int score);

private:
	bool has_arrow;
	bool has_gold;
	bool wumpus_dead;
	int x_pos;
	int y_pos;
	Direction orientation;

	void UpdatePos(Direction d);
};

#endif // AGENT_H
