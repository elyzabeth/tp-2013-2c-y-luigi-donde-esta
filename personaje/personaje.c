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

	int id_proceso;
	id_proceso = getpid();

	system("clear");
	log_info(LOGGER,"************** Iniciando Personaje '%s' (PID: %d) ***************\n", personaje.nombre, id_proceso);

	principal(argc, argv);

	finalizarPersonaje();

	return EXIT_SUCCESS;
}


int principal(int argc, char *argv[]) {

	do {
		reiniciar(false);

		levantarHilosxNivel();

		esperarHilosxNivel();

	} while (personaje.reiniciar);

	chequearFinTodosLosNiveles ();

	return 0;
}




/**
 * @NAME: inicializarPersonaje
 * @DESC: Inicializa todas las variables y estructuras necesarias para el proceso personaje
 */
void inicializarPersonaje() {
	// TODO agregar inicializaciones necesarias

	levantarArchivoConfiguracionPersonaje();

	LOGGER = log_create(configPersonajeLogPath(), "PERSONAJE", configPersonajeLogConsola(), configPersonajeLogNivel() );
	log_info(LOGGER, "INICIALIZANDO PERSONAJE '%s' ", configPersonajeNombre());

	strcpy(personaje.nombre, configPersonajeNombre());
	strcpy(personaje.ip_orquestador, configPersonajePlataformaIp());
	personaje.puerto_orquestador = configPersonajePlataformaPuerto();

	reiniciar(false);

	listaHilosxNivel = list_create();

	pthread_mutex_init (&mutexEnvioMensaje, NULL);
	pthread_mutex_init (&mutexVidas, NULL);
	pthread_mutex_init (&mutexListaHilosxNivel, NULL);
	pthread_mutex_init (&mutexReinicio, NULL);

	REINTENTOS = 0;

	inicializarVariablesGlobales();
}

void inicializarVariablesGlobales() {

	reiniciar(true);
	VIDAS = configPersonajeVidas();

	if (planDeNiveles != NULL)
		queue_destroy_and_destroy_elements(planDeNiveles, (void*)destruirObjetivosxNivel);

	planDeNiveles = configPersonajePlanDeNiveles();

	list_clean_and_destroy_elements(listaHilosxNivel, (void*)destruirEstructuraHiloPersonaje);
}

void reiniciar(bool valor) {
	pthread_mutex_lock (&mutexReinicio);
	personaje.reiniciar = valor;
	pthread_mutex_unlock (&mutexReinicio);
}

/**
 * @NAME: finalizarPersonaje
 * @DESC: Finaliza todas las variables y estructuras que fueron creadas para el proceso personaje
 */
void finalizarPersonaje() {
	log_info(LOGGER, "FINALIZANDO PROCESO PERSONAJE\n");
	reiniciar(false);

	// TODO Bajar Hilos
	finalizarHilosPersonaje();

	pthread_mutex_destroy(&mutexEnvioMensaje);
	pthread_mutex_destroy(&mutexVidas);
	pthread_mutex_destroy(&mutexListaHilosxNivel);
	pthread_mutex_destroy(&mutexReinicio);

	list_destroy_and_destroy_elements(listaHilosxNivel, (void*)destruirEstructuraHiloPersonaje);
	queue_destroy_and_destroy_elements(planDeNiveles, (void*)destruirObjetivosxNivel);

	destruirConfigPersonaje();
	log_destroy(LOGGER);
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
		log_debug(LOGGER, "\nhilo %d/de %d) Envio mensaje de FINALIZAR a hilo Personaje '%c' (%s) - por Pipe socket: %d", ++i, cantHilosPersonaje, hPersonaje->personaje.id, hPersonaje->personaje.nivel, hPersonaje->fdPipe[1]);
		write(hPersonaje->fdPipe[1], buffer_header, sizeof(header_t));
		pthread_join(hPersonaje->tid, NULL);
	}

	list_iterate(listaHilosxNivel, (void*)_finalizar_hilo);

	list_clean_and_destroy_elements(listaHilosxNivel, (void*)destruirEstructuraHiloPersonaje);

	free(buffer_header);
}

void esperarHilosxNivel() {


	void _join_thread (t_hilo_personaje *hilo){
		pthread_join(hilo->tid, NULL);
	}

	list_iterate(listaHilosxNivel, (void*)_join_thread);

}

