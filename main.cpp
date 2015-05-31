#include "Parser.h"

bool Init(char * infile, char * ref_file, char * fea_file, map<string, float> & fea_map, vector<string> & p_vec, vector<string> & r_vec)
{
	string line;
	ifstream in(infile);
	while(getline(in, line)){
		p_vec.push_back(line);
	}
	in.close();
	ifstream in2(ref_file);
	if(in2.good()){
		while(getline(in2, line)){
			r_vec.push_back(line);
		}
		in2.close();
	}
	ifstream inmap(fea_file);
	while(getline(inmap, line)){
		string::size_type pos = line.find("\t");
		if(pos != string::npos){
			fea_map.insert(pair<string, float>(line.substr(0, pos), atof(line.substr(pos+1, line.size()-pos-1).c_str() )));
		}
	}
	inmap.close();
	return true;
}


int main(int argc, char * argv[])
{
	if(argc < 5){
		cerr << "Usage: exe parse_file, reference_file, feat_file, vocab_file" << endl;
		return -1;
	}
	map<string, float> fea_map;
	vector<string> p_vec;
	vector<string> r_vec;
	Vocab vb(argv[4]);
	if(false == Init(argv[1], argv[2], argv[3], fea_map, p_vec, r_vec)){
		cerr << "Init error" << endl;
		return -1;
	}
	struct timeval beg_time;
	gettimeofday(&beg_time,NULL);
	for(size_t i = 0; i< p_vec.size(); ++i){
		//cout << i << endl;
		Parser p(p_vec[i], &fea_map, &vb, true);
		if(r_vec.size() > 0)
			p.Train(r_vec[i]);
		else
			p.Decode();
		//p.Print();
	}
	struct timeval end_time;
	gettimeofday(&end_time, NULL);
	double timeuse = (double)(end_time.tv_sec - beg_time.tv_sec) + ((double) (end_time.tv_usec - beg_time.tv_usec)) / 1000000;
	cout << "Total_TIMEUSE = [" << timeuse << "], time per sentence: [" << timeuse/p_vec.size() << "]"  << endl;
	if(r_vec.size() > 0){
		ofstream out("feamap");
		for(map<string, float>::iterator iter = fea_map.begin(); iter != fea_map.end(); ++iter){
			out << iter->first << "\t" << iter->second << endl;
		}
		out.close();
	}
	return 0;
}
