/*
 * interbloqueo.c
 *
 *  Created on: Oct 11, 2013
 *      Author: elizabeth
 */

#include "funcionesNivel.h"
/*NODO DE RECURSOS ASIGNADOS*/
typedef struct {
	char cIdRecurso[2];
	int32_t iCantAsignada;
} t_RecursoAsignado;
/*
typedef struct {
	char cNombreRecurso[15];
	char cIdRecurso[2];
	int32_t iCantTotal;
	int32_t iPosX;
	int32_t iPosY;
	int32_t iCantDisponible;
} t_Recursos;
#define BUFFER 1024
//NODO DE LISTA DE PERSONAJES EN EL NIVEL//




typedef struct stPersonajesNivel {
	int iSocket;
	char cNombre[12];
	char cSimbolo[2];
	int16_t iPosX;
	int16_t iPosY;
	char cIdRecursoBloquea[2];
	t_list * lstRecursosAsignados;
} stPersonajesNivel;
*/
//NODO DE LISTA DE PERSONAJES QUE ABANDONARON EL NIVEL
typedef struct {
	char cSimbolo[2];
	t_list * lstRecursosAsignados;
}stPersonajesOut;


/*
int32_t matAsignacion[20][20];
int32_t matSolicitud[20][20];
int32_t vecDisponibles[20];
*/
int32_t totalRecursos;
int32_t totalPersonajes;

// Prototipos de funciones del hilo
int32_t detectarDeadlock();
t_personaje* recovery();

t_caja buscaRecursoXIdRecursoDD(listaDeRecursosTrabajados,personaje->cIdRecursoBloquea);


