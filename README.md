Mi personal version de GPSDO, armado con informacion capturada aqui y alla... 

Si alguien piensa que tiene algun derecho sobre parte de lo escrito, perfecto, puede ser.
Por ejemplo, partes de este trabajo se basa el lo programado por F2DC en su gpsdo, caso las rutinas de
medicion de frecuencia por interrupt, despues de buscar y ver que todos hacen lo mismo, use parte de ellas y
a el va el agradecimiento por hacer disponible su trabajo.

El resto de este fuente esta disponible para uso gratuito por radioaficionados, no asi para uso comercial
Si la idea es vender algo con todo o parte de este fuente, debe contactar primero a LU9DA
( lu9da (at) lu9da dot org) para obtener atuorizacion escrita. 

La libreria SI5351 es la de Etherkit!

El display chinesco usa la MicroLCD library
For more information, please visit https://github.com/stanleyhuangyc/MultiLCD/tree/master/MicroLCD
en la libreria, en microLCD.h, descomentar el #define MEMORY_SAVING sino no entra en el micro!

El encoder usa la libreria "encoder library" https://github.com/PaulStoffregen/Encoder

La plaquetita GPS lleva un contacto mas al led PPS, para pescar la salida de un pulso por segundo cuando detecta satelite

El SI5351 se pone mas estable con un pequeño disipador pegado arriba del cristal de 25 mHz.

Para cambiar entre menu, pulsar el boton del encoder por mas de 2 segundos, lo mismo para grabar la programacion de las
dos memorias.

Cuando editamos memorias, pulsando menos de un segundo el boton del encoder, mueve el cursor de cifra en cifra, y la cifra cambia
girando el encoder. Se puede salir, grabando, en cualquier momento.

la frecuencia minima "deberia" ser de 8 kHz y la maxima de 170 mHz, pero esta limitada a 99.999.999 mHz y las salidas por el buffer 
que le ponga cada uno. Es medio criminal usar directamente las salidas del SI5351.

Se deja constancia que NO se lastimo ningun circuito integrado ni componente pasivo en la realizacion de este proyecto.

Consultas por mail (lu9da (at) lu9da dot org)

//************************************************************************************************************

My personal version of GPSDO, built with information captured here and there...

If anyone thinks they have any rights to any of this, fine, that's fine.
For example, parts of this work are based on what F2DC programmed in his GPSDO, such as the interrupt frequency measurement routines. 
After searching and seeing that they all do the same thing, I used some of them and would like to thank him for making his work available.

The rest of this source is available for free use by radio amateurs, but not for commercial use.
If you plan to sell anything with all or part of this source, you must first contact LU9DA (lu9da@lu9da.org) to obtain written authorization.

The SI5351 library is from Etherkit!

The Chinese display uses the MicroLCD library.
For more information, please visit https://github.com/stanleyhuangyc/MultiLCD/tree/master/MicroLCD
In the library, in microLCD.h, uncomment the #define MEMORY_SAVING, otherwise it won't fit in the micro!

The encoder uses the "encoder library" https://github.com/PaulStoffregen/Encoder

The GPS board has an additional contact to the PPS LED, to capture the output of one pulse per second when it detects satellites.

The SI5351 is made more stable with a small heatsink glued above the 25 mHz crystal.

To switch between menus, press the encoder button for more than 2 seconds, and the same is true for saving the programming of both memories.

When editing memories, pressing the encoder button for less than a second moves the cursor from digit to digit, and the digit changes by turning the encoder. 
You can exit, saving, at any time.

The minimum frequency "should" be 8 kHz and the maximum 170 mHz, but it is limited to 99,999,999 mHz, and the outputs by the user selected buffer schematic (74hct14 or BC548/2n5906). 
It is somewhat criminal to use the SI5351 outputs directly.

It is noted that NO integrated circuit or passive component was damaged in the creation of this project.

Feel fre to contact LU9DA by mail (lu9da (at) lu9da dot org)
