#include "Parser.h"

Parser::Parser()
{

}

Parser::~Parser()
{
	for(size_t i = 0; i < _agenda.size(); ++i)
		delete _agenda[i];

	for(size_t i = 0; i < _nodes.size(); ++i){
		delete _nodes[i];
	}
	if(_golden_root != NULL)
		delete _golden_root;
}

Parser::Parser(const std::string &instr, map<string,float> *fm, Vocab * vb, bool istrain):_feature_map(fm),_vb(vb)
{
	_istrain = istrain;
	_golden_root = NULL;
	vector<string> tmp_str;
	yutils::split_by_tag(instr, tmp_str);
	vector<unsigned int> tmp_int;
	vector<unsigned int> tmp_int2;
	vector<string> tmp_word; vector<string> tmp_tag;
	for(size_t i = 0; i < tmp_str.size(); ++i){
		string::size_type pos = tmp_str[i].rfind("/");
		if(pos != string::npos){
			tmp_word.push_back(tmp_str[i].substr(0, pos));
			tmp_tag.push_back(tmp_str[i].substr(pos+1, tmp_str[i].size()-pos-1));
		}
	}
	_vb->strs2ids(tmp_word, tmp_int);
	_vb->strs2ids(tmp_tag, tmp_int2);
	// we pad nodes with 2 "<s>" on the left and 2 "</s>" on the right. For feature scoring
	Node * t = new Node(0, 1, 1);
	_nodes.push_back(t);
	t = new Node(1, 1, 1);
	_nodes.push_back(t);
	for(int i = 0; i < (int)tmp_int.size(); ++i){
		t = new Node(i+2, tmp_int[i], tmp_int2[i]);
		_nodes.push_back(t);
	}
	t = new Node((int)tmp_int.size()+2, 2, 2);
	_nodes.push_back(t);
	t = new Node((int)tmp_int.size()+3, 2, 2);
	_nodes.push_back(t);
	InitEdges();
	//Decode();
}

