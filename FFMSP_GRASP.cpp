#include <iostream>
#include <vector>
#include <fstream>
#include <string.h>
#include <ctime>
#include <set>
#include <string>
#include <unordered_map>
#include <algorithm>

using namespace std;


class GRASP{
	private:
		vector<char> bases = {'A', 'C', 'G', 'T'};
		vector<string> dataset;
		vector<int> hamming, indices;
		vector<unordered_map<char, int>> contador;
		unordered_map<char, int> cumpleTH; // contador cantidad strings donde porcentaje hamming >= th
		double th, det;
		int n, m, tMAx;

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
			
			int columnasListas = 0;
			for(int col: indices){  // para cada columna
				columnasListas++;
				
				if(rand()%100 < (1.f-det)*100){
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
				sol[col] = encontrarMejorBase(col);
				for(int j=0; j<n; j++) if(dataset[j][col] != sol[col]) hamming[j]++;	
			}

			int calidad = 0;
			for(int h: hamming) if( h >= (int)(th*m) ) calidad++;

			return pair<string, int>{sol, calidad};
		}

		pair<string, int> busquedaLocal(pair<string, int> solucion){
			int intentosMax = 3;
			int intentos = intentosMax;
			int th_m = th*m;

			string sol = solucion.first;
			int cal = solucion.second;

			while(intentos){
				intentos -= 1;

				int columnasListas = 0;
				for(int col: indices){  // para cada columna
					columnasListas++;

					for(int i=0; i<n; i++) if(dataset[i][col] != sol[col]) hamming[i]--;

					for(char base: bases){	 // para cada base a testear
						if(base == sol[col] && rand()%3) continue;

						cumpleTH[base] = 0;
						for(int i=0; i<n; i++){	 //para cada base en la columna
							int dif = 0;
							if(dataset[i][col] != base) dif = 1;
							if(hamming[i] + dif >= th_m) cumpleTH[base]++;
						}
					}

					sol[col] = encontrarMejorBase(col);
					for(int i=0; i<n; i++) if(dataset[i][col] != sol[col]) hamming[i]++;
				}

				int calidadNueva = 0;
				for(int h: hamming) if( h >= th_m ) calidadNueva++;

				if(calidadNueva > cal){
					intentos = intentosMax;
					cal = calidadNueva;
				}
			}

			return pair<string, int>{sol, cal};
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
			
			return minRepeticion[ rand() % minRepeticion.size() ];
		}

	public:
		GRASP(string instancia, double threshold, double determinismo, int tiempoMaximo){	
			ifstream archivo(instancia);
			string gen;
 
			while(archivo >> gen) dataset.push_back(gen);
			archivo.close();

			n = dataset.size();
			m = dataset[0].size();
			th = threshold;
			det = determinismo;
			tMAx = tiempoMaximo;
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

			return solActual.second;
		}
};


int main(int argc, char *argv[]){
	string instancia = "100-300-001.txt";
	double threshold = 0.80, determinismo = 0.98;
	int tiempoMaximo = 10;
	
	srand(time(NULL));

	for(int i=0; i<argc; i++){
		if( !strcmp(argv[i], "-i" ) ) instancia = argv[i+1];
		if( !strcmp(argv[i], "-th") ) threshold = atof(argv[i+1]);
		if( !strcmp(argv[i], "-d" ) ) determinismo = atof(argv[i+1]);
		if( !strcmp(argv[i], "-t" ) ) tiempoMaximo = atoi(argv[i+1]);
	}

	GRASP grasp = GRASP(instancia, threshold, determinismo, tiempoMaximo);
	cout << grasp.iniciar() << endl;

	return 0;
}
