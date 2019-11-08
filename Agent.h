// Agent.h

#ifndef AGENT_H
#define AGENT_H

#include "Action.h"
#include "Percept.h"
#include "Location.h"
#include "Orientation.h"
#include <stack>
#include <algorithm> // For std::count

/*
Helper Class to make handling the probability storing a bit easier
*/
class ProbMap
{
public:
	const double get(Location l);
	void set(Location l, double p);
	void printBoard();

	vector<vector<double>> board;
};

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
	bool wumpus_known;
	Location pos;
	Location wumpus_pos;
	Orientation orientation;
	bool learned;
	int size;
	bool bumped = false;
	vector<Location> targets;
	vector<Location> possible_wumpuses;
	vector<Location> stenches;
	vector<Location> breezes;
	vector<Location> gold_path;
	vector<Location> known;
	vector<Location> frontier;
	ProbMap pitMap;

	Location Move(Orientation o);
	Orientation Turn(Action a);
	Action GoToTarget(Location t);
	void UpdateBoard(Percept p);
	void calculateProbs();
	double validateBoard(ProbMap map);
	vector<Location> FindWumpus();
	vector<Location> adjacent_tiles(Location t);
};

int Manhattan(Location a, Location b);
void remove_invalid(vector<Location> *v, int max);
void printLocs(string name, vector<Location> v);
void removeLoc(Location l, vector<Location> *v);
bool existsIn(Location l, vector<Location> v);

#endif // AGENT_H
