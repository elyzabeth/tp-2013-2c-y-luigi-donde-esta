/*
 * tad_caja.c
 *
 *  Created on: Oct 21, 2013
 *      Author: elyzabeth
 */

#include "tad_caja.h"

t_caja* crearCaja() {
	return (t_caja*)calloc(1, sizeof(t_caja));
}

void initCaja(t_caja *caja) {
	memset(caja, '\0', sizeof(t_caja));
}

void destruirCaja(t_caja *caja) {
	free(caja);
}
