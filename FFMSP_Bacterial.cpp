#include <iostream>
#include <vector>
#include <fstream>
#include <string.h>
#include <ctime>
#include <unordered_map>
#include <algorithm>
#include <random>
#include <set>
#include <queue>
using namespace std;


// Variables Obligatorias
string instancia = "instancias/100-300-001.txt";
char bases[4] = {'A', 'C', 'G', 'T'};

double threshold = 0.80;
int tiempoMaximo = 90;
int tuning = 0;
minstd_rand rng;

// Variables
int poblacion = 500;
int torneos = 17, nb = 90;
double pg = 0.1;

double pm = 0.02;
double pc = 0.75;
double pt = 0.10;
double ef = 0.95;

double determinismo = 0;
int torneos2 = 0;



class GRASP{
	private:
		vector<string> dataset;
		vector<int> hamming, indices;
		vector<unordered_map<char, int>> contador;
		unordered_map<char, int> calidadBase;
		vector<unordered_map<char, vector<int>>> posiciones;
		double th, det;
		int n, m;

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
		}
		
		char encontrarMejorBase(int col){
			// encontrar a los maximos de calidadBase
			int calidadBaseMax = -1;
			vector<char> maximos;
			for(auto par: calidadBase) calidadBaseMax = max(calidadBaseMax, par.second);
			for(auto par: calidadBase) if(par.second == calidadBaseMax) maximos.push_back(par.first);

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

		pair<string, int> busquedaLocal(pair<string, int> solucion, int busquedas){
			string mejorSol = solucion.first, sol = mejorSol;
			int mejorCal = solucion.second;

			while(busquedas--){

				for(int col: indices){  // para cada columna
					for(int i=0; i<n; i++) if(dataset[i][col] != sol[col]) hamming[i]--;

					for(char base: bases){	 // para cada base a testear
						calidadBase[base] = 0;
						if(base == sol[col] && rng()%(2*m) == 0) continue;
						
						for(int i=0; i<n; i++){	 //para cada base en la columna
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
		GRASP(string instancia){	
			ifstream archivo(instancia);
			string gen;
 
			while(archivo >> gen) dataset.push_back(gen);
			archivo.close();

			n = dataset.size();
			m = dataset[0].size();
			th = threshold;
			det = determinismo;
			procesarIndices();
		}

		pair<string, int> generarSol(int busquedas){
			auto p = greedyRandomizado();
			return busquedaLocal(p, busquedas);
		}

		pair<string, int> generarSolRandom(){
			string sol(m, ' ');
			for(int i=0; i<sol.size(); i++) sol[i] = bases[rng()%4];
			int cal = calcularCalidad(sol);
			return pair<string, int>(sol, cal);
		}

		int calcularCalidad(string sol){
			vector<int> hamming2(n, m);
			for(int i=0; i<sol.size(); i++) for(int j: posiciones[i][sol[i]]) hamming2[j]--;

			int cal = 0;
			for(auto h: hamming2) if(h >= (int)(th*m)) cal++;

			return cal;
		}

		char baseRandom(int i){
			vector<pair<int, char>> v;
			for(auto m: posiciones[i]){
				for(char c: bases){
					v.push_back( {300 - posiciones[i][c].size(), c} );
				}
			}

			sort(v.begin(), v.end());
			reverse(v.begin(), v.end());

			int r = rng()%900;

			int acum = 0;
			for(auto a: v){
				acum += a.first;
				if(r <= acum) return a.second;
			}

			return bases[rng()%4];
		}
};


class Bacteria{
	public:
		GRASP *grasp;
		string solucion;
		int fitness;

		Bacteria(string sol, int fit, GRASP *g){
			solucion = sol;
			fitness = fit;
			grasp = g;
		}

		void mutar(){
			for(int i=0; i<solucion.size(); i++){
				if(rng()%100 < pm*100){
					char c = bases[rng()%4];
					while(c == solucion[i]) c = bases[rng()%4];

					if(rng()%100 < 100) c = grasp->baseRandom(i);
					solucion[i] = c;
				}
			}
		}

		void conjugar(string donacion){
			for(int i=0; i<solucion.size(); i++) if(rng()%100 < pc*100) solucion[i] = donacion[i];
		}

		void transmormar(string donacion){
			for(int i=0; i<solucion.size(); i++) if(rng()%100 < pt*100) solucion[i] = donacion[i];
		}

		string lazaro(string sol1, string sol2){
			string solVieja = solucion;

			for(int i=0; i<solucion.size(); i++){
				if(rand()%2) solucion[i] = sol1[i];
				else solucion[i] = sol2[i];
			}

			mutar();
			
			return solVieja;
		}

		string comer(string donacion){
			for(int i=0; i<solucion.size(); i++) if(rng()%2) solucion[i] = donacion[i];
			return solucion;
		}
};


class Sim{
	private:
		GRASP *grasp;
		int antibiotico, mejor;
		vector<Bacteria> bacterias;
		vector<int> donadoras, receptoras;
		queue<string> sopa;
		
		void generarPoblacion(){
			pair<string, int> p;

			for(int i=0; i<poblacion; i++){
				if(rng()%100 < pg*100) p = grasp->generarSol( rng()%nb);
				else p = grasp->generarSolRandom();

				bacterias.push_back( Bacteria(p.first, p.second, grasp) );
			}
		}

		int evaluarFitness(){
			int mejorFitness = 0;

			for(Bacteria &b: bacterias){
				int nuevoFitness = grasp->calcularCalidad( b.solucion );
				b.fitness = nuevoFitness;
				mejorFitness = max(nuevoFitness, mejorFitness);
			}
			return mejorFitness;
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
			for(Bacteria &b: bacterias) if(rng()%100 < pm*100) b.mutar();
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
				if(rng()%100 < pt*100) bacterias[ rng()%bacterias.size() ].transmormar(sopa.front());
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
			}else if(0){
				for(Bacteria &b: bacterias){
					int b1 = rng()%bacterias.size();

					if(b.fitness <= bacterias[b1].fitness) continue;

					bacterias[b1].solucion = b.comer( bacterias[b1].solucion );
					bacterias[b1].mutar();

				}
			}

			receptoras.clear();
			donadoras.clear();
		}

		void torneo(){
			for(int i=0; i<torneos2; i++){
				int b1 = rng()%poblacion;
				int b2 = rng()%poblacion;
				int b3 = rng()%poblacion;
				int b4 = rng()%poblacion;

				int h1, h2;
				string p1, p2;


				if(bacterias[b1].fitness > bacterias[b2].fitness){
					p1 = bacterias[b1].solucion;
					h1 = b2;
				}else{
					p1 = bacterias[b2].solucion;
					h1 = b1;
				}

				if(bacterias[b3].fitness > bacterias[b4].fitness){
					p2 = bacterias[b3].solucion;
					h2 = b4;
				}else{			
					p2 = bacterias[b4].solucion;
					h2 = b3;
				}

				sopa.push( bacterias[h1].lazaro(p1, p2) );
				sopa.push( bacterias[h2].lazaro(p1, p2) );

				int nuevoFitness = grasp->calcularCalidad( bacterias[h1].solucion );
				bacterias[h1].fitness = nuevoFitness;

				nuevoFitness = grasp->calcularCalidad( bacterias[h2].solucion );
				bacterias[h2].fitness = nuevoFitness;
			}
		}

	public:
		Sim(string instancia){
			grasp = new GRASP(instancia);
			antibiotico = 0;
			mejor = 0;
		}

		int iniciar(){
			int ti = time(NULL);
			generarPoblacion();

			while(time(NULL) - ti <= tiempoMaximo){
				crearAntibiotico();

				transformacion();
				mutacion();

				clasificacion();
				conjugacion();				

				torneo();
				int fit = evaluarFitness();		

				if(fit > mejor){
					if(!tuning) cout << fit << endl;
					mejor = fit;
				}

				administrarAntibiotico();

				//cout << fit << " " << antibiotico << endl;
			}
			
			if(!tuning) cout << "fin " << mejor << endl;
			else cout << -mejor << endl;
			return mejor;
		}
};


int main2(int argc, char *argv[]){
	rng.seed(time(NULL));

	for(int i=0; i<argc; i++){
		if( !strcmp(argv[i], "-i" ) ) instancia = argv[i+1];
		if( !strcmp(argv[i], "-th") ) threshold = atof(argv[i+1]);
		if( !strcmp(argv[i], "-d" ) ) determinismo = atof(argv[i+1]);
		if( !strcmp(argv[i], "-t" ) ) tiempoMaximo = atof(argv[i+1]);
		if( !strcmp(argv[i], "-tuning" ) ) tuning = atof(argv[i+1]);

		// Variables
		if( !strcmp(argv[i], "-p" ) ) poblacion = atoi(argv[i+1]);

		if( !strcmp(argv[i], "-pm" ) ) pm = atof(argv[i+1]);

		if( !strcmp(argv[i], "-pc" ) ) pc = atof(argv[i+1]);

		if( !strcmp(argv[i], "-pt" ) ) pt = atof(argv[i+1]);

		if( !strcmp(argv[i], "-pg" ) ) pg = atof(argv[i+1]);
		if( !strcmp(argv[i], "-nb" ) ) nb = atoi(argv[i+1]);

		if( !strcmp(argv[i], "-torneos" ) ) torneos2 = atoi(argv[i+1]);

	}

	Sim s(instancia);
	s.iniciar();

	return 0;
}