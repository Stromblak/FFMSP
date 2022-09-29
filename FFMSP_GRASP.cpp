#include <iostream>
#include <vector>
#include <fstream>
#include <string.h>
#include <ctime>
#include "FFMSP_heuristica.cpp"
using namespace std;

int GRASP(vector<string> &dataset, double th, int n, int m, double det, double tMAx);

int main(int argc, char *argv[]){
	string instancia;
	double threshold = 0.75, determinismo = 0.9, tiempoMax = 30;
	
	srand(time(NULL));

	for(int i=0; i<argc; i++){
		if( !strcmp(argv[i], "-i" ) ) instancia = argv[i+1];
		if( !strcmp(argv[i], "-t" ) ) tiempoMax = atof(argv[i+1]);
		if( !strcmp(argv[i], "-th") ) threshold = atof(argv[i+1]);
		if( !strcmp(argv[i], "-d" ) ) determinismo = atof(argv[i+1]);
	}

	ifstream archivo(instancia);
	vector<string> set;
	string gen;
 
	while(archivo >> gen) set.push_back(gen);
	archivo.close();

	int n = set.size();
	int m = set[0].size();

	cout << GRASP(set, threshold, n, m, determinismo, tiempoMax) << endl;


	return 0;
}

int GRASP(vector<string> &dataset, double th, int n, int m, double det, double tMAx){
	int ti = time(NULL);

	vector< unordered_map<char, int> > contador(m);	
	for(int col=0; col<m; col++)
		for(int j=0; j<n; j++)
			contador[col][ dataset[j][col] ]++;

	vector<pair<int, int>> indices(m);
	for(int i=0; i<m; i++){
		int mayor = -1;
		for(auto par: contador[i]) mayor = max(par.second, mayor);
		indices[i] = {mayor, i};
	}
	sort(indices.begin(), indices.end(), greater<pair<int, int>>());

	int calidad;
	string sol;
	while(time(NULL) - ti <= tMAx){
		auto aux = greedy_random(dataset, contador, indices, th, n, m, det);
		sol = get<0>(aux);
		calidad = get<1>(aux);

		cout << calidad << endl;
		
	}

	return calidad;
}