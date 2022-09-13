#include <vector>
#include <string.h>
#include <map>
#include <cmath>
#include <algorithm>
using namespace std;


int greedy_random(vector<string> &setGen, double th, int n, int m, double a){
	vector<char> bases = {'A', 'C', 'G', 'T'};
	vector<int> hamming(n);
	map<char, int> cumpleTH; // contador cantidad strings donde porcentaje hamming >= th

	vector<map<char, int>> contador(m);	// contar cuanto se repiten las bases en cada columna
	for(int col=0; col<m; col++) 
		for(int j=0; j<n; j++) 
			contador[col][ setGen[j][col] ]++;

	//ordenar los indices de acuerdo al menor en la columna
	vector<pair<int, int>> indices;
	for(int i=0; i<m; i++){
		int menor = n+1;
		for(auto par: contador[i]) menor = min(par.second, menor);
		indices.push_back( {menor, i} );
	}
	sort(indices.begin(), indices.end());

	string sol(m, ' ');
	int columnasListas = 0;
	for(auto par: indices){  // para cada columna	
		int col = par.second;
		columnasListas++;
		cumpleTH['A'] = 0;
		cumpleTH['C'] = 0;
		cumpleTH['G'] = 0;
		cumpleTH['T'] = 0;

		if(1 + rand()%100 <= a*100){
			sol[col] = bases[rand()%4];
			for(int j=0; j<n; j++) if(setGen[j][col] != sol[col]) hamming[j]++;	
			continue;
		}
		
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

	int calidad = 0;
	for(int h: hamming) if( h >= floor(th*m) ) calidad++;
	return calidad;
}

int greedy(vector<string> &setGen, double th, int n, int m){
	greedy_random(setGen, th, n, m, 0);
}
