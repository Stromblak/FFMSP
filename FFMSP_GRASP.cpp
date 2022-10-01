#include <iostream>
#include <vector>
#include <fstream>
#include <string.h>
#include <ctime>
#include <unordered_map>
#include <algorithm>
#include <random>

#include <thread>
#include <atomic>

using namespace std;


atomic_int suma = 0;
class GRASP{
	private:
		vector<char> bases = {'A', 'C', 'G', 'T'};
		vector<string> dataset;
		vector<int> hamming, indices;
		vector<unordered_map<char, int>> contador;
		unordered_map<char, int> cumpleTH;
		double th, det;
		int n, m, tMAx, intentosExtra;
		minstd_rand rng;

		void procesarIndices(){
			contador = vector<unordered_map<char, int>>(m);
			for(int col=0; col<m; col++)
				for(int j=0; j<n; j++)
					contador[col][ dataset[j][col] ]++;

			vector<pair<int, int>> indicesAux(m);
			for(int i=0; i<m; i++){
				int mayor = -1;
				for(auto par: contador[i]) mayor = max(par.second, mayor);
				indicesAux[i] = {mayor, i};
			}
			sort(indicesAux.begin(), indicesAux.end(), greater<pair<int, int>>());

			indices = vector<int>(m);
			for(int i=0; i<m; i++) indices[i] = indicesAux[i].second;
		}

		pair<string, int> greedyRandomizado(){
			hamming = vector<int>(n);
			string sol(m, ' ');
			
			int columnasListas = 1;
			for(int col: indices){  // para cada columna			
				if(rng()%100 < det*100){
					for(char base: bases){	 // para cada base a testear
						cumpleTH[base] = 0;
						for(int i=0; i<n; i++){	 //para cada base en la columna
							int dif = 0;
							if(dataset[i][col] != base) dif = 1;
							if( hamming[i] + dif >= (int)(th*columnasListas) ) cumpleTH[base]++;
						}
					}
					sol[col] = encontrarMejorBase(col);
					
				}
				else sol[col] = bases[ rng()%4 ];
				
				for(int i=0; i<n; i++) if(dataset[i][col] != sol[col]) hamming[i]++;	
				columnasListas++;
			}

			int calidad = 0;
			for(int h: hamming) if( h >= (int)(th*m) ) calidad++;

			return pair<string, int>{sol, calidad};
		}

		char encontrarMejorBase(int col){
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
			
			return minRepeticion[ rng() % minRepeticion.size() ];
		}

		pair<string, int> busquedaLocal(pair<string, int> solucion){
			int intentos = 5, th_m = th*m;

			string sol = solucion.first;
			int cal = solucion.second;

			while(intentos){
				intentos -= 1;

				int columnasListas = 0;
				for(int col: indices){  // para cada columna
					for(int i=0; i<n; i++) if(dataset[i][col] != sol[col]) hamming[i]--;

					for(char base: bases){	 // para cada base a testear
						cumpleTH[base] = 0;
						if(base == sol[col] && rng()%600 == 0) continue;
						for(int i=0; i<n; i++){	 //para cada base en la columna
							int dif = 0;
							if(dataset[i][col] != base) dif = 1;
							if(hamming[i] + dif >= th_m) cumpleTH[base]++;
						}
					}

					sol[col] = encontrarMejorBase(col);
					for(int i=0; i<n; i++) if(dataset[i][col] != sol[col]) hamming[i]++;
					columnasListas++;
				}

				int calidadNueva = 0;
				for(int h: hamming) if(h >= th_m) calidadNueva += 1;

				if(calidadNueva > cal){
					intentos += intentosExtra;
					cal = calidadNueva;
				}
			}

			return pair<string, int>{sol, cal};
		}

	public:
		GRASP(string instancia, double threshold, double determinismo, int tiempoMaximo, int intentos){	
			ifstream archivo(instancia);
			string gen;
 
			while(archivo >> gen) dataset.push_back(gen);
			archivo.close();

			n = dataset.size();
			m = dataset[0].size();
			th = threshold;
			det = determinismo;
			tMAx = tiempoMaximo;
			intentosExtra = intentos;
			rng.seed(rand());
		}

		int iniciar(){
			int ti = time(NULL);
			pair<string, int> solActual = {" ", -1};
			procesarIndices();

			while(time(NULL) - ti <= tMAx && solActual.second != n){
				pair<string, int> solNueva = busquedaLocal( greedyRandomizado() );

				if(solNueva.second > solActual.second){
					solActual = solNueva;
					cout << "Nueva solucion: " << solNueva.second << "  Tiempo: " << time(NULL) - ti << endl;
				}
			}

			suma += solActual.second;
			return solActual.second;
		}
};


int main(int argc, char *argv[]){
	string instancia = "100-300-001.txt";
	double threshold = 0.80, determinismo = 0.8;
	int tiempoMaximo = 30, intentosExtra = 20;
	
	for(int i=0; i<argc; i++){
		if( !strcmp(argv[i], "-i" ) ) instancia = argv[i+1];
		if( !strcmp(argv[i], "-th") ) threshold = atof(argv[i+1]);
		if( !strcmp(argv[i], "-d" ) ) determinismo = atof(argv[i+1]);
		if( !strcmp(argv[i], "-t" ) ) tiempoMaximo = atoi(argv[i+1]);
		if( !strcmp(argv[i], "-e" ) ) intentosExtra = atoi(argv[i+1]);
	}

	srand(time(NULL));

	vector<GRASP> g;
	vector<thread> t;
	int hilos = 10;

	for(int i=0; i<hilos; i++){
		g.push_back( GRASP(instancia, threshold, determinismo, tiempoMaximo, intentosExtra) );
		thread aux(&GRASP::iniciar, g[i]);
		t.push_back( move(aux) );
	}

	for(int i=0; i<hilos; i++) t[i].join();

	cout << (double)(suma)/hilos << endl;

	return 0;
}