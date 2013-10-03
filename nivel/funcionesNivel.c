/*
 * funcionesNivel.c
 *
 *  Created on: Oct 3, 2013
 *      Author: elyzabeth
 */

#include "funcionesNivel.h"


int correrTest() {
	// TODO llamar a funciones de cunit.
	return EXIT_SUCCESS;
}

void inicializar () {
	levantarArchivoConfiguracionNivel();
	LOGGER = log_create(configNivelLogPath(), "NIVEL", configNivelLogConsola(), configNivelLogNivel() );
	strncpy(NOMBRENIVEL, configNivelNombre(), 20);
	log_info(LOGGER, "INFO: INICIALIZANDO NIVEL '%s'\n", NOMBRENIVEL);

	log_debug(LOGGER, "DEBUG: INICIALIZANDO NIVEL '%s' ", NOMBRENIVEL);
}

void finalizar () {
	log_info(LOGGER, "FINALIZANDO NIVEL '%s'", NOMBRENIVEL);
	log_info(LOGGER, "LIBERANDO ESTRUCTURAS DE CONFIG-NIVEL '%s'", NOMBRENIVEL);
	destruirConfigNivel();
	log_info(LOGGER, "LIBERANDO ESTRUCTURAS DE LOGGER '%s' \n\n (Adios Mundo CRUEL!) piiiiiiiiiiiiiii.....\n\n", NOMBRENIVEL);
	log_destroy(LOGGER);
}


/*
 * @NAME: signal_callback_handler
 * @DESC: Define la funcion a llamar cuando una señal es enviada al proceso
 * ctrl-c (SIGINT)
 */
void signal_callback_handler(int signum)
{
	//printf("Llego la señal %d\n",signum);
	log_info(LOGGER, "INTERRUPCION POR SEÑAL: %d\n", signum);

	switch(signum) {
		case SIGINT: // SIGINT=2
			log_info(LOGGER, " - LLEGO SEÑAL SIGINT\n");
			finalizar();
		break;
	}

	// Termino el programa
	exit(signum);
}


/*
 * @NAME: rnd
 * @DESC: Modifica el numero en +1,0,-1, sin pasarse del maximo dado
 */
void rnd(int *x, int max) {
	*x += (rand() % 3) - 1;
	*x = (*x<max) ? *x : max-1;
	*x = (*x>0) ? *x : 1;
}
