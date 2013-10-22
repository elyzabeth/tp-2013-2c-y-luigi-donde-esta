/*
 * tad_caja.h
 *
 *  Created on: Oct 21, 2013
 *      Author: elyzabeth
 */

#ifndef TAD_CAJA_H_
#define TAD_CAJA_H_

#include <stdlib.h>
#include <string.h>

#include "../config/funciones.h"

#pragma pack(1)
typedef struct {
	char NOMBRE[MAXCHARLEN+1]; // Nombre caja ej: Caja1
	char RECURSO[MAXCHARLEN+1]; // Nombre del recurso ej: Flores
	char SIMBOLO;
	int32_t INSTANCIAS;
	int32_t POSX;
	int32_t POSY;
} t_caja;
#pragma pack(0)


t_caja* crearCaja();
void initCaja(t_caja *caja);
void destruirCaja(t_caja *caja);

#endif /* TAD_CAJA_H_ */