void Parser::SetScore(Edge * e)
{
	e->_score = 0.0;
	//Y.H for no pos-tag version, all the pos tag related feature are modified or deleted
	vector<string>& features = e->_features;
	features.clear();
	stringstream ss;
	int action = e->_head;
	int p0 = 0;
	for(int j = 0; j < (int)_nodes.size(); ++j)
		if(_nodes[j]->_index == e->_left->_index)
			p0 = j;
	/*if(p0 == 0){
		cout << "dest:" << e->_left->_index << endl;
		for(size_t i = 0; i < _nodes.size(); ++i)
			cout << _nodes[i];
	}*/
	for(int j = p0-2; j < (int)_nodes.size() && j < p0+4; ++j){
		ss.str("");
		int i = j - p0;//Y.H here "i" is for presentation, "j" is the actual index in _nodes
		ss << "len" << i << "=" << _nodes[j]->_len << "|" << action;
		features.push_back(ss.str());
		ss.str("");
		int nc = _nodes[j]->_len > 1 ?0:1; // has child or not
		ss << "nc" << i << "=" << nc << "|" << action;
		features.push_back(ss.str());
		ss.str("");
		ss << "w" << i << "=" << _nodes[j]->_ch << "|" << action;
		features.push_back(ss.str());
		ss.str("");
		ss << "t" << i << "=" << _nodes[j]->_pos << "|" << action;
		features.push_back(ss.str());
		ss.str("");

		// lcp and rcp are the left-most and right-most child respectively
		int lcp = 0; int rcp = 0;
		if(_nodes[j]->_left.size() > 0)
			lcp = _nodes[j]->_left[_nodes[j]->_left.size()-1]->_pos;
		if(_nodes[j]->_right.size() > 0)
			rcp = _nodes[j]->_right[_nodes[j]->_right.size()-1]->_pos;
		ss << "tlc" << i << "=" << _nodes[j]->_pos << "#" << lcp << "|" << action;
		features.push_back(ss.str());
		ss.str("");
		ss << "trc" << i << "=" << _nodes[j]->_pos << "#" << rcp << "|" << action;
		features.push_back(ss.str());
		ss.str("");
		ss << "tlrc" << i << "=" << _nodes[j]->_pos << "#" << lcp << "#" << rcp << "|" << action;
		features.push_back(ss.str());
		ss.str("");
		//Y.H the surface distance between structure heads
		if(j < p0+3){
			int qp = _nodes[j+1]->_index - _nodes[j]->_index;
			ss << "qp" << i << "=" << qp << "|" << action;
			features.push_back(ss.str());
			ss.str("");
			ss << "qptptq" << i << "=" << qp << "#" << _nodes[j+1]->_pos << "#" << _nodes[j]->_pos << "|" << action;
			features.push_back(ss.str());
			ss.str("");
		}
	}
	vector<pair<int, int> > bigram_index;
	bigram_index.push_back(pair<int, int>(p0-1, p0));
	bigram_index.push_back(pair<int, int>(p0-1, p0+2));
	bigram_index.push_back(pair<int, int>(p0, p0+1));
	bigram_index.push_back(pair<int, int>(p0, p0+2));
	bigram_index.push_back(pair<int, int>(p0+1, p0+2));

	for(size_t i = 0; i < bigram_index.size(); ++i){
		int tp = _nodes[bigram_index[i].first]->_pos; int tq = _nodes[bigram_index[i].second]->_pos;
		int wp = _nodes[bigram_index[i].first]->_ch; int wq = _nodes[bigram_index[i].second]->_ch;
		int tlcp = 0; int trcp = 0; int tlcq = 0; int trcq = 0;
		if(_nodes[bigram_index[i].first]->_left.size() > 0)
			tlcp = _nodes[bigram_index[i].first]->_left[ _nodes[bigram_index[i].first]->_left.size()-1 ]->_pos;
		if(_nodes[bigram_index[i].first]->_right.size() > 0)
			trcp = _nodes[bigram_index[i].first]->_right[ _nodes[bigram_index[i].first]->_right.size()-1 ]->_pos;
		if(_nodes[bigram_index[i].second]->_left.size() > 0)
			tlcq = _nodes[bigram_index[i].second]->_left[ _nodes[bigram_index[i].second]->_left.size()-1 ]->_pos;
		if(_nodes[bigram_index[i].second]->_right.size() > 0)
			trcq = _nodes[bigram_index[i].second]->_right[_nodes[bigram_index[i].second]->_right.size()-1 ]->_pos;
		ss << "tptq" << i << "=" << tp << "#" << tq << "|" << action;
		features.push_back(ss.str());
		ss.str("");
		ss << "wpwq" << i << "=" << wp << "#" << wq << "|" << action;
		features.push_back(ss.str());
		ss.str("");
		ss << "tpwq" << i << "=" << tp << "#" << wq << "|" << action;
		features.push_back(ss.str());
		ss.str("");
		ss << "wptq" << i << "=" << wp << "#" << tq << "|" << action;
		features.push_back(ss.str());
		ss.str("");
		ss << "tptqlcplcq" << i << "=" << tp << "#" << tq << "#" << tlcp << "#" << tlcq << "|" << action;
		features.push_back(ss.str());
		ss.str("");
		ss << "tptqrcplcq" << i << "=" << tp << "#" << tq << "#" << trcp << "#" << tlcq << "|" << action;
		features.push_back(ss.str());
		ss.str("");
		ss << "tptqlcprcq" << i << "=" << tp << "#" << tq << "#" << tlcp << "#" << trcq << "|" << action;
		features.push_back(ss.str());
		ss.str("");
		ss << "tptqrcprcq" << i << "=" << tp << "#" << tq << "#" << trcp << "#" << trcq << "|" << action;
		features.push_back(ss.str());
		ss.str("");
	}
	for(size_t i = 0; i < features.size(); ++i){
		map<string, float>::iterator iter = _feature_map->find(features[i]);
		if(iter != _feature_map->end())
			e->_score += iter->second;
	}
	if(!_istrain) //Y.H not sure if this works or not
		e->_features.clear();
	return;
}


