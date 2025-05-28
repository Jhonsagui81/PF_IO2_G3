# PROYECTO FERIA EMI

Como parte de un curso de **Investigacion de Operaciones** participamos en una feria de proyectos, donde nuestro objetivo era implementar algunos de los modelos de cola en un prototipo para resolver un problema de la vida real. 


# Solucion

Con nuestro equipo pensamos en implementar el modelo de colas en la entrada del parqueo de nuestra Universidad. Utilizamos en especifico el modelo **M/M/1/K** ya que el parqueo de nuestra universidad tiene un numero K de parqueos disponibles.

## Prototipo
Realizamos una simulacion de la entrada del parqueo de la Universidad en una maqueta, pero agregando en la entrada una talanquera, la cual Actuaria como la **Entrada/Salida del Servidor**

Esta talanquera esta conformada por sensores y controladores. Los datos recolectados eran enviados a un backend escrito en python, mediante comunicacion Serial.

- **Finales de carrera**: Para detectar cuando un carro estuviera en la entrada o en la salida de la talanquera.
- **RFID RC522**: Al momento que el Final de carrera se activara se activaba un lector de tarjetas RFID, con esto los estudiantes podrian ingresar con su carnet universitario al parqueo sin pagar dinero. Una vez el carnet era identificado se guardaba la informacion en la base de datos, esto con la finalidad de poder mostrar en tiempo real la cantidad de personas dentro del parqueo en la Pagina web.
- **Ultrasonico**: En el caso que una persona externa a la Facultad deseara ingresar al parqueo debia presionar un boton identificado como **persona externa**. Al precionar este boton se activaba un sensor ultrasonico el cual tenia como finalidad verificar que el cliente depositara una moneda o billete como forma de pago. Una vez pagado se abria la Talanquera.
- **Infrarojo**: Una vez la persona lograba ingresar por la talanquera esta se bajaba hasta que el automovil pasara por el sensor infrarojo, evitando asi problemas como que el carro no paso a tiempo y la talanquera le cae encima.

Con esto lograbamos obtener la cantidad de parqueos disponibles, la cantidad de parqueos ocupados por estudiantes y por personas externas. Esto era sumamente util ya que hay personas que utilizan los parqueos de la universidad y luego se van a sus trabajos, esto lo hacen con el fin de no pagar parqueo en otro establecimiento. [mas detalles](./Frontend/README.md)

![alt text](<./img/img1.JPG>)
