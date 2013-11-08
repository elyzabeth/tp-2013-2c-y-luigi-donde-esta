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
	finalizarHilosPersonaje();

	pthread_mutex_destroy(&mutexEnvioMensaje);

	list_destroy_and_destroy_elements(listaHilosxNivel, (void*)destruirEstructuraHiloPersonaje);
	destruirConfigPersonaje();
	log_destroy(LOGGER);
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
		pipe(hiloPersonaje->fdPipe);

		list_add(listaHilosxNivel, hiloPersonaje);

		log_debug(LOGGER, "Hilo para nivel %s", oxn->nivel);
		log_debug(LOGGER, "%s de %s pipe: %d y %d", hiloPersonaje->personaje.nombre, hiloPersonaje->personaje.nivel, hiloPersonaje->fdPipe[0], hiloPersonaje->fdPipe[1]);
		// Creo el hilo para el nivel
		pthread_create (&hiloPersonaje->tid, NULL, (void*)personajexNivel, (t_hilo_personaje*)hiloPersonaje);


		//log_debug(LOGGER, "Hilo tid %d", hiloPersonaje->tid);

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

//	if (pipe(hiloPersonaje->fdPipe) == -1)
//	{
//		perror ("crearEstructuraHiloPersonaje: No se puede crear Tuberia de comunicacion.");
//		exit (-1);
//	}
//log_debug(LOGGER, "%s pipe: %d y %d", hiloPersonaje->personaje.nombre, hiloPersonaje->fdPipe[0], hiloPersonaje->fdPipe[1]);

	return hiloPersonaje;
}

void destruirEstructuraHiloPersonaje(t_hilo_personaje* hiloPersonaje) {

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

	hiloPxN->estado = CONECTAR_NIVEL;
	log_debug(LOGGER, "Envio t_personaje %d (%s, %c, %d, %d, %d, %s)", sizeof(t_personaje), yo.nombre, yo.id, yo.posActual.x, yo.posActual.y, yo.fd, yo.nivel);
	if (enviar_personaje(sock, &yo) != EXITO)
	{
		log_error(LOGGER,"Error al enviar informacion del personaje\n\n");
		return WARNING;
	}

	return EXITO;
}

int enviarSolicitudUbicacion (int sock, t_proximoObjetivo *proximoObjetivo, t_hilo_personaje *hiloPxN) {

	header_t header;
	t_personaje yo;

	hiloPxN->personaje.recurso = proximoObjetivo->simbolo;

	yo = hiloPxN->personaje;

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
	t_personaje yo;

	hiloPxN->personaje.recurso = proximoObjetivo->simbolo;
	yo = hiloPxN->personaje;


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

	hiloPxN->estado = SOLICITUD_RECURSO;

	return EXITO;
}

int enviarMsjPlanDeNivelFinalizado( int sock , t_hilo_personaje *hiloPxN) {

	pthread_mutex_lock (&mutexEnvioMensaje);
	t_personaje yo;
	yo = hiloPxN->personaje;

	header_t header;
	int ret;

	initHeader(&header);
	header.tipo = PLAN_NIVEL_FINALIZADO;
	header.largo_mensaje = 0;

	log_debug(LOGGER,"enviarMsjPlanDeNivelFinalizado: PLAN_NIVEL_FINALIZADO %s sizeof(header): %d, largo mensaje: %d \n", hiloPxN->personaje.nivel, sizeof(header), header.largo_mensaje);

	ret =  enviar_header(sock, &header);

	if (enviar_personaje(sock, &yo) != EXITO)
	{
		log_error(LOGGER,"enviarMsjPlanDeNivelFinalizado: Error al enviar t_personaje de PLAN_NIVEL_FINALIZADO\n\n");
		return WARNING;
	}

	pthread_mutex_unlock (&mutexEnvioMensaje);

	return ret;
}