void Parser::Rescore(Edge * e)
{
	e->_score = 0.0;
	for(size_t i = 0; i < e->_features.size(); ++i){
		map<string, float>::iterator iter = _feature_map->find(e->_features[i]);
		if(iter != _feature_map->end())
			e->_score += iter->second;
	}
	return;
}

void Parser::InitEdges()
{
	// we pad nodes with 2 "<s>" on the left and 2 "</s>" on the right.
	Edge * tmp_e = NULL;
	for(size_t i = 2; i < _nodes.size()-3; ++i){
			// true for left, false for right
		tmp_e = new Edge(_nodes[i], _nodes[i+1], false);
		SetScore(tmp_e);
		_agenda.push_back(tmp_e);
		tmp_e = new Edge(_nodes[i], _nodes[i+1], true);
		SetScore(tmp_e);
		_agenda.push_back(tmp_e);
	}
	return;
}

bool Parser::Decode()
{
	while(_nodes.size() > 5){
		Edge * e = SelectBest();
		AddEdge(e);
	}
	PrintResult(_nodes[2]);
	cout << endl;
	return true;
}

Edge * Parser::SelectBest()
{
	Edge * res = NULL;
	for(size_t i = 0; i < _agenda.size(); ++i){
		if(_agenda[i]->_active){
			if(res == NULL || _agenda[i]->_score > res->_score){
				res = _agenda[i];
			}
		}
	}
	return res;
}


void Parser::AddEdge(Edge * e)
{
	//Y.H step1: modify node, step2: update edge score (locally)
	int del_index = e->_head == false ? e->_left->_index : e->_right->_index;
	int index = 0;
	for(vector<Node *>::iterator iter = _nodes.begin(); iter != _nodes.end(); ++iter){
		if((*iter)->_index == del_index){
			if(e->_head == false){
				Node * tmpn = *iter;
				_nodes[index+1]->_left.push_back(tmpn);
				_nodes[index+1]->_len += tmpn->_len;
				//cout << "node:" << index+1 << ", ch is:" << _nodes[index+1]->_ch << ", left:" << _nodes[index+1]->_left[0]->_ch << ", len:" << _nodes[index+1]->_len << endl;
			}
			else{
				Node * tmpn = *iter;
				_nodes[index-1]->_right.push_back(tmpn);
				_nodes[index-1]->_len += tmpn->_len;
				//cout << "node:" << index-1 << ", ch is:"<< _nodes[index-1]->_ch << ", left:" << _nodes[index-1]->_right[0]->_ch << ", len:" << _nodes[index-1]->_len << endl;
			}
			if(index > 2 && index < (int)_nodes.size()-3){
				Edge * tmp_e = new Edge(_nodes[index-1], _nodes[index+1] , false);
				Edge * tmp_e2 = new Edge(_nodes[index-1], _nodes[index+1] , true);
				_nodes.erase(iter);
				SetScore(tmp_e);
				_agenda.push_back(tmp_e);
				SetScore(tmp_e2);
				_agenda.push_back(tmp_e2);
			}
			else
				_nodes.erase(iter);
			break;
		}
		++index;
	}
	UpdateAgenda(del_index);
	e->_active = false;
	if(_golden.size() > 0){
		map<string, Edge *>::iterator iter = _golden.find(e->Tostr());
		iter->second->_active = false;
	}
	//cout << "AddEdge:" << e;
	//cout << "=========" << endl;
	//for(size_t i = 0; i < _agenda.size(); ++i)
	//	cout << _agenda[i];
	_chart.push_back(e);
	return;
}

