#include "FFMSP.cpp"

#include <thread>
#include <atomic>
#include <mutex>
#include <iomanip>
#include <Utilapiset.h>

using namespace std;
using namespace chrono;


void stats();

mutex mtx;
vector<int> calidades;
vector<int> tiempos;
void funcionHilos(string instancia){
	Sim s(instancia);
	auto aux = s.iniciar();
	
	mtx.lock();
	calidades.push_back(aux.first);
	tiempos.push_back(aux.second);
	mtx.unlock();
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
		if( !strcmp(argv[i], "-d" ) ) determinismo = atof(argv[i+1]);

		// Variables
		if( !strcmp(argv[i], "-p" ) ) poblacion = atoi(argv[i+1]);
		if( !strcmp(argv[i], "-tor" ) ) torneos = atoi(argv[i+1]);

		if( !strcmp(argv[i], "-pg" ) ) pg = atof(argv[i+1]);
		if( !strcmp(argv[i], "-nb" ) ) nb = atoi(argv[i+1]);

		if( !strcmp(argv[i], "-pm" ) ) pm = atof(argv[i+1]);
		if( !strcmp(argv[i], "-bl" ) ) bl = atof(argv[i+1]);
	}

	stats();

	return 0;
}


void stats(){
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
			thread aux(funcionHilos, instancia);
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

	Beep(523,500);
}
