#include "Node.h"

Node::Node():_index(0),_len(1)
{}

Node::~Node()
{
	for(size_t i = 0; i < _left.size(); ++i)
		delete _left[i];
	for(size_t i = 0; i < _right.size(); ++i)
		delete _right[i];
}

Node::Node(int index, unsigned int ch):_index(index), _ch(ch),_len(1)
{
}

Node::Node(int index, unsigned int ch, int pos):_index(index), _ch(ch),_len(1), _pos(pos)
{
}

ostream & operator << (ostream & oss, Node * nd)
{
	oss << "[" << nd->_ch << "," << nd->_index << "]" << endl;
	return oss;
}
