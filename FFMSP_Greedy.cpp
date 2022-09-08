#include <iostream>
#include <vector>
#include <fstream>
#include <string.h>
#include <stack>
#include <map>
#include <set>
#include <random>
#include <ctime>



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
	string sol;
	vector<char> bases = {'A', 'C', 'G', 'T'};
	vector<int> hamming(n);
	stack<int> cambios;
	map<char, int> difBases; // cantidad strings donde se cumple el th
	
	for(int col=0; col<m; col++){  //para cada columna	
		difBases['A'] = 0;
		difBases['C'] = 0;
		difBases['G'] = 0;
		difBases['T'] = 0;

		for(char base: bases){	 // para cada base
			for(int j=0; j<n; j++){	 //para cada genoma

				if(setGen[j][col] != base) cambios.push(j);

				while(!cambios.empty()){
					int indice = cambios.top();
					cambios.pop();

					if(  (double)(hamming[indice] + 1) / (double)(col+1)  >= th) difBases[base]++;
				}
			}
		}


		int bMAx = 0;
		vector<char> maximos;
		for(pair<char, int> p: difBases) bMAx = max(bMAx, p.second);
		for(pair<char, int> p: difBases) if(p.second == bMAx) maximos.push_back(p.first);

		char actual = maximos[ rand() % maximos.size() ];
		sol += actual;

		for(int j=0; j<n; j++) if(setGen[j][col] != actual) hamming[j]++;	
	}

	int calidad = 0;
	for(int h: hamming) if(  (double)h / (double)m  >= th) calidad++;

	return calidad;
}