/*
 * fileSystem.c
 *
 *  Created on: 18/10/2013
 *      Author: elyzabeth
 */

#include "fileSystem.h"


void levantarHeader(int fd, char *HDR ) {

	HDR = (char*)mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, fd, 0);

	//LEO header
	memcpy(&HEADER, HDR, sizeof(GHeader));

	printf("--- HEADER: ----\n");
	printf("grasa: %s:\n", HEADER.grasa);
	printf("version: %d:\n", HEADER.version);
	printf("blk_bitmap: %d:\n", HEADER.blk_bitmap);
	printf("size_bitmap: %d:\n", HEADER.size_bitmap);

	munmap(HDR, 4096);

}

void mapearBitMap (int fd, char *TN) {
	BITMAP = mmap(NULL, BLKLEN*HEADER.size_bitmap, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, fd, BLKLEN * GHEADERBLOCKS);
}

void mapearTablaNodos(int fd) {
	int i;
	FNodo = mmap(NULL, BLKLEN*GFILEBYTABLE, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, fd, BLKLEN*(GHEADERBLOCKS+HEADER.size_bitmap));
	for (i=0; i < GFILEBYTABLE; i++) {
		NODOS[i] = (GFile*)(FNodo + i);
	}

}

void mapearDatos (int fd) {
	DATOS = mmap(NULL, TAMANIODISCO, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, fd, 0);
}

void leerArchivo(int inodo, char *buf) {
	int i=0, cantAcopiar= BLKLEN;
	uint32_t tamanioArchivo = NODOS[inodo]->file_size;
	char*aux = buf;

	while (i < 1024 && NODOS[inodo]->blk_indirect[i] != 0 ) {
		cantAcopiar = tamanioArchivo<BLKLEN?tamanioArchivo:BLKLEN;
		memcpy(aux, DATOS+(NODOS[inodo]->blk_indirect[i] * BLKLEN), cantAcopiar );
		aux += cantAcopiar;
		tamanioArchivo-=BLKLEN;
	}
}

/*
 * @DESC
 *  Esta función va a ser llamada cuando a la biblioteca de FUSE le llege un pedido
 * para obtener la metadata de un archivo/directorio. Esto puede ser tamaño, tipo,
 * permisos, dueño, etc ...
 *
 * @PARAMETROS
 * 		path - El path es relativo al punto de montaje y es la forma mediante la cual debemos
 * 		       encontrar el archivo o directorio que nos solicitan
 * 		stbuf - Esta esta estructura es la que debemos completar
 *
 * 	@RETURN
 * 		O archivo/directorio fue encontrado. -ENOENT archivo/directorio no encontrado
 */
static int grasa_getattr(const char *path, struct stat *stbuf) {
	int res = 0;
	int i=0, encontrado=-1;
	struct timespec ctime;
	struct timespec mtime;
	char *subpath = strrchr(path, '/');

	memset(stbuf, 0, sizeof(struct stat));

	//Si path es igual a "/" nos estan pidiendo los atributos del punto de montaje
	if (strcmp(path, "/") == 0) {
		encontrado = i;
		stbuf->st_mode = 0755|S_IFDIR;
		stbuf->st_nlink = 2;

	} else {

		for(i=0; i < 1024; i++) {

			if (strcmp(subpath+1, NODOS[i]->fname) == 0 && NODOS[i]->state != 0) {

				encontrado = i;
				ctime.tv_nsec = NODOS[i]->c_date;
				mtime.tv_nsec = NODOS[i]->m_date;

				stbuf->st_ctim = ctime;
				stbuf->st_mtim = mtime;
				stbuf->st_size = NODOS[i]->file_size;

				if (NODOS[i]->state == 2) {
					stbuf->st_mode = 0755|S_IFDIR;
					stbuf->st_nlink = 2;
					//stbuf->st_size = BLKLEN;

				} else if (NODOS[i]->state == 1) {
					stbuf->st_mode = S_IFREG | 0444;
					stbuf->st_nlink = 1;
				}
			}
		}
	}

	if (encontrado==-1)
		res = -ENOENT;

	return res;
}




