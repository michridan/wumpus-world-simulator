// Agent.cc

#include <iostream>
#include <iomanip>
#include "Agent.h"

using namespace std;

Agent::Agent ()
{
	pitMap = {
		{0, .2, .2, .2, .2},
		{.2, .2, .2, .2, .2},
		{.2, .2, .2, .2, .2},
		{.2, .2, .2, .2, .2},
		{.2, .2, .2, .2, .2}};
	size = 5;
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
	orientation = RIGHT;
	pos.X = pos.Y = 1;
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
	else if (has_gold && pos.X == 1 && pos.Y == 1)
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

Location Agent::Move (Orientation o)
{
	Location p = pos;
	switch(o)
	{
		case UP: p.Y++;
			break;
		case DOWN: p.Y--;
			break;
		case RIGHT: p.X++;
			break;
		case LEFT: p.X--;
	}


	if(p.X < 1 || p.Y < 1)
		return pos;
	else if(bumped && (p.X > size || p.Y > size))
		return pos;

	return p;
}

Orientation Agent::Turn(Action a)
{
	if(a == TURNLEFT)
		return static_cast<Orientation> ((orientation + 1) % 4);
	else if(a == TURNRIGHT)
		return static_cast<Orientation> ((orientation + 3) % 4);
	else return orientation;
}

int Manhattan(Location a, Location b)
{
	return abs(a.X - b.X) + abs(a.Y - b.Y);
}

Action Agent::GoToTarget(Location t)
{
	int d = Manhattan(t, pos);
	cout << "t: (" << t.X << ", " << t.Y << ") #safe: " << safe_spots.size() << endl;
	cout << "p: (" << pos.X << ", " << pos.Y << ")" << endl;
	cout << "o: ";
	PrintOrientation(orientation);
	cout << endl;
	
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

void remove_invalid(vector<Location> *v, int max)
{
	for(vector<Location>::iterator i = v->begin(); i < v->end();)
	{
		if(i->X > max || i->Y > max)
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
	printBoard();
	if (p.Bump)
	{
		bumped = true;

		// Reset position & Update Boundaries
		if(orientation == RIGHT)
		{
			pos.X--;
			size = pos.X;
		}
		else
		{
			pos.X--;
			size = pos.Y;
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
			Location temp = Move(static_cast<Orientation>(i));
			if(count(safe_spots.begin(), safe_spots.end(), temp) == 0)
			{
				cout << temp.X << ", " << temp.Y << " safe!" << endl;
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
			Location temp = Move(static_cast<Orientation>(i));
			if(!(temp == wumpus_pos) && (count(safe_spots.begin(), safe_spots.end(), temp) == 0))
			{
				cout << temp.X << ", " << temp.Y << " safe!" << endl;
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
	vector<Location> suspects, neighbors;

	// Exit if too few stenches
	if(stenches.size() < 2)
		return false;

	// Find all potential Wumpus locations
	Location cur;

	neighbors = adjacent_tiles(pos);

	while(!neighbors.empty())
	{
		int x = 0;
		cur = neighbors.back();
		neighbors.pop_back();
		
		for(Location stench: stenches)
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
	for(vector<Location>::iterator i = suspects.begin(); i < suspects.end(); i++)
	{
		cur = *i;

		for(Location t: adjacent_tiles(cur))
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

vector<Location> Agent::adjacent_tiles(Location t)
{
	vector<Location> neighbors;

	if(t.X + 1 < size)
	{
		neighbors.push_back({t.X + 1, t.Y});
	}
	if(t.X - 1 >= 1)
	{
		neighbors.push_back({t.X - 1, t.Y});
	}
	if(t.Y + 1 < size)
	{
		neighbors.push_back({t.X, t.Y + 1});
	}
	if(t.Y - 1 >= 1)
	{
		neighbors.push_back({t.X, t.Y - 1});
	}

	return neighbors;
}

void Agent::printBoard()
{
	cout << "P(pit):" << endl;
	for(vector<vector<double>>::reverse_iterator i = pitMap.rbegin(); i != pitMap.rend(); i++)
	{
		for(double prob: *i)
		{
			cout << ' ' << fixed << setprecision(2) << prob;
		}
		cout << endl;
	}
}