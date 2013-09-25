/*
 * personaje.c
 *
 *  Created on: Sep 24, 2013
 *      Author: elizabeth
 */

#include "personaje.h"

t_log* LOGGER;

int inicializar();


int main () {

	inicializar();

	// TODO agregar logica del personaje

	return EXIT_SUCCESS;
}

/**
 * @NAME: inicializar
 * @DESC: Inicializa todas las variables y estructuras necesarias para el personaje
 */
int inicializar() {
	levantarArchivoConfiguracionPersonaje();
	// TODO agregar inicializaciones necesarias
	LOGGER = log_create(configPersonajeLogPath(), "PERSONAJE", configPersonajeLogConsola(), configPersonajeLogNivel() );
	log_info(LOGGER, "INICIALIZANDO PERSONAJE '%s' ", configPersonajeNombre());


	return EXIT_SUCCESS;
}
