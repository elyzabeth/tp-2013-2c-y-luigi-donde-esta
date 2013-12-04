/*
 * orquestador.c
 *
 *  Created on: 06/10/2013
 *      Author: elyzabeth
 */

#include "plataforma.h"

int enviarMsjPersonajeConectado (int fd);
int enviarMsjNivelConectado (int fd);
void nuevoPersonaje(int fdPersonaje, fd_set *master, int *max_desc);
void nuevoNivel(int fdNivel, header_t header, fd_set *master);
void recibirPlanNivelesConcluido(header_t *header, int *fin);


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

					initHeader(&header);
					recibir_header(nuevo_sock, &header, &master, &se_desconecto);

					switch (header.tipo)
					{
						case NUEVO_PERSONAJE:
							log_info(LOGGER, "ORQUESTADOR: NUEVO PERSONAJE");
							nuevoPersonaje(nuevo_sock, &master, &max_desc);
							break;

						case NUEVO_NIVEL:
							log_info(LOGGER, "ORQUESTADOR: NUEVO NIVEL");
							nuevoNivel(nuevo_sock, header, &master);
							//agregar_descriptor(nuevo_sock, &master, &max_desc);
							break;

						case PLAN_NIVELES_CONCLUIDO:
							log_info(LOGGER, "ORQUESTADOR: PLAN_NIVELES_CONCLUIDO");
							recibirPlanNivelesConcluido(&header, &fin);
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


/**
 * @NAME: enviarMsjPersonajeConectado
 * @DESC: Envia el mensaje PERSONAJE_CONECTADO al personaje.
 * recibe el file descriptor del personaje
 */
int enviarMsjPersonajeConectado (int fd) {
	int ret;
	header_t header;

	initHeader(&header);
	header.tipo=PERSONAJE_CONECTADO;
	header.largo_mensaje=0;

	log_info(LOGGER, "enviarMsjPersonajeConectado: Envio mensaje de personaje conectado (fd: %d)...", fd);
	ret = enviar_header(fd, &header);

	return ret;
}


/**
 * @NAME: enviarMsjNivelInexistente
 * @DESC: Envia el mensaje NIVEL_INEXISTENTE al personaje.
 * recibe el file descriptor del personaje
 */
int enviarMsjNivelInexistente (int fd) {
	int ret;
	header_t header;

	initHeader(&header);
	header.tipo=NIVEL_INEXISTENTE;
	header.largo_mensaje=0;

	log_info(LOGGER, "enviarMsjPersonajeConectado: Envio mensaje NIVEL_INEXISTENTE al personaje (fd: %d)...", fd);
	ret = enviar_header(fd, &header);

	return ret;
}

/**
 * @NAME: enviarMsjNivelConectado
 * @DESC: Envia el mensaje NIVEL_CONECTADO al nivel.
 * recibe el file descriptor del nivel
 */
int enviarMsjNivelConectado (int fd) {
	int ret;
	header_t header;

	initHeader(&header);
	header.tipo=NIVEL_CONECTADO;
	header.largo_mensaje=0;

	log_info(LOGGER, "Envio mensaje de nivel conectado al nivel (fd: %d)...", fd);

	ret = enviar_header(fd, &header);

	return ret;
}


/**
 * @NAME: nuevoPersonaje
 * @DESC: Cuando llega un personaje nuevo al orquestador, se espera recibir el nivel que solicita jugar.
 * Si existe el nivel crea la estructura para el personje, lo agrega a la lista de personajes nuevos,
 * informa al planificador del nivel correspondiente y se delega la conexion.
 */
void nuevoPersonaje(int fdPersonaje, fd_set *master, int *max_desc) {
	t_planificador *planner;
	header_t header;
	t_personaje *personaje;
	int se_desconecto;

	/**Contesto Mensaje **/

	log_info(LOGGER, "nuevoPersonaje: Envio mensaje de PERSONAJE_CONECTADO al personaje...");
	if (enviarMsjPersonajeConectado(fdPersonaje) != EXITO)
	{
		log_error(LOGGER, "Error al enviar header PERSONAJE_CONECTADO\n\n");

	} else {

		// recibir informacion del personaje.
		// SI existe el nivel solicitado lo agrego si no, enviar mensaje de nivel inexistente??

		log_debug(LOGGER, "nuevoPersonaje: Espero header CONECTAR_NIVEL (tipo: %d)...", CONECTAR_NIVEL);
		recibir_header(fdPersonaje, &header, master, &se_desconecto);
		log_debug(LOGGER, "nuevoPersonaje: Recibo tipo: %d...", header.tipo);

		if (se_desconecto) {
			log_info(LOGGER, "nuevoPersonaje: El personaje se desconecto");
			quitar_descriptor(fdPersonaje, master, max_desc);
		}

		if (!se_desconecto && header.tipo == CONECTAR_NIVEL) {

			personaje = crearPersonajeVacio();

			log_debug(LOGGER, "nuevoPersonaje: Espero recibir estructura personaje (size:%d)...", header.largo_mensaje);
			recibir_personaje(fdPersonaje, personaje, master, &se_desconecto);

			log_debug(LOGGER, "nuevoPersonaje: Llego: %s, %c, %s", personaje->nombre, personaje->id, personaje->nivel);
			personaje->fd = fdPersonaje;
			// Cuando se conecta un personaje nuevo le asigno RD por default para SRDF
			personaje->rd = configPlatRemainingDistance();

			// Verifico si existe el nivel solicitado y su estado
			if ( existeNivel(personaje->nivel) && (obtenerEstadoNivel(personaje->nivel) == CORRIENDO)) {

				planner = obtenerNivel(personaje->nivel);

				log_info(LOGGER, "nuevoPersonaje: Agrego personaje '%s' ('%c') a lista de personajes Nuevos.", personaje->nombre, personaje->id);
				agregarPersonajeNuevo(personaje);
				plataforma.personajes_en_juego++;

				//Informo al planificador correspondiente
				log_debug(LOGGER, "nuevoPersonaje: Informo al planificador correspondiente.");
				enviarMsjAPlanificador(planner, NUEVO_PERSONAJE );

			} else {
				// Si el nivel solicitado no existe se informa al personaje.
				log_info(LOGGER, "nuevoPersonaje: El personaje '%s' ('%c') solicita Nivel inexistente (%s).", personaje->nombre, personaje->id, personaje->nivel);
				enviarMsjNivelInexistente(fdPersonaje);
			}
		} else {
			log_info(LOGGER, "nuevoPersonaje: Se esperaba mensaje CONECTAR_NIVEL se recibio: %d",header.tipo);
		}

	}

}


/**
 * @NAME: nuevoNivel
 * @DESC: Cuando llega un nivel nuevo al orquestador, se espera recibir informacion del nivel.
 * Si no existe el planificador del nivel se crea la estructura correspondiente, lo agrega al diccionario de niveles,
 * se lanza el hilo planificador para el nuevo nivel.
 */
void nuevoNivel(int fdNivel, header_t header, fd_set *master) {

	int se_desconecto;
	t_nivel nivel;
	t_planificador *planner;

	initNivel(&nivel);
	recibir_nivel(fdNivel, &nivel, master, &se_desconecto);

	nivel.fdSocket = fdNivel;
	planner = crearPlanificador(nivel);

	log_info(LOGGER, "ORQUESTADOR - nuevoNivel: Se conecto el Nivel %s\n", nivel.nombre);

	// PRIMERO QUITO NIVELES EN ESTADO FINALIZADO
	eliminarNivelesFinalizados();

	if (!existeNivel(nivel.nombre)) {

		// Agrego el nivel al diccionario de niveles
		agregarAListaNiveles(planner);

		// levantar hilo Planificador para el nivel
		log_info(LOGGER, "Levanto Hilo Planificador para Nivel '%s'", nivel.nombre);
		pthread_create(&(planner->tid), NULL, (void*)planificador, planner);

		/**Contesto Mensaje **/
		if (enviarMsjNivelConectado(fdNivel) != EXITO) {
			log_error(LOGGER, "Error al enviar header NIVEL_CONECTADO\n\n");
		}

	} else {
		log_error(LOGGER, "El Nivel '%s' YA EXISTE EN EL SISTEMA!", nivel.nombre);
		// TODO ENVIAR MENSAJE AL NIVEL
	}

}

/**
 * @NAME: recibirPlanNivelesConcluido
 * @DESC: Cuando un personaje informa que concluyo su plan de niveles
 * se lo debe mover a la lista de finalizados.
 * Si todos los personajes en juego finalizaron sus planes se debe lanzar el proceso Koopa.
 */
void recibirPlanNivelesConcluido(header_t *header, int *fin) {
	int espera = configPlatSleepKoopa();

	log_info(LOGGER, "\n\n\nORQUESTADOR recibirPlanNivelesConcluido espero %d segundos... \n\n", espera);
	sleep(espera);

	log_info(LOGGER, "\n\n\nORQUESTADOR: Zapatilla de goma, el que no finalizó, se embroma, punto y coma!!\n\n");
	sleep(2);

	imprimirListaPersonajesFinalizados ();

	// Si todos los personajes concluyeron sus planes
	// Lanzar el proceso Koopa.
	if ( list_size(listaPersonajesEnJuego)== 0 && list_size(listaPersonajesNuevos) == 0 ) {
		log_info(LOGGER, "\n\n\nTODOS LOS PERSONAJES CONCLUYERON SUS PLANES DE NIVELES...\n\nEJECUTO PROCESO KOOPAA!!!!!");
		// TODO lanzar proceso Koopa!

		// execv

	} else {

		log_info(LOGGER, "\n\nTodavía hay personajes en juego %d o nuevos: %d", list_size(listaPersonajesEnJuego), list_size(listaPersonajesNuevos));
	}
}
