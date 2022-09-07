#include <iostream>
#include <vector>
#include <fstream>
#include <string.h>

using namespace std;

string greedy(vector<string> &set, double th, int n, int m);

int main(int argc, char *argv[]){
	string instancia;
	double threshold;

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

	string solucion = greedy(set, threshold, n, m);

	cout << solucion << endl;
	return 0;
}

string greedy(vector<string> &set, double th, int n, int m){
	string sol = "hola";

	return sol;
}
