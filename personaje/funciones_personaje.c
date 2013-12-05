/*
 * funciones_personaje.c
 *
 *  Created on: 14/10/2013
 *      Author: elyzabeth
 */

#include "personaje.h"

void initProximoObjetivo ( t_proximoObjetivo *proximoObjetivo );
void setearProximoObjetivo ( t_proximoObjetivo *proximoObjetivo, t_hilo_personaje* hiloPxN );
int muertePersonaje(MOTIVO_MUERTE motivo, t_proximoObjetivo *proximoObjetivo, t_hilo_personaje *hiloPxN, int *sock, fd_set *master, int *maxDesc );

void* personajexNivel (t_hilo_personaje* hiloPxN) {

	int idProcesoHilo;
	int sock = -1;

	fd_set master;
	fd_set read_fds;
	int max_desc = 0;
	int i, ret;
	int fin = false;
	header_t header;
	t_proximoObjetivo proximoObjetivo;

	idProcesoHilo = getpid();
	//system("clear");

	log_info(LOGGER,"\n\n\n************** Iniciando Personaje '%s' del nivel %s (PID: %d) ***************\n", personaje.nombre, hiloPxN->personaje.nivel, idProcesoHilo);


	initProximoObjetivo(&proximoObjetivo);
	setearProximoObjetivo(&proximoObjetivo, hiloPxN);

	/***************** ME CONECTO Y ARMO MENSAJE DE PRESENTACION *******/
	log_info(LOGGER,"************** CONECTANDOSE  ***************\n");
	conectar(personaje.ip_orquestador, personaje.puerto_orquestador, &sock);

	FD_ZERO(&master);

	// Agrego descriptor del Pipe con Nivel.
	log_info(LOGGER,"%s de %s - agregar_descriptor hiloPxN->fdPipe[0]: '%d' \n", hiloPxN->personaje.nombre, hiloPxN->personaje.nivel, hiloPxN->fdPipe[0]);
	agregar_descriptor(hiloPxN->fdPipe[0], &master, &max_desc);

	agregar_descriptor(sock, &master, &max_desc);

	if (enviarMsjNuevoPersonaje(sock, hiloPxN) != EXITO) {
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
							enviarMsjMuertePersonajePlan(sock, hiloPxN);

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

							case NIVEL_INEXISTENTE: log_info(LOGGER,"\n\nPersonaje: %s - NIVEL_INEXISTENTE (%s) !!!! \n\n", hiloPxN->personaje.nombre, hiloPxN->personaje.nivel);
							hiloPxN->estado = NIVEL_INEXISTENTE;
							fin=true;
							// TODO hago algo mas si el nivel no existe??
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

							case MUERTE_PERSONAJE_XENEMIGO: log_info(LOGGER,"MUERTE_PERSONAJE_XENEMIGO en %s", hiloPxN->personaje.nivel);
							fin = muertePersonaje(MUERTE_POR_ENEMIGO, &proximoObjetivo, hiloPxN, &sock, &master, &max_desc);
							break;

							case MUERTE_PERSONAJE_XRECOVERY: log_info(LOGGER,"MUERTE_PERSONAJE_XRECOVERY en %s ", hiloPxN->personaje.nivel);
							fin = muertePersonaje(MUERTE_POR_INTERBLOQUEO, &proximoObjetivo, hiloPxN, &sock, &master, &max_desc);
							break;

							case OTRO: log_info(LOGGER, "que otro?? %s", hiloPxN->personaje.nivel);
							break;

							default: log_error(LOGGER, "\n\n Personaje '%c': ERROR mensaje NO RECONOCIDO (%d) !!\n",  hiloPxN->personaje.id, header.tipo);
						}

					} else {
						log_debug(LOGGER, "\n\n SOCKET NO RECONOCIDO!! Actividad en el socket %d \n\n", i);
					}

				}
			}
		}

	}

	log_info(LOGGER, "\n\nFINALIZANDO Hilo Personaje '%c' Nivel %s\n", hiloPxN->personaje.id, hiloPxN->personaje.nivel);
	close(sock);

	//destruirEstructuraHiloPersonaje(quitarHiloPersonajexTid(hiloPxN->tid));

	pthread_exit(NULL);
}

