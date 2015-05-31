#ifndef Y_NODE_H
#define Y_NODE_H

#include "utils.h"
class Node
{
	
public:
	Node();
	Node(int index, unsigned int _ch);
	Node(int index, unsigned int ch, int pos);
	~Node();
	friend ostream& operator << (ostream & oss, Node * nd);


	int _index;
	unsigned int _ch; // all digit here
	vector<Node *> _left;
	vector<Node *> _right;
	int _len; // the span of the structure, for child len=1
	int _pos; // pos tag of the word
	// Y.H  True for parent (visible in the queue), 
	//      False for child (invisible in the queue)
	//bool _active;
	//Node * _parent;

};

#endif