//Y.H this is a trick part, we try to locally update edges around del_index.
//    So some edges set to inactive, update some new edges, no new edges added.
void Parser::UpdateAgenda(int del_index)
{
	//cout << "Print agenda -------" << endl;
	for(size_t i = 0; i < _agenda.size(); ++i){
		//Y.H those edges with the deleted head are set to inactive
		//cout << "del index" << del_index << endl;
		if(_agenda[i]->_left->_index == del_index || _agenda[i]->_right->_index == del_index){
			_agenda[i]->_active = false;
		}
		//Y.H Nearby edges should be updated
		else if(_agenda[i]->_right->_index < del_index){
			if(_agenda[i]->_right->_index > del_index -3 && _agenda[i]->_active){
				SetScore(_agenda[i]);
				//cout << "Updated:" << _agenda[i];
			}
		}
		else if(_agenda[i]->_left->_index > del_index){
			if(_agenda[i]->_left->_index < del_index + 3 && _agenda[i]->_active){
				SetScore(_agenda[i]);
				//cout << "Lupdated:" << _agenda[i];
			}
		}
		else;
		//cout << _agenda[i];
	}
	return;
}

void Parser::Print()
{
	cout << "========== Print chart ===========" << endl; 
	for(size_t i = 0; i < _chart.size(); ++i){
		cout << _chart[i];
	}
	return ;
}

//Y.H For my own format to store dependency tree:
// ((a,(brown,fox,#R#,),#R#,),(jumped,(with,joy,#L#,),#L#,),#R#,)
//void Parser::ReadReference(const string & instr)
//{
//	stack<Edge *> edges;
//	string::size_type pos = 0;
//	int index = 2;
//	while(pos < instr.size()){
//		//Y.H all asiII, (=40, )=41, ,=44, L=76, R=82
//		if(instr[pos] == 40){
//			Edge * eg = new Edge();
//			edges.push(eg);
//			++pos;
//		}
//		else if(instr[pos] == 41){
//			if(edges.size() == 0){
//				cerr << "brackets unmathced" << endl;
//				return;
//			}
//			Edge * tmp = edges.top();
//			_golden.insert(pair<string, Edge *>(tmp->Tostr(), tmp));
//			edges.pop();
//			if(edges.size() > 0){
//				if(edges.top()->_left == NULL) // left first, right second in sequence
//					edges.top()->_left = tmp->_head? tmp->_left : tmp->_right;
//				else
//					edges.top()->_right = tmp->_head? tmp->_left : tmp->_right;
//			}
//			++pos;
//		}
//		else if(instr[pos] == 44)
//			++pos;
//		else{
//			string::size_type tmp_pos = instr.find(",", pos);
//			if(tmp_pos == string::npos){
//				cerr << "Error at point:" << tmp_pos << endl;
//				return;
//			}
//			string str = instr.substr(pos, tmp_pos-pos);
//			if(str == "#R#")
//				edges.top()->_head = false;
//			else if (str == "#L#")
//				edges.top()->_head = true;
//			else{
//				Node * tnd = new Node(index, _vb->str2id(str));
//				_golden_nodes.push_back(tnd);
//				++index;
//				if(edges.top()->_left == NULL)
//					edges.top()->_left = tnd;
//				else
//					edges.top()->_right = tnd;
//			}
//			pos = tmp_pos + 1;
//		}
//	}
//	for(map<string, Edge *>::iterator iter = _golden.begin(); iter != _golden.end(); ++iter){
//		cout << iter->first << endl;
//	}
//	return;
//}

