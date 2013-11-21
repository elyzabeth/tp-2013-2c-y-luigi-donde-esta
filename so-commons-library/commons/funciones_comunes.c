#include "funciones_comunes.h"

int cambiar_nombre_proceso(char **argv,int argc,char *nombre) {
	int i,aux;

	if(*argv != NULL && argv != NULL && argc >= 0 && nombre != NULL)
	{
		i=0;
		while(argv[i] != NULL)
		{
			aux = strlen(argv[i]);
			memset(argv[i], 0, aux);
			i++;
		}
		strcpy(argv[0],nombre);
	}
	else
		return ERROR;
	return EXITO;
}

/**
 * @NAME: calcularDistancia
 * @DESC: Calcula la distancia entre 2 puntos
 * Recibe las coordenadas del punto y del objetivo
 */
int32_t calcularDistancia (int32_t posX, int32_t posY, int32_t objX, int32_t objY) {
	return abs((objX-posX)) + abs((objY-posY));
}

int32_t calcularDistanciaCoord (t_posicion pos1, t_posicion pos2) {
	 return calcularDistancia(pos1.x, pos1.y, pos2.x, pos2.y);
}

