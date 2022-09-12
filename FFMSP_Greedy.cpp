#include <iostream>
#include <vector>
#include <fstream>
#include <string.h>
#include <map>
#include <random>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <chrono>

using namespace std;


int greedy(vector<string> &setGen, double th, int n, int m);

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


	auto start1 = chrono::high_resolution_clock::now();
	greedy(set, threshold, n, m);
	auto finish1 = chrono::high_resolution_clock::now();
	auto duration1 = chrono::duration_cast<chrono::milliseconds>(finish1-start1).count();
	cout << duration1 << endl;
	return 0;
}

int greedy(vector<string> &setGen, double th, int n, int m){
	vector<char> bases = {'A', 'C', 'G', 'T'};
	vector<int> hamming(n);
	map<char, int> cumpleTH; // contador cantidad strings donde dist hamming >= th*m

	vector<map<char, int>> contador(m);	// contar cuanto se repiten las bases en cada columna
	for(int col=0; col<m; col++)
		for(int j=0; j<n; j++) 
			contador[col][ setGen[j][col] ]++;

	//ordenar los indices de acuerdo al maximo en la columna, mayor a menor
	vector<pair<int, int>> indices;
	for(int i=0; i<m; i++){
		int mayor = -1;
		for(auto par: contador[i]) mayor = max(par.second, mayor);
		indices.push_back( {mayor, i} );
	}
	sort(indices.begin(), indices.end(), greater<pair<int, int>>());

	string sol(m, ' ');
	int columnasListas = 0;
	for(auto par: indices){  // para cada columna	
		int col = par.second;
		columnasListas++;
		cumpleTH['A'] = 0;
		cumpleTH['C'] = 0;
		cumpleTH['G'] = 0;
		cumpleTH['T'] = 0;

		for(char base: bases){	 // para cada base a testear
			for(int j=0; j<n; j++){	 //para cada base en la columna
				int hammingAux = hamming[j];
				if(setGen[j][col] != base) hammingAux += 1;
				if( hammingAux >= floor(th*columnasListas) ) cumpleTH[base]++;
			}
		}

		// encontrar a los maximos de cumpleTH
		int cumpleThMAx = -1;
		vector<char> maximos;
		for(auto par: cumpleTH) cumpleThMAx = max(cumpleThMAx, par.second);
		for(auto par: cumpleTH) if(par.second == cumpleThMAx) maximos.push_back(par.first);

		// elegir los minimos de cuanto se repiten los maximos en la columna
		int repMin = n+1;
		vector<char> minRepeticion;
		for(char c: maximos) repMin = min(repMin, contador[col][c]);
		for(char c: maximos) if(contador[col][c] == repMin) minRepeticion.push_back(c);
		
		// elegir al azar entre los minimos y actualizar dist hamming
		sol[col] = minRepeticion[ rand() % minRepeticion.size() ];
		for(int j=0; j<n; j++) if(setGen[j][col] != sol[col]) hamming[j]++;	
	}

	// calcular calidad de la solucion
	int calidad = 0;
	for(int h: hamming) if( h >= floor(th*m) ) calidad++;

	return calidad;
}