void setearProximoObjetivo ( t_proximoObjetivo *proximoObjetivo, t_hilo_personaje* hiloPxN ) {
	proximoObjetivo->simbolo = hiloPxN->objetivos.objetivos[hiloPxN->objetivosConseguidos];
	proximoObjetivo->posicion.x = 0;
	proximoObjetivo->posicion.y = 0;
}

void initProximoObjetivo ( t_proximoObjetivo *proximoObjetivo ) {
	memset(proximoObjetivo, 0, sizeof(t_proximoObjetivo));
}

t_hilo_personaje* quitarHiloPersonajexTid (int32_t tid) {

	pthread_mutex_lock (&mutexListaHilosxNivel);
	t_hilo_personaje* hilo;
	bool _buscar_x_tid (t_hilo_personaje* h) {
		return (h->tid == tid);
	}
	hilo = list_remove_by_condition(listaHilosxNivel, (void*)_buscar_x_tid);
	pthread_mutex_unlock (&mutexListaHilosxNivel);

	return hilo;
}


int recibirHeaderNuevoMsj (int sock, header_t *header, fd_set *master) {

	//pthread_mutex_lock (&mutexEnvioMensaje);
	int ret, se_desconecto;

	ret = recibir_header(sock, header, master, &se_desconecto);

	//pthread_mutex_unlock (&mutexEnvioMensaje);

	return ret;
}

int enviarMsjNuevoPersonaje( int sock, t_hilo_personaje *hiloPxN ) {

	header_t header;
	int ret;

	initHeader(&header);
	header.tipo = NUEVO_PERSONAJE;
	header.largo_mensaje = 0;

	log_debug(LOGGER,"enviarMsjNuevoPersonaje: NUEVO_PERSONAJE sizeof(header): %d, largo mensaje: %d \n", sizeof(header), header.largo_mensaje);

	if ( (ret =  enviar_header(sock, &header)) != EXITO ) {
		log_error(LOGGER,"enviarMsjNuevoPersonaje: Error al enviar header NUEVO_PERSONAJE %s \n\n", hiloPxN->personaje.nivel);
	}

	return ret;
}


