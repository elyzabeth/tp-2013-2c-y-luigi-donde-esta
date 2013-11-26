rm -r nivel3
rm -r nivel4
cp -r nivel nivel3
cp -r nivel nivel4
cp nivel/nivel3.conf nivel3/nivel.conf
cp nivel/nivel4.conf nivel4/nivel.conf

rm -r personaje2 
cp -r personaje personaje2
cp personaje/personajeLuigi.conf personaje2/personaje.conf
