/*
 * personaje.c
 *
 *  Created on: Sep 24, 2013
 *      Author: elizabeth
 */

#include "personaje.h"

int main (int argc, char *argv[]) {

	signal(SIGINT, per_signal_callback_handler);
	signal(SIGQUIT, per_signal_callback_handler);
	signal(SIGUSR1, per_signal_callback_handler);
	signal(SIGTERM, per_signal_callback_handler);

	inicializarPersonaje();

	// TODO agregar logica del personaje
	principal(argc, argv);
	//test ();

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
	// plan de niveles. Este lo moverá a una cola de finalizados y lo dejará a la espera de que
	// los demás Personajes terminen.

	int finPlanNiveles = 1;
	int _checkEnd(t_hilo_personaje *hilo) {
		return (finPlanNiveles && (hilo->estado == PLAN_NIVEL_FINALIZADO));
	}
	list_iterate(listaHilosxNivel, (void*)_checkEnd);

	if (finPlanNiveles) {

		log_info(LOGGER, "\n\nFINALICE TODO EL PLAN DE NIVELES!!!!!\n\n");
		imprimirVidasyReintentos();

		// informar al orquestador.
		enviarMsjPlanDeNivelesConcluido();

	} else {
		log_info(LOGGER, "\n\nSaliendo del Proceso Personaje sin finalizar todo el plan de niveles.!!!!!\n\n");
	}

	return 0;
}



void* personajexNivel (t_hilo_personaje *hiloPxN) {

	int id_proceso;
	int sock = -1;
	header_t header;

	fd_set master;
	fd_set read_fds;
	int max_desc = 0;
	int i, ret;
	int fin = false;

	t_proximoObjetivo proximoObjetivo;
	memset(&proximoObjetivo, 0, sizeof(t_proximoObjetivo));


	//t_objetivosxNivel *oxn = (t_objetivosxNivel*)queue_pop(planDeNiveles);
//	hiloPxN.objetivos = *oxn;
//	hiloPxN.personaje.id = configPersonajeSimbolo();
//	strcpy(hiloPxN.personaje.nombre, configPersonajeNombre());
//	strcpy(hiloPxN.personaje.nivel, oxn->nivel);
//	hiloPxN.personaje.posActual.x = 0;
//	hiloPxN.personaje.posActual.y = 0;
//	hiloPxN.personaje.recurso = proximoObjetivo.simbolo;

	proximoObjetivo.simbolo = hiloPxN->objetivos.objetivos[hiloPxN->objetivosConseguidos];
	proximoObjetivo.posicion.x=0;
	proximoObjetivo.posicion.y=0;

	id_proceso = getpid();
	system("clear");

	log_info(LOGGER,"************** Iniciando Personaje '%s' (PID: %d) ***************\n", personaje.nombre, id_proceso);

	/***************** ME CONECTO Y ARMO MENSAJE DE PRESENTACION *******/
	log_info(LOGGER,"************** CONECTANDOSE  ***************\n");
	conectar(personaje.ip_orquestador, personaje.puerto_orquestador, &sock);

	FD_ZERO(&master);

	// Agrego descriptor del Pipe con Nivel.
	agregar_descriptor(hiloPxN->fdPipe[0], &master, &max_desc);

	agregar_descriptor(sock, &master, &max_desc);

	if (enviarMsjNuevoPersonaje(sock) != EXITO)
	{
		log_error(LOGGER,"Error al enviar header NUEVO_PERSONAJE %s \n\n", hiloPxN->personaje.nivel);
		fin = true;
	}

	while(!fin)
	{
		FD_ZERO (&read_fds);
		read_fds = master;

		ret = select(max_desc+1, &read_fds, NULL, NULL, NULL);

		if(ret == -1) {
			printf("Personaje: ERROR en select en %s", hiloPxN->personaje.nivel);
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
						initHeader(&header);
						log_info(LOGGER, "Personaje '%c': Recibo mensaje desde Main por Pipe", hiloPxN->personaje.id);
						read (hiloPxN->fdPipe[0], &header, sizeof(header_t));

						log_debug(LOGGER, "Personaje '%c': mensaje recibido '%d'", hiloPxN->personaje.id, header.tipo);
						if (header.tipo == FINALIZAR) {
							log_debug(LOGGER, "Personaje '%c': '%d' ES FINALIZAR", hiloPxN->personaje.id, header.tipo);
							fin = true;
							break;
						}

					} else if (i == sock) {

						// Si NO es un mensaje del hilo principal por Pipe es un mensaje del proceso Plataforma.

						initHeader(&header);
						recibirHeaderNuevoMsj(sock, &header, &master);

						switch (header.tipo)
						{
							case PERSONAJE_CONECTADO: log_info(LOGGER,"PERSONAJE_CONECTADO en %s", hiloPxN->personaje.nivel);
							//hiloPxN.estado = PERSONAJE_CONECTADO;
							enviarInfoPersonaje(sock, hiloPxN);
							break;

							case TURNO_CONCEDIDO: log_info(LOGGER,"TURNO_CONCEDIDO en %s", hiloPxN->personaje.nivel);
							gestionarTurnoConcedido(sock, &proximoObjetivo, hiloPxN);
							break;

							case UBICACION_RECURSO: log_info(LOGGER, "UBICACION_RECURSO en %s", hiloPxN->personaje.nivel);
							recibirUbicacionRecursoPlanificador( sock, &master, &proximoObjetivo, hiloPxN);
							break;

							case RECURSO_CONCEDIDO: log_info(LOGGER,"RECURSO_CONCEDIDO en %s", hiloPxN->personaje.nivel);
							gestionarRecursoConcedido(sock, &proximoObjetivo, hiloPxN, &fin);
							break;

							case RECURSO_INEXISTENTE: log_info(LOGGER,"RECURSO_INEXISTENTE en %s", hiloPxN->personaje.nivel);
							log_error(LOGGER, "ERROR!! \n\nERROR en configuración '%c' RECURSO_INEXISTENTE en %s!!!\n\n", hiloPxN->personaje.recurso, hiloPxN->personaje.nivel);
							fin=true;
							break;

							case OTRO: log_info(LOGGER, "que otro?? %s", hiloPxN->personaje.nivel);
							break;

						}

					} else {
						log_debug(LOGGER, "Actividad en el socket %d", i);
					}

				}
			}
		}

	}

	log_info(LOGGER, "FINALIZANDO Hilo Personaje '%c' Nivel %s\n", hiloPxN->personaje.id, hiloPxN->personaje.nivel);

	pthread_exit(NULL);
}
