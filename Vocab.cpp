#include "Vocab.h"

Vocab::Vocab(const string & infile)
{
	string line;
	ifstream in(infile.c_str());
	unsigned int index = 0;
	/*_dict.insert(pair<string, int>("<UNK>", index));
	_index.push_back("<UNK>"); // 0 for "<UNK>"
	++index;
	_dict.insert(pair<string, int>("<s>", index));
	_index.push_back("<s>"); // 1 for "<s>"
	++index;
	_dict.insert(pair<string, int>("</s>", index));
	_index.push_back("</s>"); // 2 for "</s>"
	++index;*/
	vector<string> tmp_vec;
	while(getline(in, line)){
		_dict.insert(pair<string, unsigned int>(line, index));
		_index.push_back(line);
		++index;
		/*yutils::split_by_tag(line, tmp_vec);
		map<string, unsigned int>::iterator iter;
		for(size_t i = 0; i < tmp_vec.size(); ++i){
			iter = _dict.find(tmp_vec[i]);
			if
		}*/
	}
	in.close();
}

Vocab::~Vocab()
{}

int Vocab::str2id(const string & instr)
{
	map<string, unsigned int>::iterator iter = _dict.find(instr);
	if(iter != _dict.end())
		return iter->second;
	else
		return 0;
}

void Vocab::id2str(int id, string & outstr)
{
	if(id > (int)_index.size())
		outstr = "<UNK>";
	else
		outstr = _index[id];
	return;
}


void Vocab::strs2ids(const vector<string> & invec, vector<unsigned int> & outvec)
{
	for(size_t i = 0; i < invec.size(); ++i){
		outvec.push_back(str2id(invec[i]));
	}
	return;
}

void Vocab::ids2strs(const vector<int> & invec, vector<string> & outvec)
{
	string tmp;
	for(size_t i = 0; i < invec.size(); ++i){
		id2str(invec[i], tmp);
		outvec.push_back(tmp);
	}
	return;
}
