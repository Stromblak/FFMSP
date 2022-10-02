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
#include <map>

using namespace std;


class GRASP{
	private:
		vector<char> bases = {'A', 'C', 'G', 'T'};
		vector<string> dataset;
		vector<int> hamming, indices;
		vector<unordered_map<char, int>> contador;
		unordered_map<char, int> calidadBase;
		double th, det;
		int n, m, tMAx;
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
		
		char encontrarMejorBase(int col){
			// encontrar a los maximos de cumpleTH
			int calidadBaseMax = -1;
			vector<char> maximos;
			for(auto par: calidadBase) calidadBaseMax = max(calidadBaseMax, par.second);
			for(auto par: calidadBase) if(par.second == calidadBaseMax) maximos.push_back(par.first);

			if(rng()%3) return maximos[ rng() % maximos.size() ];

			// elegir los minimos de cuanto se repiten los maximos en la columna
			int repMin = n+1;
			vector<char> minRepeticion;
			for(char c: maximos) repMin = min(repMin, contador[col][c]);
			for(char c: maximos) if(contador[col][c] == repMin) minRepeticion.push_back(c);

			return minRepeticion[ rng() % minRepeticion.size() ];
		}

		pair<string, int> greedyRandomizado(){
			hamming = vector<int>(n);
			string sol(m, ' ');
			
			int columnasListas = 1;
			for(int col: indices){  // para cada columna			
				if(rng()%100 < det*100){
					for(char base: bases){	 // para cada base a testear
						calidadBase[base] = 0;
						for(int i=0; i<n; i++){	 //para cada base en la columna
							int dif = 0;
							if(dataset[i][col] != base) dif = 1;
							if( hamming[i] + dif >= (int)(th*columnasListas) ) calidadBase[base]++;
						}
					}
					sol[col] = encontrarMejorBase(col);
					
				}else sol[col] = bases[ rng()%4 ];
				
				for(int i=0; i<n; i++) if(dataset[i][col] != sol[col]) hamming[i]++;
				columnasListas++;
			}

			int calidad = calidadBase[ sol[indices.back()] ];	
			return pair<string, int>{sol, calidad};
		}

		pair<string, int> busquedaLocal(pair<string, int> solucion){
			string sol = solucion.first;
			int cal = solucion.second;
			int intentos = 5;

			vector<int>	indices2 = indices;

			while(intentos){
				int calOriginal = cal;

				for(int col: indices2){  // para cada columna 0/m-1
					for(int i=0; i<n; i++) if(dataset[i][col] != sol[col]) hamming[i]--;

					for(char base: bases){	 // para cada base a testear
						calidadBase[base] = 0;
						if(rng()%m == 0) continue;
						
						for(int i=0; i<n; i++){	 //para cada base en la columna
							int dif = 0;
							if(base != dataset[i][col]) dif = 1;
							if(hamming[i] + dif >= (int)(th*m)) calidadBase[base]++;
						}
					}
					sol[col] = encontrarMejorBase(col);
					
					for(int i=0; i<n; i++) if(dataset[i][col] != sol[col]) hamming[i]++;
				}
				
				cal = calidadBase[ sol[indices2.back()] ];						
				if(cal > calOriginal) intentos = min(20, intentos + 1);
				else intentos -= 1;
			}

			return pair<string, int>{sol, cal};
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
			rng.seed(time(NULL));
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


atomic_int suma = 0;
void funcionHilos(string instancia, double threshold, double determinismo, int tiempoMaximo){
	GRASP g(instancia, threshold, determinismo, tiempoMaximo);
	suma += g.iniciar();
}

map<string, int> mp80, mp85;
void funcionHilos2(string instancia, double threshold, double determinismo, int tiempoMaximo){
	string inst = "instancias/" + instancia + "-001.txt";

	GRASP g(inst, threshold, determinismo, tiempoMaximo);
	string aux = to_string(threshold);
	aux.resize(4);

	if(threshold == 0.80) mp80[instancia + "    " + aux] = g.iniciar();
	if(threshold == 0.85) mp85[instancia + "    " + aux] = g.iniciar();
}

int main(int argc, char *argv[]){
	string instancia = "100-300-001.txt";
	double threshold = 0.80, determinismo = 0.9;
	int tiempoMaximo = 30;
	
	for(int i=0; i<argc; i++){
		if( !strcmp(argv[i], "-i" ) ) instancia = argv[i+1];
		if( !strcmp(argv[i], "-th") ) threshold = atof(argv[i+1]);
		if( !strcmp(argv[i], "-d" ) ) determinismo = atof(argv[i+1]);
		if( !strcmp(argv[i], "-t" ) ) tiempoMaximo = atoi(argv[i+1]);
	}

	srand(time(NULL));


	vector<string> num = {"001", "002", "003", "004", "005", "006", "007", "008", "009", "010"};
	vector<thread> t;

	/*
	int hilos = 10;
	for(int i=0; i<hilos; i++){
		instancia = "instancias/200-300-" + num[i] + ".txt";
		thread aux(funcionHilos, instancia, threshold, determinismo, tiempoMaximo, intentosExtra);
		t.push_back( move(aux) );
	}
	for(int i=0; i<t.size(); i++) t[i].join();
	cout << (double)(suma)/hilos << endl;
	*/

	thread aux1(funcionHilos2, "200-300", 0.80, determinismo, tiempoMaximo);
	thread aux2(funcionHilos2, "200-300", 0.85, determinismo, tiempoMaximo);
	thread aux3(funcionHilos2, "200-600", 0.80, determinismo, tiempoMaximo);
	thread aux4(funcionHilos2, "200-600", 0.85, determinismo, tiempoMaximo);
	thread aux5(funcionHilos2, "200-800", 0.80, determinismo, tiempoMaximo);
	thread aux6(funcionHilos2, "200-800", 0.85, determinismo, tiempoMaximo);
	t.push_back( move(aux1) );
	t.push_back( move(aux2) );
	t.push_back( move(aux3) );
	t.push_back( move(aux4) );
	t.push_back( move(aux5) );
	t.push_back( move(aux6) );
	for(int i=0; i<t.size(); i++) t[i].join();

	cout << endl;
	for(auto a: mp80) cout << a.first << "    " << a.second << endl;
	for(auto a: mp85) cout << a.first << "    " << a.second << endl;


	return 0;
}


/*

200-300    0.80    82
200-600    0.80    70
200-800    0.80    61
200-300    0.85    12
200-600    0.85    7
200-800    0.85    5
*/