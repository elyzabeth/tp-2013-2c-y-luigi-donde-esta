/*
 * personaje.c
 *
 *  Created on: Sep 24, 2013
 *      Author: elizabeth
 */

#include "personaje.h"

int test ();

int main (int argc, char *argv[]) {

	signal(SIGINT, per_signal_callback_handler);
	signal(SIGQUIT, per_signal_callback_handler);
	signal(SIGUSR1, per_signal_callback_handler);
	signal(SIGTERM, per_signal_callback_handler);

	inicializarPersonaje();

	// TODO agregar logica del personaje
	//principal(argc, argv);
	test ();

	finalizarPersonaje();

	return EXIT_SUCCESS;
}


int principal(int argc, char *argv[]) {

	int id_proceso;

	id_proceso = getpid();
	system("clear");

	log_info(LOGGER,"************** Iniciando Personaje '%s' (PID: %d) ***************\n", personaje.nombre, id_proceso);

	levantarHilosxNivel();

	esperarHilosxNivel();

	// TODO	Al concluir todos los niveles, se conectará al Orquestador y notificará que concluyó su
//	plan de niveles. Este lo moverá a una cola de finalizados y lo dejará a la espera de que
//	los demás Personajes terminen.


	return 0;
}

void* personajexNivel (t_hilo_personaje *hiloPxN) {
	int sock = -1;
	header_t header;
	fd_set master;
	fd_set read_fds;
	int max_desc = 0;
	int i, ret;
	int fin = false;

	log_info(LOGGER, "Hilo Personaje para objetivos del nivel %s Iniciado", hiloPxN->personaje.nivel);
sleep(1);

	/***************** ME CONECTO Y ARMO MENSAJE DE PRESENTACION *******/
	log_info(LOGGER,"************** CONECTANDOSE  ***************\n");
	conectar(personaje.ip_orquestador, personaje.puerto_orquestador, &sock);

	FD_ZERO(&master);
	// Agrego descriptor del Pipe con Nivel.
	agregar_descriptor(hiloPxN->fdPipe[0], &master, &max_desc);

	agregar_descriptor(sock, &master, &max_desc);

	if (enviarMsjNuevoPersonaje(sock) != EXITO)
	{
		log_error(LOGGER,"Error al enviar header NUEVO_PERSONAJE\n\n");
		fin = true;
		//return WARNING;
	}


	while(!fin)
	{
		FD_ZERO (&read_fds);
		read_fds = master;

		ret = select(max_desc+1, &read_fds, NULL, NULL, NULL);

		if(ret == -1) {
			printf("Personaje %s '%c': ERROR en select.", hiloPxN->personaje.nombre, hiloPxN->personaje.id);
			sleep(1);
		}

		if (ret > 0) {
			for(i = 0; i <= max_desc; i++)
			{

				if (FD_ISSET(i, &read_fds))
				{
					// Pregunto si el socket con actividad es el del Pipe
					if( i == hiloPxN->fdPipe[0])
					{
						header_t header;
						log_info(LOGGER, "Personaje '%c': Recibo mensaje desde Main por Pipe", hiloPxN->personaje.id);
						read (hiloPxN->fdPipe[0], &header, sizeof(header_t));

						log_debug(LOGGER, "Personaje '%c': mensaje recibido '%d'", hiloPxN->personaje.id, header.tipo);
						if (header.tipo == FINALIZAR) {
							log_debug(LOGGER, "Personaje '%c': '%d' ES FINALIZAR", hiloPxN->personaje.id, header.tipo);
							fin = true;
							break;
						}

					} else {
						// Si NO es un mensaje del hilo principal por Pipe es un mensaje del proceso Plataforma.

						initHeader(&header);
						recibirHeaderNuevoMsj(sock, &header, &master);

						// Recibo el header con el tipo de mensaje
						switch (header.tipo)
						{
							case PERSONAJE_CONECTADO: log_info(LOGGER,"PERSONAJE_CONECTADO");
								enviarInfoPersonaje2(sock);
								break;

							case TURNO_CONCEDIDO: log_info(LOGGER,"TURNO_CONCEDIDO");
								//enviarSolicitudUbicacion(sock);
								break;

							case RECURSO_CONCEDIDO: log_info(LOGGER,"RECURSO_CONCEDIDO");
								//enviarSolicitudUbicacion(sock);
								break;

							case OTRO: log_info(LOGGER, "otro");
								break;

						}


					}
				}
			}

		}
	}

	log_info(LOGGER, "FINALIZANDO Hilo Personaje '%c' Nivel %s\n", hiloPxN->personaje.id, hiloPxN->personaje.nivel);

	pthread_exit(NULL);
}



