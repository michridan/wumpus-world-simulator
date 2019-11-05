// Agent.h

#ifndef AGENT_H
#define AGENT_H

#include "Action.h"
#include "Percept.h"
#include "Location.h"
#include "Orientation.h"
#include <stack>
#include <algorithm> // For std::count

struct PitInfo
{
	Location loc;
	double prob; // Probability that location contains a pit
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
	int games_played;
	int size;
	bool bumped = false;
	vector<Location> targets;
	vector<Location> stenches;
	vector<Location> breezes;
	vector<Location> gold_path;
	vector<Location> safe_spots;
	vector<Location> visited;
	vector<PitInfo> known;
	vector<PitInfo> frontier;
	vector<vector<double>> pitMap;

	Location Move(Orientation o);
	Orientation Turn(Action a);
	Action GoToTarget(Location t);
	void UpdateBoard(Percept p);
	bool FindWumpus();
	vector<Location> adjacent_tiles(Location t);
	void printBoard();
};

#endif // AGENT_H
