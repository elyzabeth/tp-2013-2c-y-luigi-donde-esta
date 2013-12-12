########################
# SCRIPT 1
# Ejecutar desde consola los siguientes comandos para intentar generar
# 600 archivos vacíos y 600 directorios. Debería soportar un máximo de 1024 nodos.
for i in {1..600}; do truncate -s 0 $i; done
for i in {601..1200}; do mkdir $i; done


########################
# SCRIPT 2
# Crear un disco de 100 MB y llenarlo con un archivo grande
dd if=/dev/urandom of=archivo.bin bs=1024 count=102400
# dd if=/dev/urandom of=archivo.bin bs=4096 count=24549
# dd if=/dev/urandom of=archivo.bin bs=4096 count=24550 

########################
# Massive File Creator

# 1) Instalar libssl-dev:
sudo apt-get install libssl-dev

# 2) Descargar Massive-File-Creator
curl -L https://github.com/sisoputnfrba/massive-file-creator/tarball/master -o mfc.tar.gz && tar xvfz mfc.tar.gz

# 3) Compilar
cd sisoputnfrba-massive-file-creator-3c89d14
gcc massive-file-creator.c -o mfc -lcrypto -lpthread

# 4) Ejecutar en el path del montaje del proceso FileSystem
# ./mfc 10 1024 /home/utnso/fsGrasa prefix_

