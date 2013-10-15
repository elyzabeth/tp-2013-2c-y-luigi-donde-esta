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
	int ret = principal(argc, argv);

	finalizarPersonaje();

	return EXIT_SUCCESS;
}

int principal(int argc, char *argv[]) {

	//personaje_t personaje; // Lo hago global para poder inicializarlo desde funcion
	int id_proceso;
	int sock = -1;
	header_t header;
	//char *buffer_header; // Lo hago global para poder inicializarlo y liberarlo desde funcion
	/************************************ Inicio **************************************/
	/*
	  if(argc != 3)
	  {
	    	puts("Error cantidad Parametros");
	  }
	 */
	id_proceso = getpid();
	system("clear");

	log_info(LOGGER,"************** Iniciando Personaje '%s' (PID: %d) ***************\n", personaje.nombre, id_proceso);
	//cambiar_nombre_proceso(argv,argc, personaje.nombre);

	/***************** ME CONECTO Y ARMO MENSAJE DE PRESENTACION *******/
	log_info(LOGGER,"************** CONECTANDOSE  ***************\n");
	conectar(personaje.ip_orquestador, personaje.puerto_orquestador, &sock);


	if (enviarMsjNuevoPersonaje(sock) != EXITO)
	{
		log_error(LOGGER,"Error al enviar header NUEVO_PERSONAJE\n\n");
		return WARNING;
	}

	while(1)
	{

		recibir (sock, buffer_header, sizeof(header_t));
		memcpy(&header, buffer_header, sizeof(header_t));

		switch (header.tipo) /*recibo estado */
		{
			case PERSONAJE_CONECTADO: log_info(LOGGER,"Personaje Conectado");
				enviarInfoPersonaje(sock);
			break;

			case OTRO: log_info(LOGGER, "otro");
			break;

		}

		sleep(15);
	}

	//free (buffer_header); /* Solo al final porque lo uso siempre */ Movido a finalizarPersonaje()
	return 0;
}


/**
 * @NAME: inicializarPersonaje
 * @DESC: Inicializa todas las variables y estructuras necesarias para el proceso personaje
 */
void inicializarPersonaje() {
	levantarArchivoConfiguracionPersonaje();
	// TODO agregar inicializaciones necesarias
	LOGGER = log_create(configPersonajeLogPath(), "PERSONAJE", configPersonajeLogConsola(), configPersonajeLogNivel() );
	log_info(LOGGER, "INICIALIZANDO PERSONAJE '%s' ", configPersonajeNombre());

	strcpy(personaje.nombre, configPersonajeNombre());
	strcpy(personaje.ip_orquestador, configPersonajePlataformaIp());
	personaje.puerto_orquestador = configPersonajePlataformaPuerto();
	VIDAS = configPersonajeVidas();
	REINTENTOS = 0;

	buffer_header = calloc(1,sizeof(header_t)); /*primera y unica vez */
}

/**
 * @NAME: finalizarPersonaje
 * @DESC: Finaliza todas las variables y estructuras que fueron creadas para el proceso personaje
 */
void finalizarPersonaje() {
	log_info(LOGGER, "FINALIZANDO PERSONAJE\n");
	free (buffer_header);
	destruirConfigPersonaje();
	log_destroy(LOGGER);
}

/*
 * En caso de tener vidas disponibles, el Personaje se descontará una vida, volverá a conectarse
 * al hilo Orquestador y le notificará su intención de iniciar nuevamente el Nivel en que estaba jugando.
 */
void muertePersonaje(MOTIVO_MUERTE motivo) {
	//TODO hacer lo necesario antes de morir, informar el motivo de muerte por pantalla y al planificador/orquestador
	decrementarVida();
	switch(motivo){
		case MUERTE_POR_ENEMIGO:
			log_info(LOGGER, "Muerte por enemigo");
			break;
		case MUERTE_POR_INTERBLOQUEO:
			log_info(LOGGER, "Muerte por interbloqueo");
			break;
//		case MUERTE_POR_SIGKILL:
//			break;
//		case MUERTE_POR_SIGTERM:
//			break;
		default: log_info(LOGGER, "Motivo de muerte no valido"); break;
	}
	// TODO llamar funcion para reiniciar el juego

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

int32_t incrementarVida() {
	VIDAS++;
	log_info(LOGGER, "Personaje incrementa vidas. VIDAS restantes: %d - Reintentos: %d\n", VIDAS, REINTENTOS);
	//TODO agregar logica luego de incrementar vidas si corresponde
	return VIDAS;
}

int32_t decrementarVida() {
	if(VIDAS > 0) {
		VIDAS--;
		log_info(LOGGER, "Personaje decrementa 1 vida. VIDAS restantes: %d - Reintentos: %d\n", VIDAS, REINTENTOS);
		return VIDAS;
	} else {
		log_info(LOGGER, "Personaje No tiene VIDAS disponibles. VIDAS restantes: %d - Reintentos: %d\n", VIDAS, REINTENTOS);
		return -1;
	}
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

	if (vidas_restantes == -1){
		//TODO interrumpir todos los planes de niveles

		printf("Desea reiniciar el juego? s/n: ");

		while ((respuesta=getc(stdin)) != 's' && respuesta != 'n')
			printf("\nPor favor ingrese 's' o 'n': ");

		if(respuesta == 's'){
			REINTENTOS++;
			log_info(LOGGER, "REINICIANDO EL JUEGO (reintentos: %d)...", REINTENTOS);
			// TODO llamar funcion que reinicie el juego
		} else {
			log_info(LOGGER, "CERRANDO PROCESO PERSONAJE");
			// TODO llamar funcion que baje los hilos
			finalizarPersonaje();
			exit(0);
		}
	}

}

