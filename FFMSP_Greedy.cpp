#include <iostream>
#include <vector>
#include <fstream>
#include <string.h>
#include <stack>
#include <map>
#include <set>
#include <random>
#include <ctime>
#include <algorithm>
using namespace std;


int greedy(vector<string> &setGen, double th, int n, int m);

int main(int argc, char *argv[]){
	string instancia;
	double threshold;
	
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

	int solucion = greedy(set, threshold, n, m);

	cout << solucion << endl;
	return 0;
}

int greedy(vector<string> &setGen, double th, int n, int m){
	vector<char> bases = {'A', 'C', 'G', 'T'};
	vector<int> hamming(n);
	map<char, int> difBases; // cantidad strings donde porcentaje hamming >= th
	string sol;

	for(int col=0; col<m; col++){  //para cada columna	
		difBases['A'] = 0;
		difBases['C'] = 0;
		difBases['G'] = 0;
		difBases['T'] = 0;

		for(char base: bases){	 // para cada base
			for(int j=0; j<n; j++){	 //para cada gen
				if(setGen[j][col] != base){
					double porcentajeDif = (double)(hamming[j] + 1) / (col+1);
					if(porcentajeDif >= th) difBases[base]++;
				}
			}
		}

		int bMAx = 0;
		vector<char> maximos;
		for(pair<char, int> p: difBases) bMAx = max(bMAx, p.second);
		for(pair<char, int> p: difBases) if(p.second == bMAx) maximos.push_back(p.first);

		char actual;
		if(maximos.size() > 1){
			map<char, int> counterBases;
			counterBases['A'] = 0;
			counterBases['C'] = 0;
			counterBases['G'] = 0;
			counterBases['T'] = 0;
			for(int j=0; j<n; j++) counterBases[ setGen[j][col] ]++;

			int bMAx2 = 0;
			vector<char> maximos2;
			for(pair<char, int> p: counterBases) if(p.second == bMAx2) maximos2.push_back(p.first);

			if(maximos2.size() > 1) actual = maximos2[ rand() % maximos2.size() ];
		
		}else actual = maximos[0];


		for(int j=0; j<n; j++) if(setGen[j][col] != actual) hamming[j]++;	
		sol += actual;
	}

	int calidad = 0;
	for(int h: hamming) if(  (double)h / m  >= th) calidad++;

	return calidad;
}