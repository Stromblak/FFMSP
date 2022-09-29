#include <iostream>
#include <vector>
#include <fstream>
#include <string.h>
#include <ctime>
#include <set>

#include "FFMSP_heuristica.cpp"
using namespace std;

int GRASP(vector<string> &dataset, double th, double det, int tMAx);

int main(int argc, char *argv[]){
	string instancia = "100-300-001.txt";
	double threshold = 0.80, determinismo = 0.9;
	int tiempoMax = 1;
	
	srand(time(NULL));

	for(int i=0; i<argc; i++){
		if( !strcmp(argv[i], "-i" ) ) instancia = argv[i+1];
		if( !strcmp(argv[i], "-th") ) threshold = atof(argv[i+1]);
		if( !strcmp(argv[i], "-d" ) ) determinismo = atof(argv[i+1]);
		if( !strcmp(argv[i], "-t" ) ) tiempoMax = atoi(argv[i+1]);
	}

	ifstream archivo(instancia);
	vector<string> set;
	string gen;
 
	while(archivo >> gen) set.push_back(gen);
	archivo.close();


	cout << GRASP(set, threshold, determinismo, tiempoMax) << endl;


	return 0;
}

pair<string, int> busquedaLocal(vector<string> &dataset, vector<pair<int, int>> &indices, vector<int> &hamming, string sol, int cal, int threshold){
	int mejora = 3;
	int th = threshold*dataset[0].size();
	int n = dataset.size();
	unordered_map<char, int> cumpleTH;

	while(mejora){
		mejora--;

		int columnasListas = 0;
		for(auto par: indices){  // para cada columna	
			int col = par.second;
			columnasListas++;

			set<char> bases = {'A', 'C', 'G', 'T'};
			//bases.erase(sol[col]);

			for(int i=0; i<n; i++) if(dataset[i][col] != sol[col]) hamming[i]--;

			
			for(char base: bases){	 // para cada base a testear
				cumpleTH[base] = 0;
				for(int i=0; i<n; i++){	 //para cada base en la columna
					int dif = 0;
					if(dataset[i][col] != base) dif = 1;
					if( hamming[i] + dif >= th ) cumpleTH[base]++;
				}
			}

			int cumpleThMAx = -1;
			vector<char> maximos;
			for(auto par: cumpleTH) cumpleThMAx = max(cumpleThMAx, par.second);
			for(auto par: cumpleTH) if(par.second == cumpleThMAx) maximos.push_back(par.first);

			sol[col] = maximos[ rand() % maximos.size() ];
			for(int i=0; i<n; i++) if(dataset[i][col] != sol[col]) hamming[i]++;

		}

		int calidadNueva = 0;
		for(int h: hamming) if( h >= th ) calidadNueva++;

		if(calidadNueva > cal){
			mejora = 5;
			cal = calidadNueva;
		}
	}

	return pair<string, int>{sol, cal};
}


int GRASP(vector<string> &dataset, double th, double det, int tMAx){
	int ti = time(NULL);
	int n = dataset.size();
	int m = dataset[0].size();

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


	// GRASP
	string sol;
	int cal = -1;

	while(time(NULL) - ti <= tMAx && cal != n){

		auto aux = greedy_random(dataset, contador, indices, th, det);
		string solAux = get<0>(aux);
		int calAux = get<1>(aux);
		vector<int> hammingAux = get<2>(aux);


		// Busqueda local
		auto aux2 = busquedaLocal(dataset, indices, hammingAux, solAux, calAux, th);

		if(calAux > cal){
			cal = calAux;
			sol = solAux;
			cout << "Nueva solucion: " << cal << endl;
			cout << "Tiempo: " << time(NULL) - ti << endl;
		}

	}

	return cal;
}