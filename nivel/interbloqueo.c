/*
 * interbloqueo.c
 *
 *  Created on: Oct 11, 2013
 *      Author: elizabeth
 */
#define BUFFER 1024
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
//int32_t detectarDeadlock();
void recovery(t_queue* colaPJInterbloqueados);
t_caja* buscaRecursoXIdRecursoDD(t_list* list,char IdRecurso);
t_vecRecursos* obtenerRecursoxPersonaje(t_personaje *personaje);

//t_caja buscaRecursoXIdRecursoDD(listaDeRecursosTrabajados,personaje->cIdRecursoBloquea);
//bool personajeConRecursoAsignado(t_pesonaje);

void* interbloqueo(t_hiloInterbloqueo *hiloInterbloqueo) {
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
	// TODO agregar_descriptor(hiloInterbloqueo.fdPipe[0], &master, &max_desc);

	TiempoChequeoDeadlock = configNivelTiempoChequeoDeadlock();
	RecoveryOn = configNivelRecovery();
	hayDeadLock = 0;
	t_caja * recurso;
	t_RecursoAsignado * recursoAux;
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
			//t_dictionary* listaRecursoXPersonaje = clonarListaRecursosxPersonaje();
			//t_list* listaPersonajesBloqueados = clonarListaPersonajesBloqueados();
			t_dictionary* listaRecursosClonados = configNivelRecursos();
			t_queue* colaPersonajesEnNivel = clonarListaPersonajesEnNivel();
			t_vecRecursos * recursosDelPersonaje;
			log_info(LOGGER, "Incio deteccion de interbloqueo...");
				if ( colaPersonajesEnNivel == NULL)
						{
							log_debug(LOGGER,"HILO DD: No hay personajes en el nivel. No hay DD");
						}
						else
						{
							int iFinalizados[queue_size(colaPersonajesEnNivel)];
							int i;
							t_queue * colaPersonajesInterBloqueados;
							t_queue * colaPersonajesInterBloqueadosAMatar;
							colaPersonajesInterBloqueados = queue_create();
							colaPersonajesInterBloqueadosAMatar = queue_create();
							/* CARGO VECTOR FINALIZADOS*/
							for (i=0; i<queue_size(colaPersonajesEnNivel);i++)
							{
								iFinalizados[i] = 0;
							}
							/* CREO LISTA RECURSOS TRABAJADOS*/
							// incialmente con todos los recursos disponibles para luego ir quitandolselos
							t_list * listaDeRecursosTrabajados;
							listaDeRecursosTrabajados = list_create();
							void creoListaRecursosTrabajados(char *key, t_caja *caja)
							{
								recursoAux = (t_RecursoAsignado *) malloc(sizeof(t_RecursoAsignado));
								recurso = caja;
								sprintf(recursoAux->cIdRecurso,"%c", recurso->SIMBOLO);
								recursoAux->iCantAsignada = recurso->INSTANCIAS;
								list_add(listaDeRecursosTrabajados,recursoAux);
							}
							dictionary_iterator(listaRecursosClonados, (void*)creoListaRecursosTrabajados);


							/*LIMPIO BUFFER Y EJECUTO ALGORITMO*/


							for (i=0; i<queue_size(colaPersonajesEnNivel);i++)
							{
								t_personaje *personaje;
								personaje = queue_pop(colaPersonajesEnNivel);
								/*VERIFICO SI EL PERSONAJE ESTÁ BLOQUEADO POR UN RECURSO*/
								if (personaje->recurso != '0')
								{
									recursosDelPersonaje = obtenerRecursoxPersonaje(personaje);
									if (recursosDelPersonaje->total == 0)
// busco los RecurssosXpersonajes que esta usando
									{
										if ((sizeof (recursosDelPersonaje) / sizeof (t_vecRecursos) )>0)
										{
											iFinalizados[i] = 0;
										}
										else
										{
											iFinalizados[i] = 1;//para cada pj que este bloqueado y con recursos asigandos
										}
									}
									recurso = buscaRecursoXIdRecursoDD(listaDeRecursosTrabajados,personaje->recurso);
									if((iFinalizados[i] == 1) && (1 <= recurso->INSTANCIAS))
									{
										recurso->INSTANCIAS++;
										iFinalizados[i] = 1;
									}
									else
									{
										iFinalizados[i] = 0;
										hayDeadLock = 1;
										queue_push(colaPersonajesInterBloqueados, personaje);
									}
								}
							}


						if(hayDeadLock == 1)
							{
								if(RecoveryOn == 1)
								{
								//TODO enviarMsjPorPipe(hiloInterbloqueo.fdPipeI2N[1], MUERTE_PERSONAJE_XRECOVERY);
									if (queue_size(colaPersonajesInterBloqueados) == 1)
									{
									log_info(LOGGER,"HILO DD: El personaje %s se encuentra en estado de STARVATION",colaPersonajesInterBloqueados);
									}
								else
									{
										log_info(LOGGER,"HILO DD: Hay DEADLOCK entre los personajes:");
										t_personaje * personaje;
										while (!queue_is_empty(colaPersonajesInterBloqueados))
										{
											personaje = queue_pop(colaPersonajesInterBloqueados);
											log_info(LOGGER,"HILO DD:  ------------------------->   %s",personaje->nombre);
											queue_push(colaPersonajesInterBloqueadosAMatar,personaje);
										}
									}
								//EJECUTAR EL RECOVERY DESDE NIVEL
								recovery(colaPersonajesInterBloqueadosAMatar);
								queue_clean(colaPersonajesInterBloqueados);

								queue_clean(colaPersonajesInterBloqueadosAMatar);
								}
							else
								{
									log_info(LOGGER, "HILO DD: Hay DEADLOCK.Sin RECOVERY");
								}
							}
						else
							{
								log_info(LOGGER, "HILO DD: No hay DEADLOCK");
							}
						//LIBERO LA LISTA Y LOS ELEMENTOS
						list_clean(listaDeRecursosTrabajados);
						}
					}

			else {


			for(i = 0; i <= max_desc; i++)
			{
				//TODO if (FD_ISSET(i, &read_fds) && (i == hiloInterbloqueo.fdPipe[0]) )
				{
					log_info(LOGGER, "INTERBLOQUEO: Recibo mensaje desde Nivel por Pipe");
				//TODO	read (hiloInterbloqueo.fdPipe[0], &header, sizeof(header_t));

					log_debug(LOGGER, "INTERBLOQUEO: mensaje recibido '%d'", header.tipo);

					if (header.tipo == FINALIZAR) {
						log_debug(LOGGER, "INTERBLOQUEO: '%d' ES FINALIZAR", header.tipo);
						fin = true;
						//FD_CLR(hiloInterbloqueo.fdPipe[0], &master);
				//TODO			quitar_descriptor(hiloInterbloqueo.fdPipe[0], &master, &max_desc);
						break;
					}
				}
			}
		}


	}

	log_info(LOGGER, "FINALIZANDO HILO INTERBLOQUEO...\n");

	pthread_exit(NULL);

}



t_caja* buscaRecursoXIdRecursoDD(t_list* list,char IdRecurso)
{
	bool esMismoRecursoAux(t_RecursoAsignado  *recurso)
	{
		return (recurso->cIdRecurso[0] == IdRecurso);
		//return (!strcmp(&recurso->SIMBOLO, &IdRecurso));
	}
	return list_find(list, (void*)esMismoRecursoAux);
}

void recovery(t_queue * colaPersonajesInterBloqueadosAMatar){
	t_personaje * personaje;
	personaje = queue_pop(colaPersonajesInterBloqueadosAMatar);
	quitarPersonajeBloqueadosNivel(personaje->id);
	pthread_mutex_lock (&mutexListaPersonajesMuertosxRecovery);
	queue_push(listaPersonajesMuertosxRecovery, personaje);
	pthread_mutex_unlock (&mutexListaPersonajesMuertosxRecovery);
	log_info(LOGGER," RECOVERY: Se mato el personaje--->   %s",personaje->nombre);
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