/*
 * @DESC
 *  Esta función va a ser llamada cuando a la biblioteca de FUSE le llege un pedido
 * para obtener la lista de archivos o directorios que se encuentra dentro de un directorio
 *
 * @PARAMETROS
 * 		path - El path es relativo al punto de montaje y es la forma mediante la cual debemos
 * 		       encontrar el archivo o directorio que nos solicitan
 * 		buf - Este es un buffer donde se colocaran los nombres de los archivos y directorios
 * 		      que esten dentro del directorio indicado por el path
 * 		filler - Este es un puntero a una función, la cual sabe como guardar una cadena dentro
 * 		         del campo buf
 *
 * 	@RETURN
 * 		O directorio fue encontrado. -ENOENT directorio no encontrado
 */
static int grasa_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
	(void) offset;
	(void) fi;
	int i, encontrado=-1;
	struct stat *stbuf;
	uint32_t directorio = 0;
	char *subpath = strrchr(path, '/');

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

	encontrado =1;

	if (strcmp(path, "/")==0) {
		directorio = 0;
	} else {
		for(i=0; i < 1024; i++) {
			if (strcmp(subpath+1, NODOS[i]->fname) == 0 && NODOS[i]->state != 0) {
				encontrado = i;
				directorio = i+1;
			}
		}
	}

	if (encontrado==-1)
		return -ENOENT;

	for(i=0; i < 1024; i++) {
		if (NODOS[i]->parent_dir_block == directorio && NODOS[i]->state != 0) {
			stbuf = malloc(sizeof(struct stat));
			memset(stbuf, 0, sizeof(struct stat));
			if (NODOS[i]->state == 2) {
				stbuf->st_mode = 0755|S_IFDIR;
				stbuf->st_nlink = 2;
				stbuf->st_uid = 1001;
				stbuf->st_gid = 1001;
			} else if (NODOS[i]->state == 1) {
				stbuf->st_mode = 0444 | S_IFREG;
				stbuf->st_nlink = 1;
				stbuf->st_size = NODOS[i]->file_size;
				stbuf->st_uid = 1001;
				stbuf->st_gid = 1001;
			}

			filler(buf, NODOS[i]->fname, stbuf, 0);

		}
	}


	return 0;
}


static int grasa_mkdir (const char *path, mode_t mode) {
	return 0;
}

static int grasa_read (const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
	(void) offset;
	(void) fi;
    int encontrado, i;
    char *subpath = strrchr(path, '/');

    if (strcmp(path, "/")!=0) {
		for(i=0; i < 1024; i++) {
			if (strcmp(subpath+1, NODOS[i]->fname) == 0 && NODOS[i]->state == 1) {
				encontrado = i;
			}
		}
	}

    if (encontrado==-1)
    	return -ENOENT;

    leerArchivo(i, buf);

    return 0;
}

/*
 * Esta es la estructura principal de FUSE con la cual nosotros le decimos a
 * biblioteca que funciones tiene que invocar segun que se le pida a FUSE.
 * Como se observa la estructura contiene punteros a funciones.
 */

static struct fuse_operations grasa_oper = {

//		.readdir = hello_readdir,
//		.open = hello_open,
//		.read = hello_read,

		.getattr = grasa_getattr,
		.readdir = grasa_readdir,
		.read = grasa_read,
		.mkdir = grasa_mkdir
		//.destroy = grasa_destroy
};

int main (int argc, char**argv) {

	int fd = -1;
	char *HDR=NULL;

	if ((fd = open("./disk.bin", O_RDWR, 0)) == -1)
		err(1, "open");

	levantarHeader(fd, HDR);
	mapearTablaNodos(fd);
	mapearDatos(fd);


	//printf("FNodo: %s", FNodo->fname);

	puts("-----Tabla Nodos-------");
	int i;
	for(i=0; i < 1024; i++) {
		if (NODOS[i]->state)
			printf("%d) bloque: %d - nombre: %s - state: %d - padre: %d\n", i, i+1, NODOS[i]->fname, NODOS[i]->state, NODOS[i]->parent_dir_block);
	}

	 struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	// Esta es la funcion principal de FUSE, es la que se encarga
	// de realizar el montaje, comuniscarse con el kernel, delegar todo
	// en varios threads
	return fuse_main(args.argc, args.argv, &grasa_oper, NULL);
	// close(fd);
}