Node* Parser::ReadReference(const string & instr)
{
	//cout << "[" << instr << "]" << endl;
	int beg = 0;
	int end = int(instr.size())-1;
	for(; instr[beg] != 40; ++beg);
	for(++beg; instr[beg] == 32; ++beg);
	for(; instr[end] != 41; --end);
	Node * root = new Node();
	bool meethead = false;
	while(beg < end){
		if(instr[beg] != 40){
			int f= 0; int m =0; int e= 0;
			for(; beg <= end && instr[beg] != 40; ++beg){
				if(f == 0 && instr[beg] != 32)
					f = beg;
				if(instr[beg] == 47)
					m = beg;
				if(e == 0 && m != 0 && (instr[beg] == 32 || beg == end))
					e = beg;
			}
			string ch = instr.substr(f, m-f);
			string pos = instr.substr(m+1, e-m-1);
			//cout << ch << "|" << pos << endl;
			root->_ch = _vb->str2id(ch);
			root->_pos = _vb->str2id(pos);
			meethead = true;
		}
		else{
			int left = 0; 
			int right = 0;
			int sub_b = beg;int sub_e = 0;
			while(left == 0 || left != right){
				if(instr[beg] == 40)
					++left;
				else if(instr[beg] == 41){
					++right;
					sub_e = beg;
				}
				else;
				++beg;
			}
			Node * tnode = ReadReference(instr.substr(sub_b, sub_e-sub_b+1));
			if(meethead)
				root->_right.push_back(tnode);
			else
				root->_left.push_back(tnode);
			for(++beg; instr[beg] == 32; ++beg);
		}
	}
	return root;
}

void Parser::CollectEdge(Node * root)
{
	int index = 2;
	stack<Node *> nd_stack;
	nd_stack.push(root);
	while(nd_stack.size() > 0){  //Y.H inorder traversal
		Node * nd = nd_stack.top();
		if(nd->_left.size() == 0 || nd->_left[0]->_index != 0){
			nd->_index = index++;
			nd_stack.pop();
			for(int i = int(nd->_right.size())-1; i >= 0; --i){ //Y.H here is tricky: stack is first-in-last-out.
				nd_stack.push(nd->_right[i]);
			}
		}
		else{
			for(int i = int(nd->_left.size())-1; i >= 0; --i){
				nd_stack.push(nd->_left[i]);
			}
		}
	}
	nd_stack.push(root);
	while(nd_stack.size() > 0){  //Y.H Breadth first traversal
		Node * nd = nd_stack.top();
		nd_stack.pop();
		for(size_t i = 0; i < nd->_left.size(); ++i){
			Edge * eg = new Edge(nd->_left[i], nd, false);
			_golden.insert(pair<string, Edge*>(eg->Tostr(), eg));
			nd_stack.push(nd->_left[i]);
		}
		for(size_t i = 0; i < nd->_right.size(); ++i){
			Edge * eg = new Edge(nd, nd->_right[i], true);
			_golden.insert(pair<string, Edge*>(eg->Tostr(), eg));
			nd_stack.push(nd->_right[i]);
		}
	}
	return;
}

bool Parser::Train(const string & reference)
{
	_golden_root = ReadReference(reference);
	CollectEdge(_golden_root);
	/*for(map<string, Edge*>::iterator iter = _golden.begin(); iter != _golden.end(); ++iter){
		cout << iter->first << endl;
	}*/
	while(_nodes.size() > 5){
		Edge * e = SelectBest();
		if(e == NULL){
			cout << "run out of edges" << endl;
			break;
		}
		if(IsValid(e)){
			//cout << "Add:" << e;
			AddEdge(e);
		}
		else{
			Edge * g = FindValid();
			if(g == NULL){
				cerr << "Find valid error" << endl;
				for(size_t i = 0; i < _agenda.size(); ++i){
					cout << _agenda[i];
				}
				cout << "========" << endl;
				return false;
			}
			//cout << "e:" << e;
			//cout << "g:" << g;
			if(false == Updatefeature(e, g)){
				cout << "Training error" << endl;
				return false;
			}
			/*cout << "=======fired feature ========" << endl;
			if(e->_features.size() == g->_features.size()){
				for(size_t i = 0; i < e->_features.size(); ++i){
					map<string, float>::iterator iter = _feature_map->find(e->_features[i]);
					if(iter != _feature_map->end()){
						cout << iter->first << "\t" << iter->second << endl;
					}
					iter = _feature_map->find(g->_features[i]);
					if(iter != _feature_map->end()){
						cout << iter->first << "\t" << iter->second << endl;
					}
					cout << "/////////" << endl; 
				}
			}*/
			//cout << "Rescoring ================" << endl;
			for(size_t i = 0; i < _agenda.size(); ++i){
				if(_agenda[i]->_active){
					Rescore(_agenda[i]);
					//cout << _agenda[i];
				}
			}
		}
	}
	PrintResult(_nodes[2]);
	cout << endl;
	return true;
}

