/*
 * planificador.c
 *
 *  Created on: Oct 8, 2013
 *      Author: elizabeth
 */

#include "plataforma.h"


void* planificador(void *nivel) {
	log_info(LOGGER, "PLANIFICADOR %s: Iniciado.", nivel);

	while(1) {

	}


	pthread_exit(NULL);
}
