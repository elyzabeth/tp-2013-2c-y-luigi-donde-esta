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



void* personajexNivel (t_hilo_personaje* hiloPxN) {

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
	// TODO PIERDO LOS SOCKETS PIPE !!!! CORREGIR ESTE BUG!!!!
	log_info(LOGGER,"%s de %s - agregar_descriptor hiloPxN->fdPipe[0]: '%d' \n", hiloPxN->personaje.nombre, hiloPxN->personaje.nivel, hiloPxN->fdPipe[0]);
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
							log_debug(LOGGER, "\n\nPersonaje '%c' de %s: '%d' ES FINALIZAR", hiloPxN->personaje.id, hiloPxN->personaje.nivel, header.tipo);
							fin = true;

							// TODO enviar mensaje al planificador???

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

	log_info(LOGGER, "\n\nFINALIZANDO Hilo Personaje '%c' Nivel %s\n", hiloPxN->personaje.id, hiloPxN->personaje.nivel);

	pthread_exit(NULL);
}



void enviarMsjPlanDeNivelesConcluido() {
	int ret, sock = -1;
	header_t header;

	conectar(personaje.ip_orquestador, personaje.puerto_orquestador, &sock);

	initHeader(&header);
	header.tipo = PLAN_NIVELES_CONCLUIDO;
	header.largo_mensaje = 0;
	header.id[0] = configPersonajeSimbolo();

	log_info(LOGGER, "\n\nEnviando PLAN_NIVELES_CONCLUIDO al orquestador.");
	if ((ret = enviar_header(sock, &header)) != EXITO) {
		log_error(LOGGER, "\n\nERROR AL ENVIAR PLAN_NIVELES_CONCLUIDO al ORQUESTADOR!!!\n\n");
	}
}


// TODO el mensaje de muerte lo envia el hilo pricipal del personaje al orquestador?
// o el hilo principal se lo comunica a cada hilo hijo y estos se lo comunican a sus respectivos planificadores??
void enviarMsjMuerteDePersonajeAlOrq () {
	int ret, sock = -1;
	header_t header;

	conectar(personaje.ip_orquestador, personaje.puerto_orquestador, &sock);

	initHeader(&header);
	header.tipo = MUERTE_PERSONAJE;
	header.largo_mensaje = 0;
	header.id[0] = configPersonajeSimbolo();

	log_info(LOGGER, "\n\nEnviando MUERTE_PERSONAJE al orquestador.");
	if ((ret = enviar_header(sock, &header)) != EXITO) {
		log_error(LOGGER, "\n\nERROR AL ENVIAR MUERTE_PERSONAJE al ORQUESTADOR!!!\n\n");
	}

}


void imprimirVidasyReintentos() {
	log_info(LOGGER, "\n\nVIDAS Y REINTENTOS de %s\n******************\nVIDAS: %d\nREINTENTOS: %d\n\n", personaje.nombre, VIDAS, REINTENTOS);
}


int32_t incrementarVida() {
	VIDAS++;
	log_info(LOGGER, "\n\nEl Personaje incrementa vidas.\n");
	imprimirVidasyReintentos();
	//TODO agregar logica luego de incrementar vidas si corresponde
	return VIDAS;
}

int32_t decrementarVida() {

	if(VIDAS > 0) {
		VIDAS--;
		log_info(LOGGER, "Personaje decrementa 1 vida.\n");
		imprimirVidasyReintentos();
		return VIDAS;
	} else {
		log_info(LOGGER, "\nPersonaje No tiene VIDAS disponibles.\n");
		imprimirVidasyReintentos();
		return -1;
	}
}


/**
 * @NAME: finalizarHilosPersonaje
 * @DESC: Finaliza los hilos de personajes creados para cada nivel enviandoles por pipe el mensaje FINALIZAR
 */
