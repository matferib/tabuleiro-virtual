/** @file main.cpp o inicio de tudo. Responsavel por instanciar a interface grafica principal. */

#include <stdexcept>
#include <iostream>
#include "ifg/qt/principal.h"

using namespace std;

int main(int argc, char** argv){
	ifg::qt::Principal* p = ifg::qt::Principal::CriaInstancia(argc, argv);
	try {
		p->Executa();
		ifg::qt::Principal::DestroiInstancia();
	}
	catch (exception& e) {
		cerr << e.what() << endl;
		return 1;
	}
	return 0;
}
