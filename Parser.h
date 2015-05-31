#ifndef Y_PARSER_H
#define Y_PARSER_H

#include "Edge.h"
#include "Vocab.h"

class Parser
{
public:
	Parser();
	Parser(const string & instr, map<string, float> * fm, Vocab * vb, bool istrain);
	~Parser();
	bool Decode();
	void Print();
	bool Train(const string & reference);
	bool IsValid(Edge * edge);
	Edge * FindValid();
	Node* ReadReference(const string & instr);
	void PrintResult(Node * root);
	vector<Node *> _nodes;

private:
	void InitEdges();
	void SetScore(Edge * e);
	void AddEdge(Edge * e);
	Edge * SelectBest();
	//Y.H this is a trick part, we try to locally update edges around del_index.
	//    So some edges set to inactive, update some new edges, no new edges added.
	void UpdateAgenda(int del_index);
	bool Updatefeature(Edge * error, Edge* gold);
	void Rescore(Edge * e);
	void CollectEdge(Node * root);
	//priority_queue<Edge> _agenda;
	vector<Edge *> _agenda;
	vector<Edge *> _chart;
	map<string, float> * _feature_map;
	map<string, Edge*> _golden;
	//vector<Node *> _golden_nodes;
	Node * _golden_root;
	Vocab * _vb;
	bool _istrain; //Y.H true for training, the difference is whether to keep all the feature set for edges
};

#endif
