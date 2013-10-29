/*
 * orquestador.c
 *
 *  Created on: 06/10/2013
 *      Author: elyzabeth
 */

#include "plataforma.h"


void* orquestador(t_hiloOrquestador *hiloOrquestador) {

	header_t header;
	fd_set master;
	fd_set read_fds;

	int max_desc = 0;
	int nuevo_sock;
	int listener;
	int i, se_desconecto;
	int fin = false;

	log_info(LOGGER, "ORQUESTADOR: Iniciado.");

	FD_ZERO(&master);
	FD_ZERO(&read_fds);

	// Agrego descriptor de comunicacion con plataforma por pipe
	agregar_descriptor(hiloOrquestador->fdPipe[0], &master, &max_desc);

	/****************************** Creacion Listener ****************************************/
	log_info(LOGGER, "****************** CREACION DEL LISTENER *****************\n");
	// Uso puerto seteado en el archivo de configuracion
	crear_listener(PUERTO, &listener);

	agregar_descriptor(listener, &master, &max_desc);

	log_info(LOGGER, "ORQUESTADOR: Esperando conexiones...");
	/***************************** LOGICA PRINCIPAL ********************************/
	while(!fin)
	{

		FD_ZERO (&read_fds);
		read_fds = master;

		if((select(max_desc+1, &read_fds, NULL, NULL, NULL/*&tvDemora*/)) == -1)
		{
			log_error(LOGGER, "ORQUESTADOR: error en el select()");
		}

		for(i = 0; i <= max_desc; i++)
		{
			//otrosDescriptor = 1;
			if (FD_ISSET(i, &read_fds) )
			{
				if (i == listener)
				{
					/* nueva conexion */
					log_info(LOGGER, "ORQUESTADOR: NUEVA CONEXION");

					aceptar_conexion(&listener, &nuevo_sock);
					//agregar_descriptor(nuevo_sock, &master, &max_desc);

					recibir_header(nuevo_sock, &header, &master, &se_desconecto);

					switch (header.tipo)
					{
						case NUEVO_PERSONAJE:
							log_info(LOGGER, "ORQUESTADOR: NUEVO PERSONAJE");
							nuevoPersonaje(nuevo_sock, &master, &max_desc);
							break;

						case NUEVO_NIVEL:
							log_info(LOGGER, "ORQUESTADOR: NUEVO NIVEL");
							nuevoNivel(nuevo_sock, header);
							//agregar_descriptor(nuevo_sock, &master, &max_desc);
							break;

						case OTRO:
							break;
					}
				} else if (i == hiloOrquestador->fdPipe[0]) {

					log_info(LOGGER, "ORQUESTADOR: Recibo mensaje desde Plataforma por Pipe");
					initHeader(&header);
					read (hiloOrquestador->fdPipe[0], &header, sizeof(header_t));

					log_debug(LOGGER, "ORQUESTADOR: mensaje recibido '%d'", header.tipo);

					if (header.tipo == FINALIZAR) {
						log_debug(LOGGER, "ORQUESTADOR: '%d' ES FINALIZAR", header.tipo);
						fin = true;
						//FD_CLR(hiloOrquestador->fdPipe[0], &master);
						quitar_descriptor(i, &master, &max_desc);
						break;
					}

				} else if (i != listener && i != hiloOrquestador->fdPipe[0]) {

					log_debug(LOGGER, "ORQUESTADOR: recibo mensaje socket %d", i);
					recibir_header(i, &header, &master, &se_desconecto);
					log_debug(LOGGER, "ORQUESTADOR: el tipo de mensaje es: %d\n", header.tipo);

					if(se_desconecto)
					{
						log_info(LOGGER, "ORQUESTADOR: Se desconecto el socket %d", i);
						// TODO chequear si se desconecto personaje o nivel y borrarlo de las estructuras
						// si es personaje informar al nivel para que lo borre?
						// plataforma.personajes_en_juego--;

						// Quito el descriptor del set
						//FD_CLR(i, &master);
						quitar_descriptor(i, &master, &max_desc);
					}

					if ((header.tipo == CONECTAR_NIVEL) && (se_desconecto != 1))
					{
						puts("ORQUESTADOR: CONECTAR NIVEL"); //Nunca voy a necesitar este mensaje.

					}

				}
			}
		}
	}

	log_info(LOGGER, "ORQUESTADOR: FINALIZANDO HILO.");

	pthread_exit(NULL);
}
