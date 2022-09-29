#include <iostream>
#include <vector>
#include <fstream>
#include <string.h>
#include <map>
#include <chrono>
#include <iomanip>
#include <cmath>
#include "FFMSP_heuristica.cpp"

using namespace std;
using namespace chrono;

void stats(double th);

int main(int argc, char *argv[]){
	string instancia;
	double threshold, determinismo;
	
	srand(time(NULL));

	for(int i=0; i<argc; i++){
		if( !strcmp(argv[i], "-i" ) ) instancia = argv[i+1];
		if( !strcmp(argv[i], "-th") ) threshold = atof(argv[i+1]);
		if( !strcmp(argv[i], "-d" ) ) determinismo = atof(argv[i+1]);
	}

	stats(0.85);
	return 0;
}

void stats(double th){
	vector<string> genomas = {"100-300", "100-600", "100-800", "200-300", "200-600", "200-800"};
	vector<string> genomas2 = {"-001", "-002", "-003", "-004", "-005", "-006", "-007", "-008", "-009", "-010"};

	vector<double> CalidadMedia, CalidadDesviacion;
	vector<double> TiempoMedio, TiempoDesviacion;

	int repeticiones = 5;
	for(auto in: genomas){
		cout << in << endl;

		vector<int> calidades;
		vector<int> tiempos;
	
		for(auto mid: genomas2){
			ifstream archivo("instancias/" + in + mid + ".txt");
			vector<string> set;
			string gen;

			while(archivo >> gen) set.push_back(gen);
			archivo.close();

			int n = set.size();
			int m = set[0].size();
			
			auto start = high_resolution_clock::now();
			for(int i=0; i<repeticiones; i++) calidades.push_back( greedy(set, th, n, m) );
			auto finish = high_resolution_clock::now();
			tiempos.push_back( duration_cast<milliseconds>(finish - start).count()/repeticiones );
		}

		double media = 0;
		for(auto c: calidades) media += c;
		media /= calidades.size();
		CalidadMedia.push_back(media);

		double desviacion = 0;
		for(auto c: calidades) desviacion += (c-media)*(c-media);
		desviacion = sqrt(desviacion/calidades.size());
		CalidadDesviacion.push_back(desviacion);

		double tmedia = 0;
		for(auto c: tiempos) tmedia += c;
		tmedia /= tiempos.size();
		TiempoMedio.push_back(tmedia);

		double tdesviacion = 0;
		for(auto c: tiempos) tdesviacion += (c-tmedia)*(c-tmedia);
		tdesviacion = sqrt(tdesviacion/tiempos.size());
		TiempoDesviacion.push_back(tdesviacion);
	}

	cout << fixed << setprecision(2) << endl;

	cout << "Calidad Media" << endl;
	for(auto a: CalidadMedia) cout << a << endl;
	cout << endl;

	cout << "Calidad desviacion" << endl;
	for(auto a: CalidadDesviacion) cout << a << endl;
	cout << endl;

	cout << "Tiempo Media" << endl;
	for(auto a: TiempoMedio) cout << a << endl;
	cout << endl;

	cout << "Tiempo desviacion" << endl;
	for(auto a: TiempoDesviacion) cout << a << endl;
}