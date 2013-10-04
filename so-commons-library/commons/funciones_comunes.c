#include "funciones_comunes.h"

int cambiar_nombre_proceso(char **argv,int argc,char *nombre)

{
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
