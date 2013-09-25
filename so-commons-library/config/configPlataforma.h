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

#define PATH_CONFIG_PLATAFORMA "./plataforma.conf"
#define MAXCHARLEN 200




// DECLARACION DE FUNCIONES

void levantarArchivoConfiguracionPlataforma();


int32_t configPlatPuerto ();
char* configPlatKoopa ();
char* configPlatScript ();
char* configPlatPathLog ();
char* configPlatNivelLog ();

#endif /* CONFIGPLATAFORMA_H_ */
