#include <iostream>
#include <vector>
#include <fstream>
#include <string.h>
#include <ctime>
#include <unordered_map>
#include <algorithm>
#include <random>
#include <set>
#include <chrono>

#include <atomic>
#include <thread>

using namespace std;
using namespace std::chrono;

minstd_rand rng;

double pg = 1, determinismo = 0;
int nb = 10, tSopa = 50;

int torneos = 10, torneos2 = 0, poblacion = 100;

double pm = 0.30, pm2 = 0.005;
double pc = 0.50, pc2 = 0.50;
double pt = 0.20, pt2 = 0.10;
double ef = 0.9;



class GRASP{
	private:
		vector<char> bases = {'A', 'C', 'G', 'T'};
		vector<string> dataset;
		vector<int> hamming, indices;
		vector<unordered_map<char, int>> contador;
		unordered_map<char, int> calidadBase;
		double th, det;
		int n, m;

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
						if(base == sol[col] && rng()%m == 0) continue;
						
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
		GRASP(string instancia, double threshold, double determinismo){	
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
			vector<int> hamming2(dataset.size());
			for(int i=0; i<dataset.size(); i++){
				for(int j=0; j<dataset[i].size(); j++){
					if(dataset[i][j] != sol[j]) hamming2[i]++;
				}
			}

			int cal = 0;
			for(auto h: hamming2) if(h >= (int)(th*m)) cal++;

			return cal;
		}
};


class Bacteria{
	private:
		char bases[4] = {'A', 'C', 'G', 'T'};
		int tamSol;

	public:
		string solucion;
		int fitness;

		Bacteria(string sol, int fit){
			solucion = sol;
			fitness = fit;
			tamSol = solucion.size();
		}

		void mutar(){
			for(int i=0; i<tamSol; i++) if(rng()%10000 < pm2*10000) solucion[i] = bases[rng()%4];
			/*
			int tamMut = porcentajeMut*tamSol;
			int ini = min((int)(rng()%tamSol), tamSol-tamMut);
			int fin = ini + rng()%tamMut;

			for(int i=ini; i<fin; i++) solucion[i] = bases[rng()%4];
			*/
		}
 
		void recibir(string donacion){
			for(int i=0; i<tamSol; i++) if(rng()%100 < pc2*100) solucion[i] = donacion[i];

			/*
			for(int i=0; i<tamSol; i++){
				if(rng()%100 < pa*100){

					solucion[i] = donacion[i];

					int atras = i-1;
					while(rng()%100 < pr*100 && atras >= 0){
						solucion[atras] = donacion[atras];
						atras -= 1;
					}
					
					while(rng()%100 < pr*100 && i < tamSol){
						solucion[i] = donacion[i];
						i += 1;
					}
				}
			}
			*/
		}

		void absorber(string donacion){
			for(int i=0; i<tamSol; i++) if(rng()%100 < pt2*100) solucion[i] = donacion[i];
		}

		string lazaro(string sol1, string sol2){
			string solVieja = solucion;

			for(int i=0; i<tamSol; i++){
				if(rand()%2) solucion[i] = sol1[i];
				else solucion[i] = sol2[i];
			}

			mutar();
			
			return solVieja;
		}
};


class Sim{
	private:
		GRASP *grasp;
		int antibiotico;
		vector<Bacteria> bacterias;
		vector<int> donadoras, receptoras;
		vector<string> sopa;
		

