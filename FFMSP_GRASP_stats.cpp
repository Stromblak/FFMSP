#include <iostream>
#include <vector>
#include <fstream>
#include <string.h>
#include <map>
#include <chrono>
#include <iomanip>
#include <cmath>
#include <thread>
#include <atomic>
#include <map>
#include "FFMSP_GRASP.cpp"
#include <mutex>

using namespace std;
using namespace chrono;


void stats(double th);

mutex mtx;
vector<int> calidades;
vector<int> tiempos;
void funcionHilos(string instancia, double threshold, double determinismo, int tiempoMaximo){
	GRASP g(instancia, threshold, determinismo, tiempoMaximo);
	auto aux = g.iniciar();
	
	mtx.lock();
	calidades.push_back(aux.first);
	tiempos.push_back(aux.second);
	mtx.unlock();
}

int main(int argc, char *argv[]){
	string instancia = "100-300-001.txt";
	double threshold = 0.85, determinismo = 0.9;
	int tiempoMaximo = 30;
	
	for(int i=0; i<argc; i++){
		if( !strcmp(argv[i], "-i" ) ) instancia = argv[i+1];
		if( !strcmp(argv[i], "-th") ) threshold = atof(argv[i+1]);
		if( !strcmp(argv[i], "-d" ) ) determinismo = atof(argv[i+1]);
		if( !strcmp(argv[i], "-t" ) ) tiempoMaximo = atoi(argv[i+1]);
	}

	srand(time(NULL));
	stats(threshold);

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

		calidades.clear();
		tiempos.clear();

		int hilos = 10;
		vector<thread> t;
		
		for(int i=0; i<hilos; i++){
			string instancia = "instancias/" + in + genomas2[i] + ".txt";
			thread aux(funcionHilos, instancia, th, 0.9, 300);
			t.push_back( move(aux) );
		}
		for(int i=0; i<t.size(); i++) t[i].join();

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