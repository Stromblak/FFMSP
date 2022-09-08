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
	map<char, int> cumpleTH; // contador cantidad strings donde porcentaje hamming >= th
	string sol;

	// contar cuanto se repiten las bases en cada columna
	vector<map<char, int>> contador(m);
	for(int col=0; col<m; col++){  //para cada columna	
		contador[col]['A'] = 0;
		contador[col]['C'] = 0;
		contador[col]['G'] = 0;
		contador[col]['T'] = 0;
		for(int j=0; j<n; j++) contador[col][ setGen[j][col] ]++;
	}

	for(int col=0; col<m; col++){  //para cada columna	
		cumpleTH['A'] = 0;
		cumpleTH['C'] = 0;
		cumpleTH['G'] = 0;
		cumpleTH['T'] = 0;

		for(char base: bases){	 // para cada base
			for(int j=0; j<n; j++){	 //para cada gen
				if(setGen[j][col] != base){
					double porcentajeDif = (double)(hamming[j] + 1) / (col+1);
					if(porcentajeDif >= th) cumpleTH[base]++;
				}
			}
		}

		// encontrar a los maximos de cumpleTH
		int bMAx = 0;
		vector<char> maximos;
		for(pair<char, int> p: cumpleTH) bMAx = max(bMAx, p.second);
		for(pair<char, int> p: cumpleTH) if(p.second == bMAx) maximos.push_back(p.first);

		// encontrar cuanto se repiten los maximos en la columna
		map<char, int> contadorMaximos;
		for(char c: maximos) contadorMaximos[c] = contador[col][c];
		
		// encontrar los maximos de las repeticiones de columnas
		bMAx = 0;
		maximos.clear();
		for(pair<char, int> p: contadorMaximos) bMAx = max(bMAx, p.second);
		for(pair<char, int> p: contadorMaximos) if(p.second == bMAx) maximos.push_back(p.first);
		
		// elegir al azar entre los maximos de lo anterior
		char actual = maximos[ rand() % maximos.size() ];


		for(int j=0; j<n; j++) if(setGen[j][col] != actual) hamming[j]++;	
		sol += actual;
	}

	int calidad = 0;
	for(int h: hamming) if(  (double)h / m  >= th) calidad++;

	return calidad;
}