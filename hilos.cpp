#include "FFMSP.cpp" 

#include <atomic>
#include <thread>
#include <mutex>
#include <Utilapiset.h>

atomic_int promedio;
void funcionHilos(string inst){
	Sim s(inst);
	promedio += s.iniciar().first;
}

int main(int argc, char *argv[]){
	rng.seed(time(NULL));
	update = 0;

	for(int i=0; i<argc; i++){
		// Parametros
		if( !strcmp(argv[i], "-i" ) ) instancia = argv[i+1];
		if( !strcmp(argv[i], "-th") ) threshold = atof(argv[i+1]);
		if( !strcmp(argv[i], "-t" ) ) tiempoMaximo = atof(argv[i+1]);
		if( !strcmp(argv[i], "-tuning" ) ) tuning = atof(argv[i+1]);

		// Variables
		if( !strcmp(argv[i], "-d" ) ) determinismo = atof(argv[i+1]);
		if( !strcmp(argv[i], "-p" ) ) poblacion = atoi(argv[i+1]);
		if( !strcmp(argv[i], "-nb" ) ) nb = atoi(argv[i+1]);
	}
	
	
	vector<string> genomas = {"100-300", "100-600", "100-800", "200-300", "200-600", "200-800"};
	vector<string> genomas2 = {"-001", "-002", "-003", "-004", "-005", "-006", "-007", "-008", "-009", "-010"};
	
	int hilos = 10;
	vector<thread> t;
	for(int i=0; i<hilos; i++){
		string inst = "instancias/200-300" + genomas2[i] + ".txt";
		thread aux(funcionHilos, inst);
		t.push_back( move(aux) );
	}
	for(int i=0; i<t.size(); i++) t[i].join();

	cout << "Promedio: " << (double)(promedio)/hilos << endl;
	
	Beep(523,500);


	return 0;
}