		void generarPoblacion(){
			pair<string, int> p;

			for(int i=0; i<poblacion; i++){
				if(rng()%100 < pg*100) p = grasp->generarSol( rng()%nb);
				else p = grasp->generarSolRandom();

				bacterias.push_back( Bacteria(p.first, p.second) );
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
			for(int i: receptoras){
				if(donadoras.empty()) break;

				if(rng()%100 < pc*100){
					string donacion = bacterias[ donadoras[rng()%donadoras.size()] ].solucion;
					bacterias[i].recibir(donacion);
				}
			}
		}

		void transformacion(){
			for(Bacteria &b: bacterias){
				if(rng()%100 < pt*100) {
					string donacion = sopa[rng()%sopa.size()];
					b.absorber(donacion);
				}
			}
		}

		void administrarAntibiotico(){
			if(donadoras.empty()) return;
			
			for(int i: receptoras){
				if(bacterias[i].fitness <= antibiotico && rng()%100 < ef*100){
					int don1 = donadoras[rng()%donadoras.size()];
					int don2 = donadoras[rng()%donadoras.size()];

					string sol1 = bacterias[don1].solucion;
					string sol2 = bacterias[don2].solucion;

					sopa[rng()%sopa.size()] = bacterias[i].lazaro(sol1, sol2);

					/*
					
					int pos = rng()%sopa.size();
					string b1 = bacterias[i].lazaro(sol1, sol2);

					for(int j=0; j<b1.size(); j++){
						if(rng()%2) sopa[pos][j] = b1[j]; 
					}
					*/
					
				}
			}
			
			receptoras.clear();
			donadoras.clear();
		}

		void variacionForzada(){
			for(int i=0; i<tSopa; i++){
				sopa.clear();
				auto a = grasp->generarSolRandom();
				sopa.push_back(a.first); 
			}

			for(Bacteria &b: bacterias) b.mutar();
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

				sopa[rng()%sopa.size()] = bacterias[h1].lazaro(p1, p2);
				sopa[rng()%sopa.size()] = bacterias[h2].lazaro(p1, p2);

				int nuevoFitness = grasp->calcularCalidad( bacterias[h1].solucion );
				bacterias[h1].fitness = nuevoFitness;

				nuevoFitness = grasp->calcularCalidad( bacterias[h2].solucion );
				bacterias[h2].fitness = nuevoFitness;
			}
		}

	public:
		Sim(string instancia, double threshold, double determinismo){
			grasp = new GRASP(instancia, threshold, determinismo);
			antibiotico = 0;

			for(int i=0; i<tSopa; i++){
				auto a = grasp->generarSolRandom();
				sopa.push_back(a.first); 
			}
		}

		int iniciar(){
			int ti = time(NULL);
			generarPoblacion();

			int mejorFit = 0;

			while(time(NULL) - ti <= 90){

				crearAntibiotico();

				
				transformacion();
				mutacion();
				
				clasificacion();
				conjugacion();


				int fit = evaluarFitness();
				cout << fit << " " << antibiotico << endl;
				if(fit > mejorFit){
					cout << fit << endl;
					mejorFit = fit;
				}


				torneo();
				//if(fit <= antibiotico) variacionForzada();

				administrarAntibiotico();

			}
			cout << "fin" << endl;
			return mejorFit;
		}
};


atomic_int promedio;
void funcionHilos(string instancia, double threshold, double determinismo){
	Sim s(instancia, threshold, determinismo);
	promedio += s.iniciar();
}


int main(int argc, char *argv[]){
	promedio = 0;
	
	auto now = high_resolution_clock::now();
	auto nanos = duration_cast<nanoseconds>(now.time_since_epoch()).count();

	rng.seed(nanos);


	string instancia = "instancias/100-300-001.txt";
	double threshold = 0.80;
	int tiempoMaximo = 90, intentosExtra = 20, poblacion = 100;
	
	for(int i=0; i<argc; i++){
		if( !strcmp(argv[i], "-i" ) ) instancia = argv[i+1];
		if( !strcmp(argv[i], "-th") ) threshold = atof(argv[i+1]);
		if( !strcmp(argv[i], "-d" ) ) determinismo = atof(argv[i+1]);
		if( !strcmp(argv[i], "-t" ) ) tiempoMaximo = atoi(argv[i+1]);
		if( !strcmp(argv[i], "-ie" ) ) intentosExtra = atoi(argv[i+1]);
		if( !strcmp(argv[i], "-p" ) ) poblacion = atoi(argv[i+1]);
	}

	Sim s(instancia, threshold, determinismo);
	int cal = s.iniciar();
	cout << "r: " << cal << endl;

	/*
	int hilos = 10;
	vector<thread> t;
	vector<string> genomas = {"100-300", "100-600", "100-800", "200-300", "200-600", "200-800"};
	vector<string> genomas2 = {"-001", "-002", "-003", "-004", "-005", "-006", "-007", "-008", "-009", "-010"};
	
	for(int i=0; i<hilos; i++){
		string instancia = "instancias/" + genomas[0] + genomas2[i] + ".txt";
		thread aux(funcionHilos, instancia, threshold, determinismo);
		t.push_back( move(aux) );
	}
	for(int i=0; i<t.size(); i++) t[i].join();

	cout << "Promedio: " << (double)(promedio)/hilos << endl;

	*/
	return 0;
}
