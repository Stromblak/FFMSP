#include <iostream>
#include <vector>
#include <fstream>
#include <string.h>
#include <ctime>
#include <unordered_map>
#include <algorithm>
#include <random>
#include <queue>
using namespace std;


// Parametros
string instancia = "instancias/200-800-001.txt";
double threshold = 0.80;
int tiempoMaximo = 90;
int tuning = 0, update = 0;
minstd_rand rng;

// Variables
double determinismo = 0.9;
int poblacion = 500;
int torneos = 17;
double pg = 0.05;
int nb = 23;

double pm = 0.005;
double pc = 0.75;
double pt = 0.10;
double ef = 0.95;
double bl = 0.05;


class Dataset{
	public:
		int n;

	private:
		vector<char> bases;
		vector<string> dataset;
		vector<int> hamming, indices;
		unordered_map<char, int> calidadBase;
		vector<unordered_map<char, int>> contador;
		vector<unordered_map<char, vector<int>>> posiciones;
		vector<vector<pair<int, char>>> pesos;
		double th, det;
		int m, th_m;

		void procesarIndices(){
			contador = vector<unordered_map<char, int>>(m);
			posiciones = vector<unordered_map<char, vector<int>>>(m);

			for(int col=0; col<m; col++){
				for(int j=0; j<n; j++){
					posiciones[col][dataset[j][col]].push_back(j);
				}
				for(char c: bases){
					contador[col][c] = posiciones[col][c].size();
				}
			}

			vector<pair<int, int>> indicesAux(m);
			for(int i=0; i<m; i++){
				int mayor = -1;
				for(auto par: contador[i]) mayor = max(par.second, mayor);
				indicesAux[i] = {mayor, i};
			}
			sort(indicesAux.begin(), indicesAux.end(), greater<pair<int, int>>());

			indices = vector<int>(m);
			for(int i=0; i<m; i++) indices[i] = indicesAux[i].second;

			pesos = vector<vector<pair<int, char>>>(m);
			for(int col=0; col<m; col++){
				for(char c: bases){
					pesos[col].push_back( {m - posiciones[col][c].size(), c} );
				}
			}
		}
		