void levantarHilosxNivel() {
	int i;
	int cant = queue_size(planDeNiveles);
	t_objetivosxNivel *oxn;
	t_hilo_personaje *hiloPersonaje;

	for (i = 0; i < cant; i++) {

		oxn = queue_pop(planDeNiveles);

		hiloPersonaje = crearEstructuraHiloPersonaje(oxn);

		list_add(listaHilosxNivel, hiloPersonaje);

		log_debug(LOGGER, "Hilo para nivel %s", oxn->nivel);
		log_debug(LOGGER, "%s de %s pipe: %d y %d", hiloPersonaje->personaje.nombre, hiloPersonaje->personaje.nivel, hiloPersonaje->fdPipe[0], hiloPersonaje->fdPipe[1]);

		// Creo el hilo para el nivel
		pthread_create (&(hiloPersonaje->tid), NULL, (void*)personajexNivel, (t_hilo_personaje*)hiloPersonaje);

		queue_push(planDeNiveles, oxn);

	}

}

t_hilo_personaje* crearEstructuraHiloPersonaje(t_objetivosxNivel *oxn) {
	t_hilo_personaje *hiloPersonaje;

	hiloPersonaje = calloc(1, sizeof(t_hilo_personaje));

	strcpy(hiloPersonaje->personaje.nombre, configPersonajeNombre());
	strcpy(hiloPersonaje->personaje.nivel, oxn->nivel);
	hiloPersonaje->objetivos = *oxn;
	hiloPersonaje->personaje.id = configPersonajeSimbolo();
	hiloPersonaje->personaje.recurso = '-';
	hiloPersonaje->personaje.posActual.x = 0;
	hiloPersonaje->personaje.posActual.y = 0;
	hiloPersonaje->moverPorX=true;

	if (pipe(hiloPersonaje->fdPipe) == -1)
	{
		perror ("crearEstructuraHiloPersonaje: No se puede crear Tuberia de comunicacion.");
		exit (-1);
	}
log_debug(LOGGER, "%s pipe: %d y %d", hiloPersonaje->personaje.nombre, hiloPersonaje->fdPipe[0], hiloPersonaje->fdPipe[1]);

	return hiloPersonaje;
}

void destruirEstructuraHiloPersonaje(t_hilo_personaje* hiloPersonaje) {

	close(hiloPersonaje->fdPipe[0]);
	close(hiloPersonaje->fdPipe[1]);

	free(hiloPersonaje);
}


int chequearFinTodosLosNiveles () {
	// TODO	Al concluir todos los niveles, se conectará al Orquestador y notificará que concluyó su
	// plan de niveles. Este lo moverá a una cola de finalizados y lo dejará a la espera de que
	// los demás Personajes terminen.

	int finPlanNiveles = 1;

	int _checkEnd(t_hilo_personaje *hilo) {
		log_debug(LOGGER, "chequearFinTodosLosNiveles: %s - hilo->estado == %s", hilo->personaje.nivel, getStringTipo(hilo->estado));
		finPlanNiveles = finPlanNiveles && (hilo->estado == PLAN_NIVEL_FINALIZADO);
		return finPlanNiveles;
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
	pthread_mutex_lock (&mutexVidas);
	VIDAS++;
	log_info(LOGGER, "\n\nEl Personaje incrementa vidas.\n");
	imprimirVidasyReintentos();
	//TODO agregar logica luego de incrementar vidas si corresponde
	pthread_mutex_unlock (&mutexVidas);

	return VIDAS;
}

int32_t decrementarVida() {

	pthread_mutex_lock (&mutexVidas);
	int ret;
	if(VIDAS > 0) {
		VIDAS--;
		log_info(LOGGER, "Personaje decrementa 1 vida.\n");
		imprimirVidasyReintentos();
		ret = VIDAS;
	} else {
		log_info(LOGGER, "\nPersonaje No tiene VIDAS disponibles.\n");
		imprimirVidasyReintentos();
		ret = -1;
	}
	pthread_mutex_unlock (&mutexVidas);

	return ret;
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
		sleep(2);
		printf("\n\nKnock, knock, Neo...\n\n");
		sleep(2);
		printf("You take the blue pill and the story ends. You wake in your bed and you believe whatever you want to believe.\n");
		printf("You take the red pill and you stay in Wonderland and I show you how deep the rabbit-hole goes.\n");
		printf("Remember that all I'am offering is the truth. Nothing more. \n");
		printf("\nThis is your last chance. After this there is no turning back... \n\n Choose blue or red pill b/r: ");
		printf("\n(traducción: Desea reiniciar el juego? s/n:) ");

		while ((respuesta=getc(stdin)) != 's' && respuesta != 'b' && respuesta != 'n' && respuesta != 'r')
			printf("\nPor favor ingrese 's' o 'b' para reiniciar, o ingrese 'n' o 'r' para terminar: ");

		if(respuesta == 's' || respuesta == 'b') {
			REINTENTOS++;
			log_info(LOGGER, "VOLVIENDO A MATRIX...\n\nREINICIANDO EL JUEGO...");

			// TODO llamar funcion que reinicie el juego
			inicializarVariablesGlobales();

			imprimirVidasyReintentos();


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
