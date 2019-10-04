// Agent.h

#ifndef AGENT_H
#define AGENT_H

#include "Action.h"
#include "Percept.h"
#include <stack>
#include <algorithm> // For std::count

enum Direction{NORTH, EAST, SOUTH, WEST};

struct Point
{
	int x;
	int y;

	bool operator==(Point p)
	{
		return (p.x == x) && (p.y == y);
	}
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
	Point pos;
	Point wumpus_pos;
	Direction orientation;
	int games_played;
	int size;
	bool bumped = false;
	vector<Point> targets;
	vector<Point> stenches;
	vector<Point> gold_path;
	vector<Point> safe_spots;
	vector<Point> visited;

	Point Move(Direction d);
	Direction Turn(Action a);
	Action GoToTarget(Point t);
	void UpdateBoard(Percept p);
	bool FindWumpus();
	vector<Point> adjacent_tiles(Point t);
};

#endif // AGENT_H