		char encontrarMejorBase(int col){
			int calidadBaseMax = -1;
			vector<char> maximos;
			for(auto par: calidadBase) calidadBaseMax = max(calidadBaseMax, par.second);
			for(auto par: calidadBase) if(par.second == calidadBaseMax) maximos.push_back(par.first);

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
			for(int col: indices){
				if(rng()%100 < det*100){
					for(char base: bases){
						calidadBase[base] = 0;
						for(int i=0; i<n; i++){
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

		pair<string, int> busquedaLocal(pair<string, int> solucion, int busquedas){
			string mejorSol = solucion.first, sol = mejorSol;
			int mejorCal = solucion.second;

			while(busquedas--){

				for(int col: indices){
					for(int i=0; i<n; i++) if(dataset[i][col] != sol[col]) hamming[i]--;

					for(char base: bases){
						calidadBase[base] = 0;
						if(base == sol[col] && rng()%(2*m) == 0) continue;
						
						for(int i=0; i<n; i++){
							int dif = 0;
							if(base != dataset[i][col]) dif = 1;
							if(hamming[i] + dif >= (int)(th*m)) calidadBase[base]++;
						}
					}
					sol[col] = encontrarMejorBase(col);
					
					for(int i=0; i<n; i++) if(dataset[i][col] != sol[col]) hamming[i]++;
				}
									
				if(calidadBase[ sol[indices.back()] ] > mejorCal){
					mejorSol = sol;
					mejorCal = calidadBase[ sol[indices.back()] ];
				}
			}

			return pair<string, int>{mejorSol, mejorCal};
		}


	public:
		Dataset(string instancia){	
			ifstream archivo(instancia);
			string gen;
 
			while(archivo >> gen) dataset.push_back(gen);
			archivo.close();

			n = dataset.size();
			m = dataset[0].size();
			th = threshold;
			det = determinismo;
			th_m = th*m;
			bases = vector<char>{'A', 'C', 'G', 'T'};
			procesarIndices();
		}

		pair<string, int> miniGrasp(){
			auto p = greedyRandomizado();
			return busquedaLocal(p, nb);
		}

		pair<string, int> solRandom(){
			string sol(m, ' ');
			for(char &c: sol) c = bases[rng()%4];
			int cal = calcularCalidad(sol);
			return pair<string, int>(sol, cal);
		}
		
		pair<string, int> busquedaLocalBacteria(string sol){
			vector<int> hamming2(n, m);
			for(int i=0; i<m; i++) for(int j: posiciones[i][sol[i]]) hamming2[j]--;

			for(int col: indices){
				if( !(rng()%100 < bl*100) ) continue;

				for(int i=0; i<n; i++) if(dataset[i][col] != sol[col]) hamming2[i]--;

				for(char base: bases){
					calidadBase[base] = 0;
					
					for(int i=0; i<n; i++){
						int dif = 0;
						if(base != dataset[i][col]) dif = 1;
						if(hamming2[i] + dif >= (int)(th*m)) calidadBase[base]++;
					}
				}
				sol[col] = encontrarMejorBase(col);
				
				for(int i=0; i<n; i++) if(dataset[i][col] != sol[col]) hamming2[i]++;
			}

			int cal = 0;
			for(int h: hamming2) if(h >= th_m) cal++;

			return pair<string, int>{sol, cal};
		}

		int calcularCalidad(string sol){
			vector<int> hamming2(n, m);
			for(int i=0; i<m; i++) for(int j: posiciones[i][sol[i]]) hamming2[j]--;

			int cal = 0;
			for(int h: hamming2) if(h >= th_m) cal++;

			return cal;
		}

		char baseDif(int i, char c){
			//int i = rng()%4;
			//char a = bases[i];
			//if(a == c) a = bases[(i+1+rng()%2)%4];
			//return a;

			char b;
			int acum = 0, r = rng()%(3*m);
			for(int j=0; j<4; j++){
				acum += pesos[i][j].first;
				if(r <= acum){
					b = pesos[i][j].second;
					if(b == c) b = pesos[i][(j+1+rng()%2)%4].second;
					break;
				}
			}

			return b;
		}
};


class Bacteria{
	private:
		Dataset *dataset;
		int cambio;

	public:
		string solucion;
		int fitness;

		Bacteria(Dataset *d){
			pair<string, int> p;
			if(rng()%100 < pg*100) p = d->miniGrasp();
			else p = d->solRandom();

			solucion = p.first;
			fitness = p.second;
			dataset = d;
			cambio = 0;
		}

		void mutar(){
			for(int i=0; i<solucion.size(); i++){
				if(rng()%1000 < pm*1000) solucion[i] = dataset->baseDif(i, solucion[i]);
			}
			cambio = 1;
		}

		void conjugar(string donacion){
			for(int i=0; i<solucion.size(); i++) if(rng()%100 < pc*100) solucion[i] = donacion[i];
			cambio = 1;
		}

		void transformar(string donacion){
			for(int i=0; i<solucion.size(); i++) if(rng()%100 < pt*100) solucion[i] = donacion[i];
			cambio = 1;
		}

		void busquedaLocal(){
			auto p = dataset->busquedaLocalBacteria(solucion);
			solucion = p.first;
			fitness = p.second;
		}

		int actualizarFitness(){
			if(cambio){
				fitness = dataset->calcularCalidad(solucion);
				cambio = 0;
			}
			return fitness;
		}

		string lazaro(string sol1, string sol2){
			string solVieja = solucion;

			for(int i=0; i<solucion.size(); i++){
				if(rng()%2) solucion[i] = sol1[i];
				else solucion[i] = sol2[i];
			}

			mutar();
			actualizarFitness();
			
			return solVieja;
		}
};


class Sim{
	private:
		Dataset *dataset;
		int antibiotico, mejor, tiempoMejor, ti;
		vector<Bacteria> bacterias;
		vector<int> donadoras, receptoras;
		queue<string> sopa;
		
		void generarPoblacion(){
			for(int i=0; i<poblacion; i++) bacterias.push_back( Bacteria(dataset) );
		}

		void evaluarFitness(){
			int mejorFit = 0;
			for(Bacteria &b: bacterias) mejorFit = max(mejorFit, b.actualizarFitness());

			if(mejorFit > mejor){
				mejor = mejorFit;
				tiempoMejor = time(NULL) - ti;
				if(!tuning) cout << "Nueva solucion: " << mejor << "  Tiempo: " << time(NULL) - ti << endl;
			}
			if(!tuning && update) cout << mejorFit << " " << antibiotico << endl;
		}

		void crearAntibiotico(){
			int antiBio = 10000;

			for(int i=0; i<torneos; i++){
				int b1 = bacterias[rng()%poblacion].fitness;
				int b2 = bacterias[rng()%poblacion].fitness;
				antiBio = min(antiBio, max(b1, b2));
			}

			antibiotico = antiBio;
		}

		void clasificacion(){
			for(int i=0; i<poblacion; i++){
				if(bacterias[i].fitness > antibiotico) donadoras.push_back(i);
				else receptoras.push_back(i);
			}
		}

		void mutacion(){
			for(Bacteria &b: bacterias) if(rng()%1000 < pm*1000) b.mutar();
		}

		void conjugacion(){
			if(donadoras.empty()) return;

			for(int i: receptoras){
				if(rng()%100 < pc*100){
					string donacion = bacterias[ donadoras[rng()%donadoras.size()] ].solucion;
					bacterias[i].conjugar(donacion);
				}
			}
		}

		void transformacion(){
			while(!sopa.empty()){
				if(rng()%100 < pt*100) bacterias[ rng()%bacterias.size() ].transformar(sopa.front());
				sopa.pop();
			}
		}

		void administrarAntibiotico(){
			if(!donadoras.empty()){
				for(int i: receptoras){
					if(bacterias[i].fitness <= antibiotico && rng()%100 < ef*100){
						int don1 = donadoras[rng()%donadoras.size()];
						int don2 = donadoras[rng()%donadoras.size()];

						string sol1 = bacterias[don1].solucion;
						string sol2 = bacterias[don2].solucion;

						sopa.push( bacterias[i].lazaro(sol1, sol2) );
					}
				}
			}

			receptoras.clear();
			donadoras.clear();
		}

		void busquedaLocal(){
			for(Bacteria &b: bacterias) if(rng()%100 < bl*100) b.busquedaLocal();
		}

	public:
		Sim(string instancia){
			ti = time(NULL);
			dataset = new Dataset(instancia);
			antibiotico = 0;
			mejor = 0;
		}

		pair<int, int> iniciar(){
			generarPoblacion();

			while(time(NULL) - ti <= tiempoMaximo && mejor != dataset->n){
				crearAntibiotico();

				busquedaLocal();
				transformacion();
				mutacion();

				clasificacion();
				conjugacion();

				evaluarFitness();
				administrarAntibiotico();
			}
			
			if(!tuning) cout << "Mejor solucion: " << mejor << "  Tiempo: " << tiempoMejor << endl;
			else cout << -mejor << endl;
			return pair<int, int>(mejor, tiempoMejor);
		}
};


int main(int argc, char *argv[]){
	rng.seed(time(NULL));

	for(int i=0; i<argc; i++){
		// Parametros
		if( !strcmp(argv[i], "-i" ) ) instancia = argv[i+1];
		if( !strcmp(argv[i], "-th") ) threshold = atof(argv[i+1]);
		if( !strcmp(argv[i], "-t" ) ) tiempoMaximo = atof(argv[i+1]);
		if( !strcmp(argv[i], "-tuning" ) ) tuning = atof(argv[i+1]);

		// Variables
		if( !strcmp(argv[i], "-d" ) ) determinismo = atof(argv[i+1]);

		if( !strcmp(argv[i], "-p" ) ) poblacion = atoi(argv[i+1]);
		if( !strcmp(argv[i], "-tor" ) ) torneos = atoi(argv[i+1]);

		if( !strcmp(argv[i], "-pg" ) ) pg = atof(argv[i+1]);
		if( !strcmp(argv[i], "-nb" ) ) nb = atoi(argv[i+1]);

		if( !strcmp(argv[i], "-pm" ) ) pm = atof(argv[i+1]);
		if( !strcmp(argv[i], "-pc" ) ) pc = atof(argv[i+1]);
		if( !strcmp(argv[i], "-pt" ) ) pt = atof(argv[i+1]);
		if( !strcmp(argv[i], "-ef" ) ) ef = atof(argv[i+1]);
		if( !strcmp(argv[i], "-bl" ) ) bl = atof(argv[i+1]);
	}

	Sim s(instancia);
	s.iniciar();

	return 0;
}