int test () {

	int id_proceso;
	int sock = -1;
	header_t header;

	fd_set master;
	fd_set read_fds;
	int max_desc = 0;
	int i, ret;
	int fin = false;

	t_hilo_personaje hiloPxN;
	memset(&hiloPxN,'\0', sizeof(t_hilo_personaje));

	t_proximoObjetivo proximoObjetivo;
	memset(&proximoObjetivo,'\0', sizeof(t_proximoObjetivo));


	t_objetivosxNivel *oxn = (t_objetivosxNivel*)queue_pop(planDeNiveles);
	hiloPxN.objetivos = *oxn;
	proximoObjetivo.simbolo = hiloPxN.objetivos.objetivos[hiloPxN.objetivosConseguidos];
	hiloPxN.personaje.id = configPersonajeSimbolo();
	strcpy(hiloPxN.personaje.nombre, configPersonajeNombre());
	strcpy(hiloPxN.personaje.nivel, oxn->nivel);
	hiloPxN.personaje.posActual.x = 0;
	hiloPxN.personaje.posActual.y = 0;
	hiloPxN.personaje.recurso = proximoObjetivo.simbolo;

	//char *buffer_header;
	//buffer_header = calloc(1,sizeof(header_t));

	id_proceso = getpid();
	system("clear");

	log_info(LOGGER,"************** Iniciando Personaje '%s' (PID: %d) ***************\n", personaje.nombre, id_proceso);

	/***************** ME CONECTO Y ARMO MENSAJE DE PRESENTACION *******/
	log_info(LOGGER,"************** CONECTANDOSE  ***************\n");
	conectar(personaje.ip_orquestador, personaje.puerto_orquestador, &sock);

	FD_ZERO(&master);
	agregar_descriptor(sock, &master, &max_desc);

	if (enviarMsjNuevoPersonaje(sock) != EXITO)
	{
		log_error(LOGGER,"Error al enviar header NUEVO_PERSONAJE\n\n");
		return WARNING;
	}

	while(!fin)
	{
		FD_ZERO (&read_fds);
		read_fds = master;

		ret = select(max_desc+1, &read_fds, NULL, NULL, NULL);

		if(ret == -1) {
			printf("Personaje: ERROR en select.");
			sleep(1);
		}

		if (ret > 0) {
			for(i = 0; i <= max_desc; i++)
			{

				if (FD_ISSET(i, &read_fds))
				{
					if (i == sock) {

						initHeader(&header);
						recibirHeaderNuevoMsj(sock, &header, &master);

						switch (header.tipo) /*recibo estado */
						{
							case PERSONAJE_CONECTADO: log_info(LOGGER,"PERSONAJE_CONECTADO");
							enviarInfoPersonaje(sock, &hiloPxN);
							break;

							case TURNO_CONCEDIDO: log_info(LOGGER,"TURNO_CONCEDIDO");
							gestionarTurnoConcedido(sock, &proximoObjetivo, &hiloPxN);
							break;

							case UBICACION_RECURSO: log_info(LOGGER, "UBICACION_RECURSO");
							recibirUbicacionRecursoPlanificador( sock, &master, &proximoObjetivo, &hiloPxN);
							break;

							case RECURSO_CONCEDIDO: log_info(LOGGER,"RECURSO_CONCEDIDO");
							gestionarRecursoConcedido(sock, &proximoObjetivo, &hiloPxN);
							break;

							case OTRO: log_info(LOGGER, "que otro??");
							break;

						}

					} else {
						log_debug(LOGGER, "Actividad en el socket %d", i);
					}

				}
			}
		}

	}

	return 0;
}
