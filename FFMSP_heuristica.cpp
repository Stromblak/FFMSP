#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <tuple>

using namespace std;


auto greedy_random(vector<string> &dataset, auto contador, auto indices, double th, int n, int m, double d){
	vector<char> bases = {'A', 'C', 'G', 'T'};
	vector<int> hamming(n);
	unordered_map<char, int> cumpleTH; // contador cantidad strings donde porcentaje hamming >= th

	string sol(m, ' ');
	int columnasListas = 0;
	for(auto par: indices){  // para cada columna	
		int col = par.second;
		columnasListas++;
		
		if(rand()%100 < (1.f-d)*100){
			sol[col] = bases[ rand()%4 ];
			for(int i=0; i<n; i++) if(dataset[i][col] != sol[col]) hamming[i]++;	
			continue;
		}
		
		for(char base: bases){	 // para cada base a testear
			cumpleTH[base] = 0;
			for(int i=0; i<n; i++){	 //para cada base en la columna
				int dif = 0;
				if(dataset[i][col] != base) dif = 1;
				if( hamming[i] + dif >= (int)(th*columnasListas) ) cumpleTH[base]++;
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
		for(int j=0; j<n; j++) if(dataset[j][col] != sol[col]) hamming[j]++;	
	}

	int calidad = 0;
	for(int h: hamming) if( h >= (int)(th*m) ) calidad++;

	return tuple<string, int>{sol, calidad};
}