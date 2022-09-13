#include <iostream>
#include <vector>
#include <fstream>
#include <string.h>
#include <map>
#include <chrono>
#include <iomanip>
#include "FFMSP_heuristica.cpp"

using namespace std;
using namespace chrono;

void stats(double th);

int main(int argc, char *argv[]){
	string instancia;
	double threshold, alpha;
	
	srand(time(NULL));

	for(int i=0; i<argc; i++){
		if( !strcmp(argv[i], "-i" ) ) instancia = argv[i+1];
		if( !strcmp(argv[i], "-th") ) threshold = atof(argv[i+1]);
		if( !strcmp(argv[i], "-a" ) ) alpha = atof(argv[i+1]);
	}

	ifstream archivo(instancia);
	vector<string> set;
	string gen;

	while(archivo >> gen) set.push_back(gen);
	archivo.close();

	int n = set.size();
	int m = set[0].size();

	stats(threshold);
	return 0;
}

void stats(double th){
	vector<string> genomas = {"100-300-", "100-600-", "100-800-", "200-300-", "200-600-", "200-800-"};
	vector<string> genomas2 = {"001", "002", "003", "004", "005", "006", "007", "008", "009", "010"};
	string fin = ".txt";

	for(auto in: genomas){
		cout << in << endl;

		vector<int> calidades;
		vector<int> tiempos;

		for(auto mid: genomas2){
			ifstream archivo(in + mid + fin);
			vector<string> set;
			string gen;

			while(archivo >> gen) set.push_back(gen);
			archivo.close();

			int n = set.size();
			int m = set[0].size();
			
			auto start = high_resolution_clock::now();
			for(int i=0; i<10; i++) calidades.push_back( greedy(set, th, n, m) );
			auto finish = high_resolution_clock::now();
			tiempos.push_back(duration_cast<milliseconds>(finish - start).count()/10);
		}

		double media = 0;
		for(auto c: calidades) media += c;
		media /= calidades.size();

		double desviacion = 0;
		for(auto c: calidades) desviacion += (c-media)*(c-media);
		desviacion = sqrt(desviacion/calidades.size());


		double tmedia = 0;
		for(auto c: tiempos) tmedia += c;
		tmedia /= tiempos.size();

		double tdesviacion = 0;
		for(auto c: tiempos) tdesviacion += (c-tmedia)*(c-tmedia);
		tdesviacion = sqrt(tdesviacion/tiempos.size());

		cout << fixed;
		cout << setprecision(2);

		cout << "Calidad Media: " << media << endl;
		cout << "Calidad desviacion: " << desviacion << endl;
		cout << "Tiempo Media: " << tmedia << endl;
		cout << "Tiempo desviacion: " << tdesviacion << endl;
		cout << endl;
	}
}