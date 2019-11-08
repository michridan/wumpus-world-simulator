// Agent.cc

#include <iostream>
#include <iomanip>
#include <cmath> // for pow()
#include "Agent.h"

using namespace std;

// Agent Member Functions

Agent::Agent ()
{
	pitMap.board = {
		{0, .2, .2, .2, .2},
		{.2, .2, .2, .2, .2},
		{.2, .2, .2, .2, .2},
		{.2, .2, .2, .2, .2},
		{.2, .2, .2, .2, .2}};
	size = 5;
	targets.push_back({1,1});
	learned = false;
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

	printLocs("Known:", known);
	printLocs("Frontier:", frontier);
	printLocs("Breeze:", breezes);
	printLocs("GoldPath:", gold_path);
	printLocs("Targets:", targets);
	cout << endl;
	calculateProbs();
	pitMap.printBoard();

	if(percept.Glitter)
	{
		has_gold = true;
		targets = gold_path;

		action = GRAB;
	}
	else if (has_gold && pos.X == 1 && pos.Y == 1)
	{
		learned == true;
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
	known.push_back(pos);
	pitMap.set(pos, 1);
	frontier.erase(find(frontier.begin(), frontier.end(), pos)); // make better?
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
	else if((p.X > size || p.Y > size))
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

Action Agent::GoToTarget(Location t)
{
	int d = Manhattan(t, pos);
	cout << "Target: (" << t.X << ", " << t.Y << ")" << endl;
	cout << "Position: (" << pos.X << ", " << pos.Y << ")" << endl;
	cout << "Orientation: ";
	PrintOrientation(orientation);
	cout << endl;
	
	if(Manhattan(t, Move(orientation)) < d)
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

void Agent::UpdateBoard(Percept p)
{
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
		remove_invalid(&frontier, size);
		return;
	}

	if(p.Breeze && !existsIn(pos, breezes))
	{
		breezes.push_back(pos);
	}

	if(existsIn(pos, known))
	{
		if(!learned && !has_gold)
		{
			while(existsIn(pos, gold_path))
			{
				gold_path.pop_back();
			}
		}
	}

	for(int i = 0; i < 4; i++)
	{
		Location temp = Move(static_cast<Orientation>(i));
		if(!existsIn(temp, frontier) && !existsIn(temp, known))
		{
			frontier.push_back(temp);
			targets.push_back(temp);
		}
	}

	if(p.Scream)
	{
		possible_wumpuses.clear();
		possible_wumpuses.push_back(Move(orientation));
		wumpus_dead = true;
	}

	if(p.Stench && possible_wumpuses.size() != 1)
	{
		stenches.push_back(pos);
		possible_wumpuses = FindWumpus();
	}

	if(!existsIn(pos, known))
	{
		known.push_back(pos);
		removeLoc(pos, &frontier);
		pitMap.set(pos, 0);
	}

	if(!learned && !has_gold)
	{
		gold_path.push_back(pos);
	}
}

void Agent::calculateProbs()
{
	// use binary encoding to get combos?
	ProbMap qMap;
	qMap.board = pitMap.board;
	for(Location query: frontier)
	{
		double ptrue = 0, pfalse = 0;
		vector<Location> qknown = known, qfrontier = frontier;
		qknown.push_back(query);
		removeLoc(query, &qfrontier);
		cout << "Query: (" << query.X << ',' << query.Y << ')' << endl;
		printLocs("  Frontier': ", qfrontier); 

		for(int C = 0; C < pow(2, qfrontier.size()); C++) // Cycle through all possible true/false combinations
		{
			double pC = 1;
			for(int i = 0; i < qfrontier.size(); i++)
			{
				double p = (C >> i) % 2; // gives the probability for frontier[i] in this combination 
				qMap.set(qfrontier[i], p);
				pC *= (p - 0.8); // .2 if true, -.8 if false, will get absolute value later
			}
			pC = abs(pC);
			qMap.set(query, 1);
			ptrue += pC * validateBoard(qMap);
			qMap.set(query, 0);
			pfalse += pC * validateBoard(qMap);
		}
		ptrue *= 0.2;
		pfalse *= 0.8;
		ptrue /= (ptrue + pfalse);
		cout << "  P(true) = " << ptrue << endl;
		pitMap.set(query, ptrue);
	}
	cout << endl;
}

// Returns 1 if pit information is consistent with breezes, 0 otherwise
double Agent::validateBoard(ProbMap map)
{
	for(Location b: known)
	{
		double sum = 0;
		for(Location p: adjacent_tiles(b))
		{
			sum += map.get(p);
		}

		if((sum == 0 && existsIn(b, breezes)) || (sum != 0 && !existsIn(b, breezes)))
			return 0;
	}
	return 1;
}

vector<Location> Agent::FindWumpus()
{
	vector<Location> suspects, neighbors;

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
			if(existsIn(t, known) && !existsIn(t, stenches))
			{
				i = suspects.erase(i);
				break;
			}
		}
	}

	printLocs("Possible Wumpus Locations:", suspects);

	if(suspects.size() == 1)
	{
		wumpus_pos = suspects[0];
	}

	return suspects;
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

// ProbMap Member Functions

const double ProbMap::get(Location l)
{
	return board[l.Y - 1][l.X - 1];
}

void ProbMap::set(Location l, double p)
{
	board[l.Y - 1][l.X - 1] = p;
}

void ProbMap::printBoard()
{
	cout << "P(pit):" << endl;
	for(vector<vector<double>>::reverse_iterator i = board.rbegin(); i != board.rend(); i++)
	{
		for(double prob: *i)
		{
			cout << ' ' << fixed << setprecision(2) << prob;
		}
		cout << endl;
	}
}

// Utility functions

int Manhattan(Location a, Location b)
{
	return abs(a.X - b.X) + abs(a.Y - b.Y);
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

void printLocs(string name, vector<Location> v)
{
	cout << name;
	for(Location l: v)
	{
		cout << " (" << l.X << "," << l.Y << ')';
	}

	cout << endl;
}

void removeLoc(Location l, vector<Location> *v)
{
	v->erase(find(v->begin(), v->end(), l));
}

bool existsIn(Location l, vector<Location> v)
{
	return (count(v.begin(), v.end(), l) > 0);
}