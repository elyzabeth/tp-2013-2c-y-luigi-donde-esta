/*
 * funciones_comunes.h
 */

#ifndef FUNCIONES_COMUNES_H_
#define FUNCIONES_COMUNES_H_

#include <termio.h>
#include <sys/ioctl.h>
#include <math.h>
#include <ctype.h>
#include <float.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <malloc.h>
#include <stdarg.h>

#include "../tads/tad_posicion.h"

#define ERROR		     0
#define EXITO		     1
#define WARNING          2

int cambiar_nombre_proceso(char **argv,int argc,char *nombre);
int32_t calcularDistancia (int32_t posX, int32_t posY, int32_t objX, int32_t objY);
int32_t calcularDistanciaCoord (t_posicion pos1, t_posicion pos2);

#endif /* FUNCIONES_COMUNES_H_ */
