/*
 * NivelMain.c
 *
 *  Created on: 22/09/2013
 *      Author: elyzabeth
 */

#include "NivelMain.h"


int main (int argc, char**argv) {

	// Correr tests
	if (argc > 1 && strcmp(argv[1], "-test")==0)
		return correrTest();

	// Registro signal and signal handler
	signal(SIGINT, signal_callback_handler);
	signal(SIGQUIT, signal_callback_handler);
	signal(SIGUSR1, signal_callback_handler);
	signal(SIGTERM, signal_callback_handler);

	inicializarNivel();

	principal ();

	finalizarNivel();

	return EXIT_SUCCESS;
}

void principal () {
	int id_proceso, i, se_desconecto;
	int fin = false;
	int sockPlataforma = -1;
	header_t header;
	fd_set master;
	fd_set read_fds;
	FD_ZERO(&master);
	FD_ZERO(&read_fds);
	int max_desc = 0;
	char buffer[BUF_LEN];

	id_proceso = getpid();
	log_info(LOGGER,"************** Iniciando Nivel '%s' (PID: %d) ***************\n", NOMBRENIVEL, id_proceso);

	// Lanzo Hilo de Interbloqueo
	pthread_create (&hiloInterbloqueo.tid, NULL, (void*) interbloqueo, NULL);


	// Conectar con proceso Plataforma
	conectar(configNivelPlataformaIp(), configNivelPlataformaPuerto(), &sockPlataforma);

	if (enviarMSJNuevoNivel(sockPlataforma) != EXITO) {
		log_error(LOGGER, "ERROR en conexion con Plataforma");
		finalizarNivel();
		exit(EXIT_FAILURE);
	}

	// Agrego descriptor del socket de conexion con plataforma
	agregar_descriptor(sockPlataforma, &master, &max_desc);

	// Agrego descriptor del inotify
	agregar_descriptor(notifyFD, &master, &max_desc);

	// Agrego descriptor de comunicacion con hilo de interbloqueo por pipe
	agregar_descriptor(hiloInterbloqueo.fdPipeI2N[0], &master, &max_desc);

	// Agrego fd del pipe con hilos enemigos
	agregarFDPipeEscuchaEnemigo(&master, &max_desc);

	while(!fin) {
		FD_ZERO (&read_fds);
		read_fds = master;

		if((select(max_desc+1, &read_fds, NULL, NULL, NULL)) == -1)	{
			log_error(LOGGER, "NIVEL_MAIN ERROR en el select");
		} else {

			for(i = 0; i <= max_desc; i++)
			{
				if (FD_ISSET(i, &read_fds))
				{
					if (i == sockPlataforma)
					{
						log_debug(LOGGER, "1) recibo mensaje socket %d", i);
						initHeader(&header);
						recibir_header(i, &header, &master, &se_desconecto);

						if(se_desconecto)
						{
							log_info(LOGGER, "Se desconecto el socket %d ", i);
							quitar_descriptor(i, &master, &max_desc);

						} else {

							log_debug(LOGGER, "Llego mensaje %d (fd:%d)", header.tipo, i);

							switch(header.tipo) {

								case NIVEL_CONECTADO:
									log_info(LOGGER, "Llego mensaje '%d' NIVEL_CONECTADO (fd:%d)", header.tipo, i);
									break;

								case NUEVO_PERSONAJE:
									log_info(LOGGER, "Llego mensaje '%d' NUEVO_PERSONAJE (fd:%d)", header.tipo, i);
									tratarNuevoPersonaje(i, header, &master);
									break;

								case SOLICITUD_UBICACION:
									log_info(LOGGER, "Llego mensaje '%d' SOLICITUD_UBICACION (fd:%d)", header.tipo, i);
									tratarSolicitudUbicacion(i, header, &master);
									break;

								case SOLICITUD_RECURSO:
									log_info(LOGGER, "Llego mensaje '%d' SOLICITUD_RECURSO (fd:%d)", header.tipo, i);
									tratarSolicitudRecurso(i, header, &master);
									break;

								case MOVIMIENTO_REALIZADO:
									log_info(LOGGER, "Llego mensaje '%d' MOVIMIENTO_REALIZADO (fd:%d)", header.tipo, i);
									tratarMovimientoRealizado(i, header, &master);
									break;

								case PLAN_NIVEL_FINALIZADO:
									log_info(LOGGER, "Llego mensaje '%d' PLAN_NIVEL_FINALIZADO (fd:%d)", header.tipo, i);
									tratarPlanNivelFinalizado (i, header, &master);
									break;

								case MUERTE_PERSONAJE:
									log_info(LOGGER, "Llego mensaje '%d' MUERTE_PERSONAJE (fd:%d)", header.tipo, i);
									tratarMuertePersonaje (i, header, &master);
									break;

								case PERSONAJE_DESBLOQUEADO:
									log_info(LOGGER, "Llego mensaje '%d' PERSONAJE_DESBLOQUEADO (fd:%d)", header.tipo, i);
									// TODO HACER ALGO CON ESTE MENSAJE??
									desbloquearPersonaje(i, header, &master);
									break;

								default: log_error(LOGGER, "Llego mensaje '%d' NO RECONOCIDO (fd:%d)", header.tipo, i);
									break;
							}
						}

					} else if (i == notifyFD) {

						log_info(LOGGER, "Hubo un cambio en el archivo de configuracion (fd:%d)", i);
						read(notifyFD, buffer, BUF_LEN);
						levantarCambiosArchivoConfiguracionNivel();
						log_info(LOGGER, "Nuevos Valores: algoritmo=%s - quantum=%d - retardo=%d", configNivelAlgoritmo(), configNivelQuantum(), configNivelRetardo());

						enviarMsjCambiosConfiguracion(sockPlataforma);

					} else if ( i == hiloInterbloqueo.fdPipeI2N[0]) {

						// Llega mensaje desde hilo interbloqueo por Pipe
						log_info(LOGGER, "NivelMain: Recibo mensaje desde Interbloqueo por Pipe: %d", i);

						initHeader(&header);
						read (i, &header, sizeof(header_t));

						log_debug(LOGGER, "NivelMain: mensaje recibido '%d'", header.tipo);
						switch (header.tipo)
						{
							case MUERTE_PERSONAJE_XRECOVERY: log_debug(LOGGER, "'%d' ES MUERTE_PERSONAJE_XRECOVERY", header.tipo);
								// TODO Mover personaje a una lista de fiambres??

								// informar al planificador
								enviarMsjMuertexRecovery(sockPlataforma);

								break;
						}

					} else {

						// NO ES NI notifyFD NI mi Planificador Ni el hilo de Interbloqueo
						// Entonces debe ser un hilo Enemigo
						log_debug(LOGGER, "NivelMain: actividad en el socket %d", i);

						log_info(LOGGER, "NivelMain: Recibo mensaje desde Enemigo por Pipe: %d", i);
						initHeader(&header);
						read (i, &header, sizeof(header_t));

						log_debug(LOGGER, "NivelMain: mensaje recibido '%d'", header.tipo);
						switch (header.tipo)
						{
							case MUERTE_PERSONAJE_XENEMIGO: log_debug(LOGGER, "'%d' ES MUERTE_PERSONAJE_XENEMIGO", header.tipo);
								// TODO mover personaje a lista de fiambres ??
								// informar al planificador
								enviarMsjMuertexEnemigo(sockPlataforma);
								break;
						}

//						if(se_desconecto)
//						{
//							log_info(LOGGER, "Se desconecto el socket %d", i);
//							quitar_descriptor(i, &master, &max_desc);
//
//						} else {
//							log_debug(LOGGER, "2) Llego mensaje del socket %d: %d NO RECONOCIDO", i, header.tipo);
//						}

					}
				}
			}
		}
	} // Cierro while

	pthread_join (hiloInterbloqueo.tid, NULL); //espera que finalice el hilo de interbloqueo para continuar

	close (sockPlataforma);

	return;
}


