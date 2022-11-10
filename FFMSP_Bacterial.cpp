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


// Parametros
string instancia = "instancias/100-300-001.txt";
double threshold = 0.80;
int tiempoMaximo = 90;
int tuning = 0, update = 1;
double determinismo = 0;
minstd_rand rng;


// Variables
int poblacion = 1000;
int torneos = 17;
double pg = 0.01;
int nb = 40;

double pm = 0.005;
double pc = 0.75;
double pt = 0.10;
double ef = 0.95;

double reg = 0;
double bl = 0.02;


class Dataset{
	private:
		vector<char> bases{'A', 'C', 'G', 'T'};
		vector<string> dataset;
		vector<int> hamming, indices;
		unordered_map<char, int> calidadBase;
		vector<unordered_map<char, int>> contador;
		vector<unordered_map<char, vector<int>>> posiciones;
		vector<vector<pair<int, char>>> pesos;
		double th, det;
		int n, m, th_m;

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
			procesarIndices();
		}

		pair<string, int> generarSol(int busquedas){
			auto p = greedyRandomizado();
			return busquedaLocal(p, busquedas);
		}

		pair<string, int> generarSolRandom(){
			string sol(m, ' ');
			for(char &c: sol) c = bases[rng()%4];
			int cal = calcularCalidad(sol);
			return pair<string, int>(sol, cal);
		}

		int calcularCalidad(string sol){
			vector<int> hamming2(n, m);
			for(int i=0; i<m; i++) for(int j: posiciones[i][sol[i]]) hamming2[j]--;

			int cal = 0;
			for(int h: hamming2) if(h >= th_m) cal++;

			return cal;
		}

		pair<string, int> busquedaLocal2(string sol){
			vector<int> hamming2(n, m);
			for(int i=0; i<m; i++) for(int j: posiciones[i][sol[i]]) hamming2[j]--;

			int cal = 0;
			for(int h: hamming2) if(h >= th_m) cal++;


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

			return pair<string, int>{sol, cal};
		}

		char baseRandom(int i, char c){
			int j = rng()%4;
			char a = bases[j];
			while(a == c) a = bases[rng()%4];
			return a;
		}
};


class Bacteria{
	public:
		Dataset *dataset;
		string solucion;
		int fitness;
		int cambio;

		Bacteria(string sol, int fit, Dataset *d){
			solucion = sol;
			fitness = fit;
			dataset = d;
			cambio = 0;
		}

		void mutar(){
			for(int i=0; i<solucion.size(); i++){
				if(rng()%1000 < pm*1000) solucion[i] = dataset->baseRandom(i, solucion[i]);
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

		void bl(){
			auto p = dataset->busquedaLocal2(solucion);
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
		Dataset *dataset;
		int antibiotico, mejor, ti;
		vector<Bacteria> bacterias;
		vector<int> donadoras, receptoras;
		queue<string> sopa;
		
		void generarPoblacion(){
			pair<string, int> p;

			for(int i=0; i<poblacion; i++){
				if(rng()%100 < pg*100) p = dataset->generarSol( nb );
				else p = dataset->generarSolRandom();
				bacterias.push_back( Bacteria(p.first, p.second, dataset) );
			}
		}

		void evaluarFitness(){
			int mejorFit = 0;
			for(Bacteria &b: bacterias) mejorFit = max(mejorFit, b.actualizarFitness());

			if(mejorFit > mejor){
				mejor = mejorFit;
				if(!tuning) cout << mejor << endl;
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
			
			//double aux = pm;
			//pm *= 2;
			//for(int i=0; i<poblacion*0.5; i++) if(rng()%100 < pm*100) bacterias[i].mutar();

			//pm = aux;
			//for(int i=poblacion*0.5; i<poblacion; i++) if(rng()%100 < pm*100) bacterias[i].mutar();
		}

		void conjugacion(){
			if(donadoras.empty()){
				mutacion();
				return;
			}

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
			for(int i=0; i<0; i++){
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

				bacterias[h1].lazaro(p1, p2);
				bacterias[h2].lazaro(p1, p2);
			}
		}

		void r(){
			for(Bacteria &b: bacterias){
				if(rng()%100 < reg*100){
					int b2 = rng()%poblacion;

					for(int j=0; j<b.solucion.size(); j++){
						if(rng()%2){
							char aux = bacterias[b2].solucion[j];
							b.solucion[j] = bacterias[b2].solucion[j];
							bacterias[b2].solucion[j] = aux;
						}
					}

					b.mutar();
					bacterias[b2].mutar();
				}
			}
		}

		void bl2(){
			for(Bacteria &b: bacterias) if(rng()%100 < bl*100) b.bl();
		}

	public:
		Sim(string instancia){
			ti = time(NULL);
			dataset = new Dataset(instancia);
			antibiotico = 0;
			mejor = 0;
		}

		int iniciar(){
			generarPoblacion();

			while(time(NULL) - ti <= tiempoMaximo){
				crearAntibiotico();

				transformacion();
				mutacion();

				clasificacion();
				conjugacion();				

				torneo();
				r();
				bl2();

				evaluarFitness();	
				administrarAntibiotico();
			}
			
			if(!tuning) cout << "fin " << mejor << endl;
			else cout << -mejor << endl;
			return mejor;
		}
};

int main(){
	Sim sim(instancia);

	sim.iniciar();
	return 0;
}