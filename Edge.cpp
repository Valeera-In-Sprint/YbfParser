#include "Edge.h"


Edge::Edge():_left(NULL),_right(NULL)
{}

Edge::Edge(Node * left, Node * right, 
		   bool head):_left(left),_right(right), _head(head), _active(true)
{
}

Edge::~Edge()
{}

bool Edge::operator < (Edge & eg)
{
	return _score < eg._score;
}

ostream & operator << (ostream & oss, Edge * nd)
{
	oss << "[" << nd->_left->_index << "," << nd->_right->_index << "], head:" << nd->_head << ", active:" << nd->_active << ", score:" << nd->_score << endl;
	/*oss << "features:" << endl;
	for(size_t i = 0; i < nd->_features.size(); ++i){
		oss << nd->_features[i] << " ";
	}
	oss << endl;*/
	return oss;
}

string Edge::Tostr()
{
	stringstream ss;
	int left = _left == NULL ? 0:_left->_index;
	int right = _right == NULL ? 0:_right->_index;
	ss << left << "|" << right << "|" << (int)_head;
	return ss.str();
}