int enviarInfoPersonaje(int sock, t_hilo_personaje *hiloPxN) {
	header_t header;
	t_personaje yo = hiloPxN->personaje;

	log_debug(LOGGER, "enviarInfoPersonaje: Envio mensaje con info del personaje");
	log_debug(LOGGER, "Datos: (%s, %d, %c)",  yo.nombre, yo.id, yo.id);

	initHeader(&header);
	header.tipo = CONECTAR_NIVEL;
	header.largo_mensaje = sizeof(t_personaje);

	log_debug(LOGGER, "enviarInfoPersonaje: Envio header CONECTAR_NIVEL (size: %d)", sizeof(header_t));
	if (enviar_header(sock, &header) != EXITO)
	{
		log_error(LOGGER,"enviarInfoPersonaje: Error al enviar header CONECTAR_NIVEL\n\n");
		return WARNING;
	}

	hiloPxN->estado = CONECTAR_NIVEL;
	log_debug(LOGGER, "enviarInfoPersonaje: Envio t_personaje %d (%s, %c, %d, %d, %d, %s)", sizeof(t_personaje), yo.nombre, yo.id, yo.posActual.x, yo.posActual.y, yo.fd, yo.nivel);
	if (enviar_personaje(sock, &yo) != EXITO)
	{
		log_error(LOGGER,"enviarInfoPersonaje: Error al enviar informacion del personaje\n\n");
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


	return ret;
}

int enviarMsjMuertePersonajePlan ( int sock, t_hilo_personaje *hiloPxN ) {
	int ret;
	header_t header;
	t_personaje yo;
	yo = hiloPxN->personaje;

	initHeader(&header);
	header.tipo = MUERTE_PERSONAJE;
	header.largo_mensaje = 0;

	log_debug(LOGGER,"enviarMsjMuertePersonajePlan: MUERTE_PERSONAJE (%s de %s) \n", hiloPxN->personaje.nombre, hiloPxN->personaje.nivel);

	 if ((ret = enviar_header(sock, &header)) != EXITO){
		 log_error(LOGGER,"enviarMsjMuertePersonajePlan: ERROR al enviar MUERTE_PERSONAJE (%s de %s) \n", hiloPxN->personaje.nombre, hiloPxN->personaje.nivel);
		 return WARNING;
	 }

	if (enviar_personaje(sock, &yo) != EXITO)
	{
		log_error(LOGGER,"enviarMsjMuertePersonajePlan: Error al enviar t_personaje de MUERTE_PERSONAJE\n\n");
		return WARNING;
	}

	return ret;
}

int enviarMsjMovimientoRealizado (int sock, t_hilo_personaje *hiloPxN) {
	int ret;
	header_t header;
	t_personaje yo;

	yo = hiloPxN->personaje;

	initHeader(&header);
	header.tipo = MOVIMIENTO_REALIZADO;
	header.largo_mensaje = sizeof(t_personaje);

	log_debug(LOGGER, "enviarMsjMovimientoRealizado: Envio header_t MOVIMIENTO_REALIZADO (size:%d)", sizeof(header_t));
	if ( (ret = enviar_header(sock, &header)) != EXITO)
	{
		log_error(LOGGER,"enviarSolicitudRecurso: Error al enviar header MOVIMIENTO_REALIZADO\n\n");
		return WARNING;
	}

	log_debug(LOGGER, "enviarMsjMovimientoRealizado: Envio t_personaje size: %d (%s, %c, pos(%d, %d), fd: %d, nivel: %s, recurso: %c)", sizeof(t_personaje), yo.nombre, yo.id, yo.posActual.x, yo.posActual.y, yo.fd, yo.nivel, yo.recurso);
	if ((ret = enviar_personaje(sock, &yo)) != EXITO)
	{
		log_error(LOGGER,"enviarMsjMovimientoRealizado: Error al enviar t_personaje de MOVIMIENTO_REALIZADO\n\n");
		return WARNING;
	}


	return ret;
}

int realizarMovimiento(int sock, t_proximoObjetivo *proximoObjetivo, t_hilo_personaje *hiloPxN) {
	//header_t header;
	//int ret;
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

	log_debug(LOGGER, "Informar MOVIMIENTO_REALIZADO (%d, %d) al %s", hiloPxN->personaje.posActual.x, hiloPxN->personaje.posActual.y, hiloPxN->personaje.nivel);

	// Informar movimiento realizado

//	initHeader(&header);
//	header.tipo = MOVIMIENTO_REALIZADO;
//	header.largo_mensaje = sizeof(t_personaje);
//
//	if (enviar_header(sock, &header) != EXITO){
//		log_error(LOGGER, "Error al enviar header MOVIMIENTO_REALIZADO");
//	}
//
//	ret = enviar_personaje(sock, &hiloPxN->personaje);
//
//	return ret;
	return EXITO;
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
		enviarMsjMovimientoRealizado(sock, hiloPxN);
	}

	return EXITO;
}

