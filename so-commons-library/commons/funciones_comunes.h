// funciones_comunes.h
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

#define ERROR		     0
#define EXITO		     1
#define WARNING          2

int cambiar_nombre_proceso(char **argv,int argc,char *nombre);
