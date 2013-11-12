/*
 * interbloqueo.c
 *
 *  Created on: Oct 11, 2013
 *      Author: elizabeth
 */

#include "funcionesNivel.h"

int32_t matAsignacion[20][20];
int32_t matSolicitud[20][20];
int32_t vecDisponibles[20];
int32_t totalRecursos;
int32_t totalPersonajes;

int32_t detectarDeadlock();

void* interbloqueo(t_hiloInterbloqueo *hiloInterbloqueoo) {
	header_t header;
	fd_set master;
	fd_set read_fds;

	int max_desc = 0;
	int i, ret;
	struct timeval timeout;
	int fin = false;

	int32_t TiempoChequeoDeadlock;
	int32_t RecoveryOn;
	int32_t hayDeadLock;

	log_info(LOGGER, "HILO DE DETECCION DE INTERBLOQUEO: Iniciado.");

	FD_ZERO(&master);
	FD_ZERO(&read_fds);

	// Agrego descriptor de comunicacion con plataforma por pipe
	agregar_descriptor(hiloInterbloqueo.fdPipe[0], &master, &max_desc);

	TiempoChequeoDeadlock = configNivelTiempoChequeoDeadlock();
	RecoveryOn = configNivelRecovery();
	hayDeadLock = 0;


	while(!fin) {

		FD_ZERO (&read_fds);
		read_fds = master;
		timeout.tv_sec = TiempoChequeoDeadlock; /// retardo en segundos timeout
		timeout.tv_usec = 0;

		if((ret = select(max_desc+1, &read_fds, NULL, NULL, &timeout )) == -1)
		{
			puts("INTERBLOQUEO: ERROR en select");

		} else if (ret == 0) {

			log_info(LOGGER, "Incio deteccion de interbloqueo...");

			// Aca va la logica de interbloqueo
			hayDeadLock = detectarDeadlock();
			if (hayDeadLock && RecoveryOn) {
				// TODO agregar logica de recovery
			}

		} else {


			for(i = 0; i <= max_desc; i++)
			{
				if (FD_ISSET(i, &read_fds) && (i == hiloInterbloqueo.fdPipe[0]) ) {
					log_info(LOGGER, "INTERBLOQUEO: Recibo mensaje desde Nivel por Pipe");
					read (hiloInterbloqueo.fdPipe[0], &header, sizeof(header_t));

					log_debug(LOGGER, "INTERBLOQUEO: mensaje recibido '%d'", header.tipo);

					if (header.tipo == FINALIZAR) {
						log_debug(LOGGER, "INTERBLOQUEO: '%d' ES FINALIZAR", header.tipo);
						fin = true;
						FD_CLR(hiloInterbloqueo.fdPipe[0], &master);
						break;
					}
				}
			}
		}


	}

	log_info(LOGGER, "FINALIZANDO HILO INTERBLOQUEO...\n");

	pthread_exit(NULL);

}


int32_t detectarDeadlock() {

	int32_t hayDeadLock = 0;
//	int32_t T[20];
//
//	t_list *listaPJBloqueados = clonarListaPersonajesBloqueados();
//	t_dictionary *recursosxPJ = clonarListaRecursosxPersonaje();
//	t_dictionary *dicRecursos;

	// TODO agregar algoritmo de deteccion de interbloqueo
	// Lleno las matrices y los vectores necesarios.

	// 1) Se marca cada proceso que tenga una fila de la matriz de Asignacion completamente a cero
	// 2) Se inicia un vector temporal T asignandole el vector de disponibles
	// 3) Se busca un indice i tal que el proceso i no este marcado actualmente y la fila i-esima de S(olicitud) sea menor o igual a T (disponibles).
	//    Es decir, Se ejecuta Tk = Tk + Aik, para 1 <= k <= m. A continuacion se vuelve al 3 paso.

	enviarMsjPorPipe(hiloInterbloqueo.fdPipeI2N[1], MUERTE_PERSONAJE_XRECOVERY);

	return hayDeadLock;

}