Edge * Parser::FindValid()
{
	Edge * best = NULL;
	for(size_t i = 0; i < _agenda.size(); ++i){
		if(_agenda[i]->_active && IsValid(_agenda[i])){
			if(best == NULL || _agenda[i]->_score > best->_score)
				best = _agenda[i];
		}
	}
	return best;
}

bool Parser::IsValid(Edge * edge)
{
	map<string, Edge*>::iterator iter = _golden.find(edge->Tostr());
	if(iter == _golden.end())
		return false;
	int child_index = edge->_head? edge->_right->_index : edge->_left->_index;
	for(map<string, Edge*>::iterator iter = _golden.begin(); iter != _golden.end(); ++iter){
		int head_index = iter->second->_head? iter->second->_left->_index : iter->second->_right->_index;
		if(child_index == head_index){
			bool inchart = false;
			for(size_t i = 0; i < _chart.size(); ++i){
				if(_chart[i]->Tostr() == iter->second->Tostr())
					inchart = true;
			}
			if(!inchart)
				return false;
		}
	}
	return true;
}


bool Parser::Updatefeature(Edge * error, Edge* gold)
{
	bool issame = true;
	if(error->_features.size() == gold->_features.size()){
		for(size_t i = 0; i < error->_features.size(); ++i)
		//	cout << error->_features[i] << "|\t|" << gold->_features[i] << endl; 
			if(error->_features[i] != gold->_features[i])
				issame = false;
	}
	if(issame == true)
		return false;
	/*if(issame == true){
		cout << endl;
		for(size_t i = 0; i < _nodes.size(); ++i)
			cout << _nodes[i];
		cout << endl;
		cout << "How are you !!=====" << endl;
		cout << endl;
		cout << "E:" << error ;
		cout << "G:" << gold;
		for(size_t i = 0;i < _agenda.size(); ++i){
			//SetScore(_agenda[i]);
			cout << _agenda[i];
		}
	}*/
	//assert(issame == false);
	for(size_t i = 0; i < error->_features.size(); ++i){
		map<string, float>::iterator iter = _feature_map->find(error->_features[i]);
		if(iter != _feature_map->end()){
			iter->second -= 1.0;
		}
		//Y.H this may not work in multi-threading, all modify one feature map
		else
			_feature_map->insert(pair<string, float>(error->_features[i], -1.0));
	}
	for(size_t i = 0; i < gold->_features.size(); ++i){
		map<string, float>::iterator iter = _feature_map->find(gold->_features[i]);
		if(iter != _feature_map->end()){
			iter->second += 1.0;
		}
		//Y.H this may not work in multi-threading, all modify one feature map
		else
			_feature_map->insert(pair<string, float>(gold->_features[i], 1.0));
	}
	return true;
}


void Parser::PrintResult(Node * root)
{
	if(root == NULL)
	  return;
	cout << "(";
	for(int i = root->_left.size()-1; i >= 0; --i){
		PrintResult(root->_left[i]);
		cout << " ";
	}
	string ch;
	string tag;
	_vb->id2str(root->_ch, ch);
	_vb->id2str(root->_pos, tag);
	cout << ch << "/" << tag;
	for(size_t i = 0; i < root->_right.size(); ++i){
		cout << " ";
		PrintResult(root->_right[i]);
	}
	cout << ")";
}
