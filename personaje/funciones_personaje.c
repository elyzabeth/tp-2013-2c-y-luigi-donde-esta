/*
 * funciones_personaje.c
 *
 *  Created on: 14/10/2013
 *      Author: elyzabeth
 */

#include "personaje.h"


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
	planDeNiveles = configPersonajePlanDeNiveles();
	listaHilosxNivel = list_create();

	pthread_mutex_init (&mutexEnvioMensaje, NULL);
}

/**
 * @NAME: finalizarPersonaje
 * @DESC: Finaliza todas las variables y estructuras que fueron creadas para el proceso personaje
 */
void finalizarPersonaje() {
	log_info(LOGGER, "FINALIZANDO PERSONAJE\n");

	// TODO Bajar Hilos
	// matarHilos()...

	pthread_mutex_destroy(&mutexEnvioMensaje);

	list_destroy_and_destroy_elements(listaHilosxNivel, (void*)destruirHiloPersonaje);
	destruirConfigPersonaje();
	log_destroy(LOGGER);
}


void esperarHilosxNivel() {

	void _join_thread (t_hilo_personaje hilo){
		pthread_join(hilo.tid, NULL);
	}

	list_iterate(listaHilosxNivel, (void*)_join_thread);
}

void levantarHilosxNivel() {
	int i;
	int cant = queue_size(planDeNiveles);
	t_objetivosxNivel *oxn;
	t_hilo_personaje *hiloPersonaje;

	for (i = 0; i < cant; i++) {
		hiloPersonaje = crearHiloPersonaje(personaje);
		oxn = queue_pop(planDeNiveles);
		strcpy(hiloPersonaje->personaje.nivel, oxn->nivel);
		strcpy(hiloPersonaje->personaje.nombre, configPersonajeNombre());
		hiloPersonaje->personaje.id = configPersonajeSimbolo();
		hiloPersonaje->objetivos = *oxn;

		list_add(listaHilosxNivel, hiloPersonaje);

		// Creo el hilo para el nivel
		pthread_create (&hiloPersonaje->tid, NULL, (void*)personajexNivel, (t_hilo_personaje*)hiloPersonaje);

		pthread_join(hiloPersonaje->tid, NULL);
		break;
	}

}

t_hilo_personaje* crearHiloPersonaje() {
	t_hilo_personaje *hiloPersonaje = calloc(1, sizeof(t_hilo_personaje));

	memset(hiloPersonaje, '\0', sizeof(t_hilo_personaje));
	hiloPersonaje->personaje.posActual.x = 0;
	hiloPersonaje->personaje.posActual.y = 0;
	pipe(hiloPersonaje->fdPipe);

	return hiloPersonaje;
}

void destruirHiloPersonaje(t_hilo_personaje* hiloPersonaje) {

	close(hiloPersonaje->fdPipe[0]);
	close(hiloPersonaje->fdPipe[1]);

	free(hiloPersonaje);
}

int recibirHeaderNuevoMsj (int sock, header_t *header) {

	pthread_mutex_lock (&mutexEnvioMensaje);
	char *buffer_header;
	int ret;

	buffer_header = calloc(1,sizeof(header_t));
	memset(&header, '\0', sizeof(header_t));

	ret = recibir (sock, buffer_header, sizeof(header_t));
	memcpy(&header, buffer_header, sizeof(header_t));

	free(buffer_header);
	pthread_mutex_unlock (&mutexEnvioMensaje);

	return ret;
}

int enviarMsjNuevoPersonaje( int sock ) {
	pthread_mutex_lock (&mutexEnvioMensaje);

	header_t header;
	char *buffer_header;
	int ret;

	buffer_header = calloc(1,sizeof(header_t));
	memset(&header, '\0', sizeof(header_t));
	header.tipo = NUEVO_PERSONAJE;
	header.largo_mensaje = 0;

	memset(buffer_header, '\0', sizeof(header_t));
	memcpy(buffer_header, &header, sizeof(header_t));

	log_debug(LOGGER,"sizeof(header): %d, largo mensaje: %d  buffer_header: %lu\n", sizeof(header), header.largo_mensaje, sizeof(&buffer_header));

	ret =  enviar(sock, buffer_header, sizeof(header_t));
	free(buffer_header);

	pthread_mutex_unlock (&mutexEnvioMensaje);

	return ret;
}

int enviarInfoPersonaje(int sock) {
	// PRUEBA
	header_t header;
	t_personaje yo;
	char* buffer;
	log_debug(LOGGER, "Envio mensaje de prueba con info del personaje");

	log_debug(LOGGER, "Datos: (%s, %d, %c)",  configPersonajeNombre(),configPersonajeSimbolo(), configPersonajeSimbolo());
	buffer = calloc(1, sizeof(header_t));

	memset(&yo, '\0', sizeof(t_personaje));
	strcpy(yo.nombre, configPersonajeNombre());
	yo.id = configPersonajeSimbolo();
	yo.posActual.x = 0;
	yo.posActual.y = 0;
	yo.fd = 0;
	strcpy( yo.nivel, "Nivel1");

	memset(&header, '\0', sizeof(header_t));
	header.tipo = CONECTAR_NIVEL;
	header.largo_mensaje = sizeof(t_personaje);

	memcpy(buffer, &header, sizeof(header_t));

	log_debug(LOGGER, "Envio header CONECTAR_NIVEL %d", sizeof(header_t));
	if (enviar(sock, buffer, sizeof(header_t)) != EXITO)
	{
		log_error(LOGGER,"Error al enviar header CONECTAR_NIVEL\n\n");
		free(buffer);
		return WARNING;
	}

	free(buffer);
	buffer = calloc(1, sizeof(t_personaje));
	memcpy(buffer, &yo, sizeof(t_personaje));

	log_debug(LOGGER, "Envio t_personaje %d (%s, %c, %d, %d, %d, %s)", sizeof(t_personaje), yo.nombre, yo.id, yo.posActual.x, yo.posActual.y, yo.fd, yo.nivel);
	if (enviar(sock, buffer, sizeof(t_personaje)) != EXITO)
	{
		log_error(LOGGER,"Error al enviar informacion del personaje\n\n");
		free(buffer);
		return WARNING;
	}

	free(buffer);

	return EXITO;
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

