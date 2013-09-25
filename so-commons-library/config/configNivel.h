/*
 * configNivel.h
 *
 *  Created on: Sep 19, 2013
 *      Author: elizabeth
 */

#ifndef CONFIGNIVEL_H_
#define CONFIGNIVEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../commons/config.h"
#include "../commons/string.h"
#include "../commons/log.h"
#include "../commons/collections/dictionary.h"
#include "funciones.h"

#define PATH_CONFIG_NIVEL "./nivel.conf"
#define MAXCHARLEN 200
#define MAXCANTCAJAS 50 // Cantidad maxima de cajas de recursos por nivel

typedef struct {
	char NOMBRE[MAXCHARLEN+1]; // Nombre caja ej: Caja1
	char RECURSO[MAXCHARLEN+1]; // Nombre del recurso ej: Flores
	char SIMBOLO;
	int32_t INSTANCIAS;
	int32_t POSX;
	int32_t POSY;
} t_caja;


// DECLARACION DE FUNCIONES

void levantarArchivoConfiguracionNivel ();
void levantarCambiosArchivoConfiguracionNivel();
void destruirConfigNivel ();

char* configNivelNombre();
char* configNivelPlataforma();
char* configNivelPlataformaIp();
int32_t configNivelPlataformaPuerto();
int32_t configNivelTiempoChequeoDeadlock();
int32_t configNivelRecovery();
int32_t configNivelEnemigos();
int32_t configNivelSleepEnemigos();
char* configNivelAlgoritmo();
int32_t configNivelQuantum();
int32_t configNivelRetardo();
char* configNivelLogPath();
t_log_level configNivelLogNivel();
int32_t configNivelLogConsola();
t_caja* configNivelRecurso(char simbolo);
t_dictionary* configNivelRecursos();


#endif /* CONFIGNIVEL_H_ */