void finalizarHilosPersonaje() {
	int i = 0;
	int32_t cantHilosPersonaje;
	header_t header;

	cantHilosPersonaje = list_size(listaHilosxNivel);

	if(cantHilosPersonaje <= 0)
		return;

	char* buffer_header = calloc(1,sizeof(header_t));

	log_info(LOGGER, "FINALIZANDO HILOS Personaje");

	initHeader(&header);
	header.tipo = FINALIZAR;
	header.largo_mensaje=0;

	memset(buffer_header, '\0', sizeof(header_t));
	memcpy(buffer_header, &header, sizeof(header_t));

	void _finalizar_hilo(t_hilo_personaje *hPersonaje) {
		log_debug(LOGGER, "hilo/de  %d/%d) Envio mensaje de FINALIZAR a hilo Personaje '%c' (%s)", ++i, cantHilosPersonaje, hPersonaje->personaje.id, hPersonaje->personaje.nivel);
		write(hPersonaje->fdPipe[1], buffer_header, sizeof(header_t));
		pthread_join(hPersonaje->tid, NULL);
		sleep(3);
	}

	list_iterate(listaHilosxNivel, (void*)_finalizar_hilo);

	free(buffer_header);
}


/*
 * Si no le quedaran vidas disponibles, el Personaje deberá interrumpir todos sus planes de
 * niveles y mostrar en pantalla un mensaje preguntando al usuario si desea reiniciar el juego,
 * informando también la cantidad de reintentos que ya se realizaron. De aceptar, el Personaje
 * incrementará su contador de reintentos y reiniciará su Plan de Niveles. En caso negativo, el
 * Personaje se cerrará, abandonando el juego.
 */
void manejoSIGTERM() {
	char respuesta;
	int vidas_restantes = decrementarVida();

	if (vidas_restantes == -1) {
		//TODO interrumpir todos los planes de niveles
		// llamar funcion que baje los hilos y ver que otras variables hay que reiniciar!!
		finalizarHilosPersonaje();

		printf("\n\nKnock, knock, Neo...\n\n");
		sleep(2);
		printf("You take the blue pill and the story ends. You wake in your bed and you believe whatever you want to believe.\n");
		printf("You take the red pill and you stay in Wonderland and I show you how deep the rabbit-hole goes.\n");
		printf("Remember that all I am offering is the truth. Nothing more... \n");
		printf("Take blue or red pill b/r: ");
		printf("(traducción: Desea reiniciar el juego? s/n:) ");

		while ((respuesta=getc(stdin)) != 's' && respuesta != 'b' && respuesta != 'n' && respuesta != 'r')
			printf("\nPor favor ingrese 's' o 'b' para reiniciar, o ingrese 'n' o 'r' para terminar: ");

		if(respuesta == 's' || respuesta == 'b') {
			REINTENTOS++;
			log_info(LOGGER, "VOLVIENDO A MATRIX...\n\nREINICIANDO EL JUEGO...");
			imprimirVidasyReintentos();
			// TODO llamar funcion que reinicie el juego

		} else {
			log_info(LOGGER, "SALIENDO DE MATRIX....\n\n CERRANDO PROCESO PERSONAJE");

			finalizarPersonaje();

			exit(0);
		}
	}

}


/*
 * @NAME: per_signal_callback_handler
 * @DESC: Define la funcion a llamar cuando una señal es enviada al proceso
 * ctrl-c (SIGINT)
 */
void per_signal_callback_handler(int signum)
{
	log_info(LOGGER, "INTERRUPCION POR SEÑAL: %d = %s \n", signum, strsignal(signum));

	switch(signum) {

	case SIGUSR1: // SIGUSR1=10 ( kill -s USR1 <PID> )
		log_info(LOGGER, " - LLEGO SEÑAL SIGUSR1\n");
		//Debo incrementar 1 vida
		incrementarVida();
		break;
	case SIGTERM: // SIGTERM=15 ( kill <PID>)
		log_info(LOGGER, " - LLEGO SEÑAL SIGTERM\n");
		manejoSIGTERM();
		break;
	case SIGINT: // SIGINT=2 (ctrl-c)
		log_info(LOGGER, " - LLEGO SEÑAL SIGINT\n");
		finalizarPersonaje();
		// Termino el programa
		exit(signum);
		break;
	case SIGKILL: // SIGKILL=9 ( kill -9 <PID>)
		log_info(LOGGER, " - LLEGO SEÑAL SIGKILL\n");
		finalizarPersonaje();
		// Termino el programa
		exit(signum);
		break;
	case SIGQUIT: // SIGQUIT=3 (ctrl-4 o kill -s QUIT <PID>)
		log_info(LOGGER, " - LLEGO SEÑAL SIGQUIT\n");
		finalizarPersonaje();
		// Termino el programa
		exit(signum);
		break;
	}

}
