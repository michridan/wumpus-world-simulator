// Agent.cc

#include <iostream>
#include "Agent.h"

using namespace std;

Agent::Agent ()
{
	size = 1;
	safe_spots.push_back({1,1});
	targets.push_back({1,1});
	games_played = 0;
}

Agent::~Agent ()
{
}

void Agent::Initialize ()
{
	has_arrow = true;
	has_gold = false;
	wumpus_dead = false;
	orientation = EAST;
	pos.x = pos.y = 1;
}

Action Agent::Process (Percept& percept)
{
	Action action;

	UpdateBoard(percept);

	if(percept.Glitter)
	{
		has_gold = true;
		targets = gold_path;

		action = GRAB;
	}
	else if (has_gold && pos.x == 1 && pos.y == 1)
	{
		games_played++;
		targets = gold_path;
		reverse(targets.begin(), targets.end());
		action = CLIMB;
	}
	else
	{
		if(pos == targets.back())
		{
			// move to next target
			targets.pop_back();
		}
		action = GoToTarget(targets.back());
	}

	return action;
}

void Agent::GameOver (int score)
{
	games_played++;
}

Point Agent::Move (Direction d)
{
	Point p = pos;
	switch(d)
	{
		case NORTH: p.y++;
			break;
		case SOUTH: p.y--;
			break;
		case EAST: p.x++;
			break;
		case WEST: p.x--;
	}


	if(p.x < 1 || p.y < 1)
		return pos;
	else if(bumped && (p.x > size || p.y > size))
		return pos;

	return p;
}

Direction Agent::Turn(Action a)
{
	if(a == TURNLEFT)
		return static_cast<Direction> ((orientation + 3) % 4);
	else if(a == TURNRIGHT)
		return static_cast<Direction> ((orientation + 1) % 4);
	else return orientation;
}

int Manhattan(Point a, Point b)
{
	return abs(a.x - b.x) + abs(a.y - b.y);
}

Action Agent::GoToTarget(Point t)
{
	int d = Manhattan(t, pos);
	cout << "t: (" << t.x << ", " << t.y << ") #safe: " << safe_spots.size() << endl;
	cout << "p: (" << pos.x << ", " << pos.y << ")" << endl;
	
	if(Manhattan(t, Move(orientation)) < d && count(safe_spots.begin(), safe_spots.end(), Move(orientation)) > 0)
	{
		pos = Move(orientation);
		return GOFORWARD;
	}
	else if(Manhattan(t, Move(Turn(TURNLEFT))) < d)
	{
		orientation = Turn(TURNLEFT);
		return TURNLEFT;
	}
	else
	{
		orientation = Turn(TURNRIGHT);
		return TURNRIGHT;
	}
}

void remove_invalid(vector<Point> *v, int max)
{
	for(vector<Point>::iterator i = v->begin(); i < v->end();)
	{
		if(i->x > max || i->y > max)
		{
			cout << "erased!" << endl;
			i = v->erase(i);
		}
		else
		{
			i++;
		}
	}
}

void Agent::UpdateBoard(Percept p)
{
	if (p.Bump)
	{
		bumped = true;

		// Reset position & Update Boundaries
		if(orientation % 2 == 1) // Meaning EAST
		{
			pos.x--;
			size = pos.x;
		}
		else
		{
			pos.y--;
			size = pos.y;
		}

		remove_invalid(&targets, size);
		remove_invalid(&gold_path, size);
		remove_invalid(&safe_spots, size);
		return;
	}
	if(count(visited.begin(), visited.end(), pos) == 1)
	{
		if(games_played == 0 && !has_gold)
		{
			while(count(gold_path.begin(), gold_path.end(), pos) == 1)
			{
				gold_path.pop_back();
			}
			gold_path.push_back(pos);
		}
		return;
	}

	if(!p.Stench)
	{
		for(int i = 0; i < 4; i++)
		{
			Point temp = Move(static_cast<Direction>(i));
			if(count(safe_spots.begin(), safe_spots.end(), temp) == 0)
			{
				cout << temp.x << ", " << temp.y << " safe!" << endl;
				safe_spots.push_back(temp);
				targets.push_back(temp);
			}
		}
	}
	else if(!wumpus_known)
	{
		stenches.push_back(pos);
		wumpus_known = FindWumpus();
	}
	else
	{
		for(int i = 0; i < 4; i++)
		{
			Point temp = Move(static_cast<Direction>(i));
			if(!(temp == wumpus_pos) && (count(safe_spots.begin(), safe_spots.end(), temp) == 0))
			{
				cout << temp.x << ", " << temp.y << " safe!" << endl;
				safe_spots.push_back(temp);
				targets.push_back(temp);
			}
		}
	}

	visited.push_back(pos);
	if(games_played == 0 && !has_gold)
	{
		gold_path.push_back(pos);
	}
}

bool Agent::FindWumpus()
{
	vector<Point> suspects, neighbors;

	// Exit if too few stenches
	if(stenches.size() < 2)
		return false;

	// Find all potential Wumpus locations
	Point cur;

	neighbors = adjacent_tiles(pos);

	while(!neighbors.empty())
	{
		int x = 0;
		cur = neighbors.back();
		neighbors.pop_back();
		
		for(Point stench: stenches)
		{
			if(Manhattan(cur, stench) == 1)
				x++;
		}
		if(x == stenches.size())
		{
			suspects.push_back(cur);
		}
	}

	// Narrow down list
	for(vector<Point>::iterator i = suspects.begin(); i < suspects.end(); i++)
	{
		cur = *i;

		for(Point t: adjacent_tiles(cur))
		{
			if(count(visited.begin(), visited.end(), pos) > 0 && count(stenches.begin(), stenches.end(), t))
			{
				i = suspects.erase(i);
				break;
			}
		}
	}

	if(suspects.size() == 1)
	{
		wumpus_pos = suspects[0];
		return true;
	}

	return false;
}

vector<Point> Agent::adjacent_tiles(Point t)
{
	vector<Point> neighbors;

	if(t.x + 1 < size)
	{
		neighbors.push_back({t.x + 1, t.y});
	}
	if(t.x - 1 >= 1)
	{
		neighbors.push_back({t.x - 1, t.y});
	}
	if(t.y + 1 < size)
	{
		neighbors.push_back({t.x, t.y + 1});
	}
	if(t.y - 1 >= 1)
	{
		neighbors.push_back({t.x, t.y - 1});
	}

	return neighbors;
}