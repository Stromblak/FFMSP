#include <iostream>
#include <vector>
#include <fstream>
#include <string.h>
#include <ctime>
#include "FFMSP_heuristica.cpp"
using namespace std;


int main(int argc, char *argv[]){
	string instancia;
	double threshold, alpha;
	
	srand(time(NULL));

	for(int i=0; i<argc; i++){
		if( !strcmp(argv[i], "-i" ) ) instancia = argv[i+1];
		if( !strcmp(argv[i], "-th") ) threshold = atof(argv[i+1]);
	}

	ifstream archivo(instancia);
	vector<string> set;
	string gen;

	while(archivo >> gen) set.push_back(gen);
	archivo.close();

	int n = set.size();
	int m = set[0].size();

	cout << greedy(set, threshold, n, m) << endl;
	return 0;
}