/*
 * interbloqueo.c
 *
 *  Created on: Oct 11, 2013
 *      Author: elizabeth
 */

#include "funcionesNivel.h"

int32_t detectarDeadlock();


void* interbloqueo(void *parametro) {
	int32_t TiempoChequeoDeadlock;

	log_info(LOGGER, "DETECCION DE INTERBLOQUEO: Iniciado.");

	TiempoChequeoDeadlock = configNivelTiempoChequeoDeadlock();
	int32_t RecoveryOn = configNivelRecovery();
	int32_t hayDeadLock = 0;


	while(1) {
		log_info(LOGGER, "Incio deteccion de interbloqueo...");


		hayDeadLock = detectarDeadlock();
		if (hayDeadLock && RecoveryOn) {
			// TODO agregar logica de recovery
		}

		// TODO preguntar si es correcto usar sleep aca
		sleep(TiempoChequeoDeadlock);

	}

	pthread_exit(NULL);

}

int32_t detectarDeadlock() {
	int32_t hayDeadLock = 0;

	// TODO agregar algoritmo de deteccion de interbloqueo

	return hayDeadLock;

}

