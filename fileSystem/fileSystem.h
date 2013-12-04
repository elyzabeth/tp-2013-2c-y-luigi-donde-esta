/*
 * fileSystem.h
 *
 *  Created on: 18/10/2013
 *      Author: elyzabeth
 */

#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#define FUSE_USE_VERSION 26

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <err.h>
#include <time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/time.h>


#include "commons/bitarray.h"
#include "commons/log.h"
#include "commons/string.h"



#define GFILEBYTABLE 1024
#define GFILEBYBLOCK 1
#define GFILENAMELENGTH 71
#define GHEADERBLOCKS 1
#define BLKINDIRECT 1000
#define BLKDIRECT 1024
#define BLKSIZE 4096 //Tamaño de bloque fijo en bytes


#define BORRADO 0
#define ARCHIVO 1
#define DIRECTORIO 2

typedef uint32_t ptrGBloque;

typedef struct grasa_header_t { // un bloque
	unsigned char grasa[5];
	uint32_t version;
	uint32_t blk_bitmap;
	uint32_t size_bitmap; // en bloques
	unsigned char padding[4073];
} GHeader;

typedef struct grasa_file_t { // un cuarto de bloque (256 bytes)
	uint8_t state; // 0: borrado, 1: archivo, 2: directorio
	//unsigned char fname[GFILENAMELENGTH];
	char fname[GFILENAMELENGTH];
	uint32_t parent_dir_block;
	uint32_t file_size;
	uint64_t c_date;
	uint64_t m_date;
	ptrGBloque blk_indirect[BLKINDIRECT];
} GFile;

GHeader HEADER;
char *BITMAP;
t_bitarray 	*bitvector;
GFile *FNodo;
GFile *NODOS[GFILEBYTABLE]; // un array de 1024 posiciones de estructuras de tipo GFile
char *DATOS;
ptrGBloque blk_direct[BLKDIRECT];

uint32_t TAMANIODISCO; // TODO tamaño del disco, lo debe tomar por parametro
uint32_t BITMAPBITS;
uint32_t ADMINBLKS;
uint32_t MAXBLK;

t_log* LOGGER;

pthread_mutex_t mutexGrasaWrite;
pthread_mutex_t mutexGrasaBitVector;
pthread_mutex_t mutexGrasaNodesTable;

#endif /* FILESYSTEM_H_ */
