// Agent.cc

#include <iostream>
#include "Agent.h"

using namespace std;

Agent::Agent ()
{
	has_arrow = true;
	has_gold = false;
	wumpus_dead = false;
	orientation = EAST;
	x_pos = y_pos = 1;
}

Agent::~Agent ()
{

}

void Agent::Initialize ()
{

}

Action Agent::Process (Percept& percept)
{
	Action action;

	if(percept.Glitter)
	{
		has_gold = true;
		action = GRAB;
	}
	else if(percept.Stench && has_arrow)
	{
		has_arrow = false;
		action = SHOOT;
	}
	else if (percept.Bump)
	{
		action = static_cast<Action> (rand() % 2 + 1); // 50/50 shot of left/right
		UpdatePos(static_cast<Direction> ((orientation + 2) % 4));
		if(action == TURNLEFT)
			orientation = static_cast<Direction> ((orientation + 1) % 4);
		else
			orientation = static_cast<Direction> ((orientation + 3) % 4);
	}
	else if (has_gold && x_pos == 1 && y_pos == 1)
	{
		action = CLIMB;
	}
	else
	{
		action = GOFORWARD;
		UpdatePos(orientation);
	}
	

	return action;
}

void Agent::GameOver (int score)
{

}

void Agent::UpdatePos (Direction d)
{
	switch(d)
	{
		case NORTH: x_pos++;
			break;
		case SOUTH: x_pos--;
			break;
		case EAST: y_pos++;
			break;
		case WEST: y_pos--;
	}
}