int realizarMovimiento(int sock, t_proximoObjetivo *proximoObjetivo, t_hilo_personaje *hiloPxN) {
	header_t header;
	int ret;
	int flag = false;

	if (hiloPxN->moverPorX || hiloPxN->personaje.posActual.y == proximoObjetivo->posicion.y) {
		if(hiloPxN->personaje.posActual.x < proximoObjetivo->posicion.x ) {
			hiloPxN->personaje.posActual.x++;
			flag = true;
		} else if(hiloPxN->personaje.posActual.x > proximoObjetivo->posicion.x ) {
			hiloPxN->personaje.posActual.x--;
			flag = true;
		}

	}
	if (!flag) {
		if ((!hiloPxN->moverPorX || hiloPxN->personaje.posActual.x == proximoObjetivo->posicion.x)) {
			if(hiloPxN->personaje.posActual.y < proximoObjetivo->posicion.y ){
				hiloPxN->personaje.posActual.y++;
			} else if(hiloPxN->personaje.posActual.y > proximoObjetivo->posicion.y ){
				hiloPxN->personaje.posActual.y--;
			}
		}
	}

	hiloPxN->moverPorX = !hiloPxN->moverPorX;

	log_debug(LOGGER, "Informar MOVIMIENTO_REALIZADO (%d, %d)", hiloPxN->personaje.posActual.x, hiloPxN->personaje.posActual.y);
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
		//hiloPxN.estado = SOLICITUD_UBICACION;

	} else if (!calcularDistanciaCoord(hiloPxN->personaje.posActual, proximoObjetivo->posicion) && hiloPxN->estado != SOLICITUD_RECURSO ) {
		// SI mi posicion actual es = a la posicion del objetivo
		// Solicitar una instancia del recurso al Planificador.


		log_debug(LOGGER, "Estoy en la caja de recursos, debo solicitar una instancia.");
		enviarSolicitudRecurso(sock, proximoObjetivo, hiloPxN);

	} else {
		//hiloPxN.estado = TURNO_CONCEDIDO;
		// SI tengo coordenadas del objetivo pero no es igual a mi posicion actual.
		// Calcular proximo movimiento, avanzar y notificar al Planificador con un mensaje.
		log_debug(LOGGER, "Debo realizar proximo movimiento...");
		realizarMovimiento(sock, proximoObjetivo, hiloPxN);
	}

	return EXITO;
}

int gestionarRecursoConcedido (int sock, t_proximoObjetivo *proximoObjetivo, t_hilo_personaje *hiloPxN, int *fin) {

	hiloPxN->objetivosConseguidos++;
	hiloPxN->estado = RECURSO_CONCEDIDO;
	log_info(LOGGER, "gestionarRecursoConcedido. objetivosConseguidos: %d/%d", hiloPxN->objetivosConseguidos,hiloPxN->objetivos.totalObjetivos );

	// TODO agregar logica...
	//	Cuando el recurso sea asignado, el hilo analizará si necesita otro recurso y
	//	volverá al punto 3) (esperar TURNO_CONCEDIDO), o, si ya cumplió sus objetivos del Nivel
	// Notificar a su Planificador que completó los objetivos de ese nivel y desconectarse.
	if(hiloPxN->objetivosConseguidos == hiloPxN->objetivos.totalObjetivos){

		// TODO Se completo el nivel
		log_info(LOGGER, "COMPLETE EL PLAN DEL %s !!!\n", hiloPxN->personaje.nivel);
		hiloPxN->estado = PLAN_NIVEL_FINALIZADO;
		enviarMsjPlanDeNivelFinalizado(sock, hiloPxN);
		*fin = true;

	} else if (hiloPxN->objetivosConseguidos < hiloPxN->objetivos.totalObjetivos) {

		// Todavia quedan recursos por conseguir.
		proximoObjetivo->simbolo = hiloPxN->objetivos.objetivos[hiloPxN->objetivosConseguidos];
		proximoObjetivo->posicion.x = 0;
		proximoObjetivo->posicion.y = 0;

	}

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





