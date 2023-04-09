#include "FFMSP.cpp" 

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
		if( !strcmp(argv[i], "-nb" ) ) nb = atoi(argv[i+1]);
	}

	Hormigero h(instancia);
	h.iniciar();

	return 0;
}