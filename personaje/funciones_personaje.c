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

int recibirHeaderNuevoMsj (int sock, header_t *header, fd_set *master) {

	pthread_mutex_lock (&mutexEnvioMensaje);
	int ret, se_desconecto;

	ret = recibir_header(sock, header, master, &se_desconecto);

	pthread_mutex_unlock (&mutexEnvioMensaje);

	return ret;
}

int enviarMsjNuevoPersonaje( int sock ) {

	pthread_mutex_lock (&mutexEnvioMensaje);

	header_t header;
	int ret;

	initHeader(&header);
	header.tipo = NUEVO_PERSONAJE;
	header.largo_mensaje = 0;

	log_debug(LOGGER,"enviarMsjNuevoPersonaje: NUEVO_PERSONAJE sizeof(header): %d, largo mensaje: %d \n", sizeof(header), header.largo_mensaje);

	ret =  enviar_header(sock, &header);

	pthread_mutex_unlock (&mutexEnvioMensaje);

	return ret;
}

int enviarInfoPersonaje(int sock, t_hilo_personaje *hiloPxN) {
	header_t header;
	t_personaje yo = hiloPxN->personaje;

	log_debug(LOGGER, "Envio mensaje con info del personaje");

//	log_debug(LOGGER, "Datos: (%s, %d, %c)",  configPersonajeNombre(),configPersonajeSimbolo(), configPersonajeSimbolo());
	log_debug(LOGGER, "Datos: (%s, %d, %c)",  yo.nombre, yo.id, yo.id);

	initHeader(&header);
	header.tipo = CONECTAR_NIVEL;
	header.largo_mensaje = sizeof(t_personaje);

	log_debug(LOGGER, "Envio header CONECTAR_NIVEL %d", sizeof(header_t));
	if (enviar_header(sock, &header) != EXITO)
	{
		log_error(LOGGER,"Error al enviar header CONECTAR_NIVEL\n\n");
		return WARNING;
	}

	log_debug(LOGGER, "Envio t_personaje %d (%s, %c, %d, %d, %d, %s)", sizeof(t_personaje), yo.nombre, yo.id, yo.posActual.x, yo.posActual.y, yo.fd, yo.nivel);
	if (enviar_personaje(sock, &yo) != EXITO)
	{
		log_error(LOGGER,"Error al enviar informacion del personaje\n\n");
		return WARNING;
	}

	return EXITO;
}


