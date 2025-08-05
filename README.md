En esteproyecto utiliza un sensor de temperatura conectado a una ESP8266 que mediante wifi se conecta a la API de teleegram
e informa cuando se sobrepasa cieto umbral de temperatura. También permite consultar en tiempo real a través de un identificador

Para poder utilizar el codigo hay que crear un bot desde el Bot father de telegram. Es muy simple solo hay que seguir los pasos.

Luego se cambian los token en el codigo de la ESP

Yo utilizo el Bot dentro de un grupo para que varias personas podamos consultar o recibir las alertas. Hay que poner tambien los identificadores del chat de dicho grupo para dirigir correctamente las solicitudes


CAMBIOS:

* Para evitar que cuando la temperatura esta muy cerca del umbral y este variando por ejemplo entre 29.9°C y 30°C envie muchas alertas. Al enviar la primer alerta se inicia un contador de 5 minutos
esto permite solo recibir una alerta en ese tiempo y ademas va aclarando que cantidad de alertas se van enviando desde que se supero la temperatura por primera vez

* Se añadio la opcion de que te envie todos los dias a modo de registro la temperatura en un horario definido por el usuario (Se aconseja las 12 del medio dia ya que es la hora de mayor temperatura del dia habitualmente)
