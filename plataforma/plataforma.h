
#ifndef plataforma_H_
#define plataforma_H_

#include <stdio.h>
#include <stdlib.h>
#include <curses.h>

#include "commons/comunicacion.h"
#include "config/configPlataforma.h"
#include "commons/log.h"
#include <commons/collections/list.h>

typedef struct config_plataforma_s
{
  //char id[LARGOID];
  char nombre [10];
  char cantidad_conexiones; /*igual a cantidad de personajes?????*/

}config_plataforma_t;


#endif /* plataforma_H_ */