int enviarInfoPersonaje2(int sock) {
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

int enviarSolicitudUbicacion (int sock, t_proximoObjetivo *proximoObjetivo, t_hilo_personaje *hiloPxN) {

	header_t header;
	t_personaje yo = hiloPxN->personaje;

	yo.recurso = proximoObjetivo->simbolo;

	initHeader(&header);
	header.tipo = SOLICITUD_UBICACION;
	header.largo_mensaje = sizeof(t_personaje);

	log_debug(LOGGER, "enviarSolicitudUbicacion: Envio header_t SOLICITUD_UBICACION (size:%d)", sizeof(header_t));
	if (enviar_header(sock, &header) != EXITO)
	{
		log_error(LOGGER,"enviarSolicitudUbicacion: Error al enviar header SOLICITUD_UBICACION\n\n");
		return WARNING;
	}

	log_debug(LOGGER, "enviarSolicitudUbicacion: Envio t_personaje size: %d (%s, %c, pos(%d, %d), fd: %d, nivel: %s, recurso: %c)", sizeof(t_personaje), yo.nombre, yo.id, yo.posActual.x, yo.posActual.y, yo.fd, yo.nivel, yo.recurso);
	if (enviar_personaje(sock, &yo) != EXITO)
	{
		log_error(LOGGER,"enviarSolicitudUbicacion: Error al enviar t_personaje de SOLICITUD_UBICACION\n\n");
		return WARNING;
	}

	return EXITO;
}

int recibirUbicacionRecursoPlanificador( int sock, fd_set *master, t_proximoObjetivo *proximoObjetivo, t_hilo_personaje *hiloPxN ) {
	int ret, se_desconecto;
	t_caja caja;

	log_debug(LOGGER, "recibirUbicacionRecursoPlanificador: Espero recibir estructura t_caja (size:%d)...", sizeof(t_caja));

	if ((ret = recibir_caja(sock, &caja, master, &se_desconecto))!=EXITO) {
		log_error(LOGGER, "recibirUbicacionRecursoPlanificador: ERROR al recibir t_caja con ubicacion del recurso");
	}

	log_debug(LOGGER, "recibirUbicacionRecursoPlanificador: Llego: %s (%c) posicion (%d, %d).", caja.RECURSO, caja.SIMBOLO, caja.POSX, caja.POSY);

	// TODO hacer algo con la info que llega!
	proximoObjetivo->posicion.x = caja.POSX;
	proximoObjetivo->posicion.y = caja.POSY;

	return ret;
}

int enviarSolicitudRecurso (int sock, t_proximoObjetivo *proximoObjetivo, t_hilo_personaje *hiloPxN) {

	header_t header;
	t_personaje yo = hiloPxN->personaje;

	yo.recurso = proximoObjetivo->simbolo;

	initHeader(&header);
	header.tipo = SOLICITUD_RECURSO;
	header.largo_mensaje = sizeof(t_personaje);

	log_debug(LOGGER, "enviarSolicitudRecurso: Envio header_t SOLICITUD_RECURSO (size:%d)", sizeof(header_t));
	if (enviar_header(sock, &header) != EXITO)
	{
		log_error(LOGGER,"enviarSolicitudRecurso: Error al enviar header SOLICITUD_RECURSO\n\n");
		return WARNING;
	}

	log_debug(LOGGER, "enviarSolicitudRecurso: Envio t_personaje size: %d (%s, %c, pos(%d, %d), fd: %d, nivel: %s, recurso: %c)", sizeof(t_personaje), yo.nombre, yo.id, yo.posActual.x, yo.posActual.y, yo.fd, yo.nivel, yo.recurso);
	if (enviar_personaje(sock, &yo) != EXITO)
	{
		log_error(LOGGER,"enviarSolicitudRecurso: Error al enviar t_personaje de SOLICITUD_RECURSO\n\n");
		return WARNING;
	}

	return EXITO;
}

int realizarMovimiento(int sock, t_proximoObjetivo *proximoObjetivo, t_hilo_personaje *hiloPxN) {
	header_t header;
	int ret;

	log_info(LOGGER, "Debo calcular proximo movimiento...");
	if(hiloPxN->posicionActual.x+1<= proximoObjetivo->posicion.x ){
		hiloPxN->posicionActual.x+=1;
	}
	if(hiloPxN->posicionActual.y+1<= proximoObjetivo->posicion.y ){
		hiloPxN->posicionActual.y+=1;
	}

	hiloPxN->personaje.posActual = hiloPxN->posicionActual;

	// Informar movimiento realizado
	initHeader(&header);
	header.tipo = MOVIMIENTO_REALIZADO;
	header.largo_mensaje = sizeof(t_personaje);

	if (enviar_header(sock, &header) != EXITO){
		log_error(LOGGER, "Error al enviar header MOVIMIENTO_REALIZADO");
	}

	ret = enviar_personaje(sock, &hiloPxN->personaje);

	return ret;
}

int gestionarTurnoConcedido(int sock, t_proximoObjetivo *proximoObjetivo, t_hilo_personaje *hiloPxN) {

	log_info(LOGGER, "gestionarTurnoConcedido.");

	// SI no tengo las coordenadas del proximo objetivo las solicito.
	if(!proximoObjetivo->posicion.x && !proximoObjetivo->posicion.y) {
		log_debug(LOGGER, "No tengo coordenadas de proximo objetivo debo solicitar ubicacion.");
		enviarSolicitudUbicacion(sock, proximoObjetivo, hiloPxN);

	} else if (!calcularDistancia(hiloPxN->posicionActual.x, hiloPxN->posicionActual.y, proximoObjetivo->posicion.x, proximoObjetivo->posicion.y)) {
		// SI mi posicion actual es = a la posicion del objetivo
		// Solicitar una instancia del recurso al Planificador.
		log_debug(LOGGER, "Estoy en la caja de recursos, debo solicitar una instancia.");
		enviarSolicitudRecurso(sock, proximoObjetivo, hiloPxN);
	} else {
		// SI tengo coordenadas del objetivo pero no es igual a mi posicion actual.
		// Calcular proximo movimiento, avanzar y notificar al Planificador con un mensaje.
		log_debug(LOGGER, "Debo realizar proximo movimiento...");
		realizarMovimiento(sock, proximoObjetivo, hiloPxN);
	}

	return EXITO;
}

int gestionarRecursoConcedido (int sock, t_proximoObjetivo *proximoObjetivo, t_hilo_personaje *hiloPxN) {

	log_info(LOGGER, "gestionarRecursoConcedido.");
	//TODO agregar logica...
	//	Cuando el recurso sea asignado, el hilo analizará si necesita otro recurso y
	//	volverá al punto 3) (esperar TURNO_CONCEDIDO), o, si ya cumplió sus objetivos del Nivel
	// Notificar a su Planificador que completó los objetivos de ese nivel y desconectarse.


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

