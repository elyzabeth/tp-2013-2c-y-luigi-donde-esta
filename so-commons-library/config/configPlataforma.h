/*
 * configPlataforma.h
 *
 *  Created on: Sep 19, 2013
 *      Author: elizabeth
 */

#ifndef CONFIGPLATAFORMA_H_
#define CONFIGPLATAFORMA_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../commons/config.h"
#include "../commons/log.h"

#include "funciones.h"


// DECLARACION DE FUNCIONES

void levantarArchivoConfiguracionPlataforma();
void destruirConfigPlataforma();

int32_t configPlatPuerto ();
const char* configPlatKoopa ();
const char* configPlatScript ();
const char* configPlatFileSystem ();
int32_t configPlatSleepKoopa ();
int32_t configPlatRemainingDistance ();
char* configPlatLogPath ();
t_log_level configPlatLogNivel ();
int32_t configPlatLogConsola ();

#endif /* CONFIGPLATAFORMA_H_ */