void* interbloqueo(t_hiloInterbloqueo *hiloInterbloqueoo) {
	header_t header;
	fd_set master;
	fd_set read_fds;

	int max_desc = 0;
	int i, ret;
	struct timeval timeout;
	int fin = false;

	int32_t TiempoChequeoDeadlock;
	int32_t RecoveryOn;
	int32_t hayDeadLock;

	log_info(LOGGER, "HILO DE DETECCION DE INTERBLOQUEO: Iniciado.");

	FD_ZERO(&master);
	FD_ZERO(&read_fds);

	// Agrego descriptor de comunicacion con plataforma por pipe
	agregar_descriptor(hiloInterbloqueo.fdPipe[0], &master, &max_desc);

	TiempoChequeoDeadlock = configNivelTiempoChequeoDeadlock();
	RecoveryOn = configNivelRecovery();
	hayDeadLock = 0;
	t_caja * recurso;
	t_RecursoAsignado * recursoAux;
	char sPersonajesInterbloq[BUFFER];
	while(!fin) {

		FD_ZERO (&read_fds);
		read_fds = master;
		timeout.tv_sec = 0; /// timeout en segundos
		timeout.tv_usec = TiempoChequeoDeadlock * 1000; //timeout en microsegundos

		if((ret = select(max_desc+1, &read_fds, NULL, NULL, &timeout )) == -1)
		{
			puts("\n\nINTERBLOQUEO: ERROR en select!!\n");

		} else if (ret == 0) {

			// Aca va la logica de interbloqueo
			log_info(LOGGER, "Incio deteccion de interbloqueo...");
				if ( listaPersonajesEnNivel == NULL)
						{
							log_debug(logger,"HILO DD: No hay personajes en el nivel. No hay DD");
						}
						else
						{
							int iFinalizados[list_size(listaPersonajesEnNivel)];
							int i;
							/* CARGO VECTOR FINALIZADOS*/
							for (i=0; i<list_size(listaPersonajesEnNivel);i++)
							{
								iFinalizados[i] = 0;
							}

							/* CREO LISTA RECURSOS TRABAJADOS*/
							t_list * listaDeRecursosTrabajados;
							listaDeRecursosTrabajados = list_create();
							for (i=0; i < dictionary_size(listaRecursos);i++)
							{
								recursoAux = (t_RecursoAsignado *) malloc(sizeof(t_RecursoAsignado));
								recurso = dictionary_get(listaRecursos,i);
								sprintf(recursoAux->cIdRecurso,"%s", recurso->SIMBOLO);
								recursoAux->iCantAsignada = recurso->INSTANCIAS;
								list_add(listaDeRecursosTrabajados,recursoAux);
							}

							/*LIMPIO BUFFER Y EJECUTO ALGORITMO*/
							memset(sPersonajesInterbloq,'\0',BUFFER);
							for (i=0; i<list_size(listaPersonajesEnNivel);i++)
							{
								t_personaje *personaje;
								t_personaje = list_get(listaPersonajesEnNivel,i);

								/*VERIFICO SI EL PERSONAJE ESTÁ BLOQUEADO POR UN RECURSO*/
								if (personaje->recurso != '0')
								{
									if (personaje->listaRecursosAsignados != NULL)
							//TODO			//no exite la lista de recursos asignados en el personaje

									{
										if (list_size(personaje->listaRecursosAsignados)>0)
										{
											iFinalizados[i] = 0;
										}
										else
										{
											iFinalizados[i] = 1;
										}
									}

									recurso = buscaRecursoXIdRecursoDD(listaDeRecursosTrabajados,personaje->cIdRecursoBloquea);
									if((iFinalizados[i] == 1) && (1 <= recurso->INSTANCIAS))
									{
										recurso->INSTANCIAS++;
										iFinalizados[i] = 1;
									}
									else
									{
										iFinalizados[i] = 0;
										hayDeadLock = 1;
										strcat(sPersonajesInterbloq, personaje->nombre);
										strcat(sPersonajesInterbloq,";");
									}
								}
							}

						if(hayDeadLock == 1)
							{
								if(RecoveryOn == 1)
								{
								enviarMsjPorPipe(hiloInterbloqueo.fdPipeI2N[1], MUERTE_PERSONAJE_XRECOVERY);

								int iTamanioMsj;
								tipo_header_ipc header;

								char** cPersonajes = string_split(sPersonajesInterbloq, ";");
								if (cPersonajes[1] == NULL)
									{
										log_info(logger,"HILO DD: El personaje %s se encuentra en estado de STARVATION",cPersonajes[0]);
									}
								else
									{
										log_info(logger,"HILO DD: Hay DEADLOCK entre los personajes:");
										int dd = 0;
										while (cPersonajes[dd] != NULL)
										{
											log_info(logger,"HILO DD:  ------------------------->   %s",cPersonajes[dd]);
											dd++;
										}

									}
								free(cPersonajes);
								log_trace(logger,"HILO DD: Buffer a enviar en PEDIDO_RECOVERY a ORQUESTADOR: %s", sPersonajesInterbloq);
								armaheaderipc(&header, PEDIDO_RECOVERY, strlen(sPersonajesInterbloq));
								iTamanioMsj = sizeof(header);
								if (sendall(iSockOrquestador, (char *)&header, &iTamanioMsj) != 0)
									{
										log_error(logger, "HILO DD: Error al enviar PEDIDO_RECOVERY header a ORQUESTADOR");
									}
								else
									{
										log_debug(logger, "HILO DD: Envio PEDIDO_RECOVERY header a ORQUESTADOR, tipo: %x", PEDIDO_RECOVERY);

										/*ENVIO MENSAJE*/
										if(sendall(iSockOrquestador, sPersonajesInterbloq, &header.payloadlength) != 0)
										{
											log_error(logger, "HILO DD: Error al enviar PEDIDO_RECOVERY mensaje a ORQUESTADOR");
										}
										log_debug(logger, "HILO DD: Envio PEDIDO_RECOVERY mensaje a ORQUESTADOR");
									}
								}
							else
								{
									log_info(logger, "HILO DD: Hay DEADLOCK.Sin RECOVERY");
								}
							}
						else
							{
								log_info(logger, "HILO DD: No hay DEADLOCK");
							}
							/*VACIO LA LISTA PARA EL PROXIMO CHEQUEO*/
							borraListaDeadLock(listaDeRecursosTrabajados);
						}
						//pthread_mutex_unlock(&semMutex);
					}

			else {


			for(i = 0; i <= max_desc; i++)
			{
				if (FD_ISSET(i, &read_fds) && (i == hiloInterbloqueo.fdPipe[0]) ) {
					log_info(LOGGER, "INTERBLOQUEO: Recibo mensaje desde Nivel por Pipe");
					read (hiloInterbloqueo.fdPipe[0], &header, sizeof(header_t));

					log_debug(LOGGER, "INTERBLOQUEO: mensaje recibido '%d'", header.tipo);

					if (header.tipo == FINALIZAR) {
						log_debug(LOGGER, "INTERBLOQUEO: '%d' ES FINALIZAR", header.tipo);
						fin = true;
						//FD_CLR(hiloInterbloqueo.fdPipe[0], &master);
						quitar_descriptor(hiloInterbloqueo.fdPipe[0], &master, &max_desc);
						break;
					}
				}
			}
		}


	}

	log_info(LOGGER, "FINALIZANDO HILO INTERBLOQUEO...\n");

	pthread_exit(NULL);

}










//	int32_t T[20];
//
//	t_list *listaPJBloqueados = clonarListaPersonajesBloqueados();
//	t_dictionary *recursosxPJ = clonarListaRecursosxPersonaje();
//	t_dictionary *dicRecursos;

	// TODO agregar algoritmo de deteccion de interbloqueo
	// Lleno las matrices y los vectores necesarios.

	// 1) Se marca cada proceso que tenga una fila de la matriz de Asignacion completamente a cero
	// 2) Se inicia un vector temporal T asignandole el vector de disponibles
	// 3) Se busca un indice i tal que el proceso i no este marcado actualmente y la fila i-esima de S(olicitud) sea menor o igual a T (disponibles).
	//    Es decir, Se ejecuta Tk = Tk + Aik, para 1 <= k <= m. A continuacion se vuelve al 3 paso.



//t_personaje* recovery() {
	//log_info(LOGGER, "Incio proceso de recovery deadlock...");
//	t_personaje *personaje = NULL;

	// TODO agregar logica de recovery
	// 1- Seleccionar la victima (es la primera que entro al nivel)
	// 2- Mover al personaje seleccionado de los listados (deberia estar en bloqueados solamente) y agregarlo al listado MuerteXrecovery.
	// 3- Informar al nivel que hay un personaje muerto (el nivel debe encargarse de informar al personaje correspondiente).
