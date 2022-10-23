#include <iostream>
#include <vector>
#include <fstream>
#include <string.h>
#include <ctime>
#include <unordered_map>
#include <algorithm>
#include <random>
using namespace std;


class GRASP{
	private:
		vector<char> bases = {'A', 'C', 'G', 'T'};
		vector<string> dataset;
		vector<int> hamming, indices;
		vector<unordered_map<char, int>> contador;
		unordered_map<char, int> calidadBase;
		double th, det;
		int n, m, ti, tMAx, intentosExtra;
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

		pair<string, int> busquedaLocal(pair<string, int> solucion){
			string mejorSol = solucion.first, sol = mejorSol;
			int mejorCal = solucion.second;
			int intentos = 5;

			while(intentos && time(NULL) - ti <= tMAx && mejorCal != n){

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
					intentos += intentosExtra;
				}else intentos -= 1;
			}

			return pair<string, int>{mejorSol, mejorCal};
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
			rng.seed(time(NULL));

			procesarIndices();
		}

		pair<int, int> iniciar(){
			ti = time(NULL);
			pair<string, int> solActual = {" ", -1};
			int tf;

			while(time(NULL) - ti <= tMAx && solActual.second != n){
				pair<string, int> solNueva = busquedaLocal( greedyRandomizado() );

				if(solNueva.second > solActual.second){
					solActual = solNueva;
					tf = time(NULL) - ti;
					cout << "Nueva solucion: " << solNueva.second << "  Tiempo: " << tf << endl;
				}
			}

			cout << "Mejor solucion: " << solActual.second << "  Tiempo usado: " << tf << endl;

			return pair<int, int>{solActual.second, tf};
		}

		pair<string, int> generarSol(){
			return greedyRandomizado();
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
	public:
		string solucion;
		int fitness;

		Bacteria(string sol, int fit){
			solucion = sol;
			fitness = fit;
		}
};



class Sim{
	private:
		GRASP *grasp;
		vector<Bacteria> bacterias, donadoras, receptoras;
		int poblacion, antibiotico;
		minstd_rand rng;

		void generarPoblacion(){
			for(int i=0; i<poblacion; i++){
				auto p = grasp->generarSol();
				bacterias.push_back( Bacteria(p.first, p.second) );
			}
		}

		void evaluarFitness(){
			for(auto b: bacterias){
				int nuevoFitness = grasp->calcularCalidad( b.solucion );
				b.fitness = nuevoFitness;
			}			
		}

		void crearAntibiotico(){
			int antiBio = 10000;
			int torneos = 10;
			for(int i=0; i<torneos; i++){
				int b1 = bacterias[rng()%poblacion].fitness;
				int b2 = bacterias[rng()%poblacion].fitness;
				antiBio = min(antiBio, max(b1, b2));
			}

			antibiotico = antiBio;
		}

		void clasificacion(){
			vector<Bacteria> donadoras, receptoras;

			for(auto b: bacterias){
				if(b.fitness > antibiotico) donadoras.push_back(b);
				else receptoras.push_back(b);
			}
		}

		void variacionGenetica(){
			if(rng()%100 < pm*100){

			}


		}

		void conjugacion(){

		}

		void transformacion(){

		}

		void mutacion(Bacteria b){

		}

		void administrarAntibiotico(){

		}

		void regeneracion(){

		}


	public:
		Sim(string instancia, double threshold, double determinismo, int intentos, int pob){
			grasp = new GRASP(instancia, threshold, determinismo, 90, intentos);
			poblacion = pob;
			rng.seed(time(NULL));
		}

		void iniciar(){
			int iter = 10;

			generarPoblacion();
			while(iter--){
				evaluarFitness();
				crearAntibiotico();
				variacionGenetica();
				administrarAntibiotico();
				regeneracion();
			}
		}
};




int main(int argc, char *argv[]){
	string instancia = "100-300-001.txt";
	double threshold = 0.80, determinismo = 0.9;
	int tiempoMaximo = 90, intentosExtra = 20, poblacion = 100;
	
	for(int i=0; i<argc; i++){
		if( !strcmp(argv[i], "-i" ) ) instancia = argv[i+1];
		if( !strcmp(argv[i], "-th") ) threshold = atof(argv[i+1]);
		if( !strcmp(argv[i], "-d" ) ) determinismo = atof(argv[i+1]);
		if( !strcmp(argv[i], "-t" ) ) tiempoMaximo = atoi(argv[i+1]);
		if( !strcmp(argv[i], "-ie" ) ) intentosExtra = atoi(argv[i+1]);
		if( !strcmp(argv[i], "-p" ) ) poblacion = atoi(argv[i+1]);

	}

	Sim s(instancia, threshold, determinismo, intentosExtra, poblacion);
	s.iniciar();


	return 0;
}