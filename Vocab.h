#ifndef Y_VOCAB_H
#define Y_VOCAB_H
#include "utils.h"

class Vocab
{
public:
	Vocab(const string & infile);
	~Vocab();
	int str2id(const string & instr);
	void id2str(int id, string & outstr);
	void strs2ids(const vector<string> & invec, vector<unsigned int> & outvec);
	void ids2strs(const vector<int> & invec, vector<string> & outvec);

	map<string, unsigned int> _dict;
	vector<string> _index;
};

#endif