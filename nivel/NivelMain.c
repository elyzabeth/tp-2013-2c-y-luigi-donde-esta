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
	int sock = -1;
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
	pthread_create (&idHiloInterbloqueo, NULL, (void*) interbloqueo, NULL);


	// Conectar con proceso Plataforma
	conectar(configNivelPlataformaIp(), configNivelPlataformaPuerto(), &sock);
	if (enviarMSJNuevoNivel(sock) != EXITO) {
		log_error(LOGGER, "ERROR en conexion con Plataforma");
		finalizarNivel();
		exit(EXIT_FAILURE);
	}

	// Agrego descriptor del socket
	agregar_descriptor(sock, &master, &max_desc);

	// Agrego descriptor del inotify
	agregar_descriptor(notifyFD, &master, &max_desc);

	//simulacroJuego ();
	//ejemploGui();

	while(!fin) {
		FD_ZERO (&read_fds);
		read_fds = master;

		if((select(max_desc+1, &read_fds, NULL, NULL, NULL)) == -1)	{
			log_error(LOGGER, "NIVEL_MAIN ERROR en el select");
		} else {

			for(i = 0; i <= max_desc; i++)
			{
				if (FD_ISSET(i, &read_fds) && (i == sock))
				{
					log_debug(LOGGER, "1) recibo mensaje socket %d", i);
					recibir_header(i, &header, &master, &se_desconecto);

					if(se_desconecto)
					{
						log_info(LOGGER, "Se desconecto el socket %d", i);
						FD_CLR(i, &master);
					} else {

						log_debug(LOGGER, "Llego mensaje %d (fd:%d)", header.tipo, i);

						switch(header.tipo) {
							case NIVEL_CONECTADO:
								log_info(LOGGER, "Llego mensaje NIVEL_CONECTADO (fd:%d)", i);
								break;
							default: log_error(LOGGER, "Llego mensaje '%d' NO RECONOCIDO (fd:%d)", header.tipo, i);
								break;
						}
					}
				}

				if (FD_ISSET(i, &read_fds) && (i == notifyFD))
				{
					log_info(LOGGER, "Hubo un cambio en el archivo de configuracion (fd:%d)", i);
					read(notifyFD, buffer, BUF_LEN);
					levantarCambiosArchivoConfiguracionNivel();
					log_info(LOGGER, "Nuevos Valores: algoritmo=%s - quantum=%d - retardo=%d", configNivelAlgoritmo(), configNivelQuantum(), configNivelRetardo());
				}

				if (FD_ISSET(i, &read_fds) && (i != sock) && (i != notifyFD))
				{
					log_debug(LOGGER, "2) actividad en el socket %d", i);
					recibir_header(i, &header, &master, &se_desconecto);

					if(se_desconecto)
					{
						log_info(LOGGER, "Se desconecto el socket %d", i);
						FD_CLR(i, &master);
					} else {
						log_debug(LOGGER, "2) Llego mensaje del socket %d: %d NO RECONOCIDO", i, header.tipo);
					}

				}

			}

		}
	}

	pthread_join (idHiloInterbloqueo, NULL); //espera que finalice el hilo de interbloqueo para continuar

	close (sock);

	return;
}


