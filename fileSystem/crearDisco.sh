# Bajo koopa
git clone https://github.com/sisoputnfrba/koopa-2c2013.git

# Bajo herramientas de grasa
git clone https://github.com/sisoputnfrba/grasa-tools.git

#Crear un disco de 100 MB
dd if=/dev/urandom of=disk.bin bs=1024 count=102400

#Formateo el disco
./grasa-tools/grasa-format disk.bin

