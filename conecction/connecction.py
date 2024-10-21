import serial
import time
import requests

# puerto linux: /dev/ttyACM0
# puerto windows: COM3
arduino_port = '/dev/ttyACM1'
baud_rate = 9600 # debe ser el mismo que en arduino
timeout = 5

# Inicializa la conexión serial
ser = serial.Serial(arduino_port, baud_rate, timeout=timeout)

# Espera un momento para que la conexión se estabilice
time.sleep(2)

print("Conexión serial establecida. Leyendo datos de Arduino...")

try:
    info1 = {}
    while True:
        # Lee una línea de datos del Arduino
        if ser.in_waiting > 0:  # Verifica si hay datos en el buffer
            data = ser.readline().decode('utf-8').rstrip()  # Lee y decodifica los datos
            print(f"Recibido: {data}")  # Muestra el dato recibido
            cadena = data.split(":")
            if "Carnet" in data:
                # Carnet: 123
                info1 = {}
                info1[cadena[0].lower()] = cadena[1]
                print(info1)
            elif "Accion" in data:
                # Accion: 0
                # 0 -> salida
                # 1 -> entrada
                info1[cadena[0].lower()] = int(cadena[1])
                print(info1)
                #post a la base de datos redis
                requests.post('http://127.0.0.1:5000/update_parking', json=info1)
                

        time.sleep(0.1)

except KeyboardInterrupt:
    print("Conexión finalizada.")
    ser.close()  # Cierra la conexión serial