int gestionarRecursoConcedido (int sock, t_proximoObjetivo *proximoObjetivo, t_hilo_personaje *hiloPxN, int *fin) {

	hiloPxN->objetivosConseguidos++;
	hiloPxN->estado = RECURSO_CONCEDIDO;
	log_info(LOGGER, "gestionarRecursoConcedido. objetivosConseguidos: %d/%d", hiloPxN->objetivosConseguidos,hiloPxN->objetivos.totalObjetivos );

	//	Cuando el recurso sea asignado, el hilo analizará si necesita otro recurso y
	//	volverá al punto 3) (esperar TURNO_CONCEDIDO), o, si ya cumplió sus objetivos del Nivel
	// Notificar a su Planificador que completó los objetivos de ese nivel y desconectarse.
	if(hiloPxN->objetivosConseguidos == hiloPxN->objetivos.totalObjetivos){

		// TODO Se completo el nivel
		log_info(LOGGER, "\n\nCOMPLETE EL PLAN DEL %s !!!\n", hiloPxN->personaje.nivel);
		hiloPxN->estado = PLAN_NIVEL_FINALIZADO;
		enviarMsjPlanDeNivelFinalizado(sock, hiloPxN);
		*fin = true;

	} else if (hiloPxN->objetivosConseguidos < hiloPxN->objetivos.totalObjetivos) {

		// Todavia quedan recursos por conseguir.
		setearProximoObjetivo(proximoObjetivo, hiloPxN);

//		proximoObjetivo->simbolo = hiloPxN->objetivos.objetivos[hiloPxN->objetivosConseguidos];
//		proximoObjetivo->posicion.x = 0;
//		proximoObjetivo->posicion.y = 0;

	}

	return EXITO;
}


/*
 * En caso de tener vidas disponibles, el Personaje se descontará una vida, volverá a conectarse
 * al hilo Orquestador y le notificará su intención de iniciar nuevamente el Nivel en que estaba jugando.
 */
int muertePersonaje(MOTIVO_MUERTE motivo, t_proximoObjetivo *proximoObjetivo, t_hilo_personaje *hiloPxN, int *sock, fd_set *master, int *maxDesc) {
	int32_t vidas;

	//  informar el motivo de muerte por pantalla
	switch(motivo){
		case MUERTE_POR_ENEMIGO:
			log_info(LOGGER, "\n\n MUERTE POR ENEMIGO!! en %s \n", hiloPxN->personaje.nivel);
			hiloPxN->estado = MUERTE_PERSONAJE_XENEMIGO;
			break;
		case MUERTE_POR_INTERBLOQUEO:
			log_info(LOGGER, "\n\n MUERTE POR INTERBLOQUEO!! en %s \n", hiloPxN->personaje.nivel);
			hiloPxN->estado = MUERTE_PERSONAJE_XRECOVERY;
			break;
//		case MUERTE_POR_SIGKILL:
//			break;
//		case MUERTE_POR_SIGTERM:
//			break;
		default: log_info(LOGGER, "\n\n Motivo de muerte no valido!! en %s\n", hiloPxN->personaje.nivel); break;
	}

	imprimirVidasyReintentos();

	//TODO hacer lo necesario antes de morir
	vidas = decrementarVida();

	if (vidas >= 0) {
		// si todavia me quedan vidas, llamar funciones para reiniciar SOLAMENTE el nivel!

		// reinicio posicion actual a (0,0) y recurso a '-'
//		hiloPxN->personaje.posActual.x = 0;
//		hiloPxN->personaje.posActual.y = 0;
//		hiloPxN->personaje.recurso = '-';
		reiniciarPersonje(&(hiloPxN->personaje));


		// reinicio contador de objetivos conseguidos a cero
		hiloPxN->objetivosConseguidos = 0;

		// seteo proximo objetivo
		setearProximoObjetivo(proximoObjetivo, hiloPxN);

		// Me desconecto de plataforma
		close(*sock);
		quitar_descriptor(*sock, master, maxDesc);

		// Me vuelvo a conectar a plataforma para que me reciba el orquestador
		log_info(LOGGER,"************** CONECTANDOSE A PLATAFORMA ***************\n");
		conectar(personaje.ip_orquestador, personaje.puerto_orquestador, sock);
		agregar_descriptor(*sock, master, maxDesc);

		// y envio mensaje de nuevoPersonaje.
		if (enviarMsjNuevoPersonaje(*sock, hiloPxN) != EXITO) {
			return true;
		}

	} else {

		log_info(LOGGER, "\n\n\n YA NO ME QUEDAN VIDAS PARA CONTINUAR!!!! - personaje %s del %s \n", hiloPxN->personaje.nombre, hiloPxN->personaje.nivel);

		// TODO llamar a funcion de reinicio completo de juego
		reinicioNivelCompleto();

		return true;
	}

	return false;

}





