/*
 * personaje.c
 *
 *  Created on: Sep 24, 2013
 *      Author: elizabeth
 */

#include "personaje.h"


int main () {

	inicializarPersonaje();

	// TODO agregar logica del personaje

	return EXIT_SUCCESS;
}

/**
 * @NAME: inicializarPersonaje
 * @DESC: Inicializa todas las variables y estructuras necesarias para el proceso personaje
 */
void inicializarPersonaje() {
	levantarArchivoConfiguracionPersonaje();
	// TODO agregar inicializaciones necesarias
	LOGGER = log_create(configPersonajeLogPath(), "PERSONAJE", configPersonajeLogConsola(), configPersonajeLogNivel() );
	log_info(LOGGER, "INICIALIZANDO PERSONAJE '%s' ", configPersonajeNombre());

}

/**
 * @NAME: finalizarPersonaje
 * @DESC: Finaliza todas las variables y estructuras que fueron creadas para el proceso personaje
 */
void finalizarPersonaje() {
	destruirConfigPersonaje();
}
