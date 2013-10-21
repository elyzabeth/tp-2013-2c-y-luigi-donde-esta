/*
 * tad_planificador.h
 *
 *  Created on: Oct 16, 2013
 *      Author: elizabeth
 */

#ifndef TAD_PLANIFICADOR_H_
#define TAD_PLANIFICADOR_H_

#include <unistd.h>
#include "tad_nivel.h"

typedef enum {
	NUEVO,
	CORRIENDO,
	FINALIZADO
}t_estado;

typedef struct planner {
	t_nivel nivel;
	int32_t fdPipe[2]; // fdPipe[0] de lectura/ fdPipe[1] de escritura
	pthread_t tid;		// Identidicador del hilo
	t_estado estado;
} t_planificador;

t_planificador* crearPlanificador (t_nivel nivel);
void destruirPlanificador (t_planificador* planificador);

#endif /* TAD_PLANIFICADOR_H_ */
