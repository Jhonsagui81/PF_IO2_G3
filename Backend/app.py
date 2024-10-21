from flask import Flask, request, jsonify
from flask_cors import CORS
from datetime import datetime
import json

import redis

app = Flask(__name__)
CORS(app, resources={r"/*": {"origins": "*"}})

# Conectar a Redis
r = redis.Redis(host='localhost', port=6379, decode_responses=True)
r.set('externos', 0)
r.set('estudiantes', 0)
r.set('disponibles', 5)
#limipiando listas
# r.ltrim('entradas', -1, 0)
# r.ltrim('salidas', -1, 0)

@app.route('/update_parking', methods=['POST'])
def update_parking():
    data = request.json
    carnet = data.get('carnet')
    accion = data.get('accion')

    fecha_hora = datetime.now().strftime('%Y-%m-%d %H:%M:%S')

    #Es externo y esta entrando 
    if "ext" in carnet and "1" in accion:
        r.incr('externos') #aumento externos
        r.decr('disponibles') #decrementos los disponibles
        
        #Registro de entrada
        registro_e = {
            'carnet': carnet,
            'accion': "Entro al parqueo",
            'fecha_hora': fecha_hora
        }
        r.rpush('entradas', str(registro_e))

    #Es externo y esta saliendo
    elif "ext" in carnet and "0" in accion:
        r.decr('externos')#Decremento externos
        r.incr('disponibles')#aumento disponibles

        #Registro de entrada
        registro_e = {
            'carnet': carnet,
            'accion': "Salio del parqueo",
            'fecha_hora': fecha_hora
        }
        r.rpush('salidas', str(registro_e))

    #Es estudiante y esta entrando
    elif isinstance(int(carnet), int) and "1" in accion:
        r.incr('estudiantes') #aumento estudiantes
        r.decr('disponibles')

        #Registro de entrada
        registro_e = {
            'carnet': carnet,
            'accion': "Entro al parqueo",
            'fecha_hora': fecha_hora
        }
        r.rpush('entradas', str(registro_e))

    #Es estudiante y esta saliendo
    elif isinstance(int(carnet), int) and "0" in accion:
        r.decr('estudiantes') #decremento estudiantes
        r.incr('disponibles') #aumento disponibles

        #Registro de entrada
        registro_e = {
            'carnet': carnet,
            'accion': "Salio del parqueo",
            'fecha_hora': fecha_hora
        }
        r.rpush('salidas', str(registro_e))

    return jsonify({
        'estado': "Actualizacion Exitosa"
    })

@app.route('/status', methods=['GET'])
def status():
    disponibles = int(r.get('disponibles'))
    estudiantes = int(r.get('estudiantes') )
    externos = int(r.get('externos'))

    return jsonify({
        'disponible': disponibles,
        'estudiantes': estudiantes,
        'externos': externos
    })


@app.route('/entradas', methods=['GET'])
def entradas():
    registros_entrada = r.lrange('entradas', 0, -1)
    datos_deserializados = []

    for dato in registros_entrada:
        try:
            registro = eval(dato)  # Convertir la cadena JSON a diccionario
            datos_deserializados.append(registro)
        except json.JSONDecodeError as e:
            print(f"Error al decodificar: {e}")

    return jsonify(datos_deserializados)
   
@app.route('/salidas', methods=['GET'])
def salidas():
    registros_entrada = r.lrange('salidas', 0, -1)
    datos_deserializados = []

    for dato in registros_entrada:
        try:
            registro = eval(dato)  # Convertir la cadena JSON a diccionario
            datos_deserializados.append(registro)
        except json.JSONDecodeError as e:
            print(f"Error al decodificar: {e}")

    return jsonify(datos_deserializados)

@app.route('/reporte', methods=['GET'])
def reporte():
    registros_entrada = r.lrange('entradas', 0, -1)
    horas_entrada = []
    hoy = hoy = datetime.now().today().strftime("%Y-%m-%d") 

    for registro in registros_entrada:
        registro = eval(registro)
        
        # Convertir la fecha-hora a un objeto datetime
        hora_entrada = datetime.strptime(registro['fecha_hora'], '%Y-%m-%d %H:%M:%S')
        fecha = str(hora_entrada).split(' ')
        # print(str(fecha[0]))
        # print(str(hoy))
        if str(fecha[0]) == str(hoy):
                horas_entrada.append(hora_entrada)  
    
    tiempos_entre_llegadas = []
    for i in range(1, len(horas_entrada)):
        tiempo_entre_llegadas = (horas_entrada[i] - horas_entrada[i-1]).total_seconds()
        tiempos_entre_llegadas.append(tiempo_entre_llegadas/60) #transforma a horas

    if tiempos_entre_llegadas:
        promedio = sum(tiempos_entre_llegadas) / len(tiempos_entre_llegadas)
        lambda_value = 1 / promedio if promedio else None
        return jsonify(promedio)
    return None

if __name__ == '__main__':
    app.run(debug=True)
