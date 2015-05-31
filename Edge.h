#ifndef Y_EDGE_H
#define Y_EDGE_H
#include "Node.h"

class Edge
{
public:
	Edge();
	Edge(Node * left, Node * right, bool head);
	~Edge();
	bool operator < (Edge & eg);
	friend ostream& operator << (ostream & oss, Edge * eg);
	string Tostr();
	Node * _left;
	Node * _right;
	bool _head; // true for left, false for right
	float _score;
	// Y.H this is a trick, since I think when one edge is added to the chart,
	// some other edges in the agenda should be deleted.
	bool _active;
	vector<string> _features;
	

};


#endif