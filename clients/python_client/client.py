import socket
import threading
import telemetry_client as tc
import car as car_module
import re
import time
import sys

# ====== Configuración del cliente ======
SERVER_IP = "127.0.0.1"
SERVER_PORT = 2000  # Puerto por defecto

# Leer puerto desde argumentos de línea de comandos
if len(sys.argv) >= 2:
    try:
        SERVER_PORT = int(sys.argv[1])
        print(f"[CONFIG] Usando puerto {SERVER_PORT} desde argumentos")
    except ValueError:
        print(f"[ERROR] Puerto inválido: {sys.argv[1]}")
        print(f"Uso: python {sys.argv[0]} [puerto]")
        sys.exit(1)

client = None
authenticated = False
running = True
response = "" #respuestas de cliente

# ====== Funciones del protocolo PTT ======
def create_message(action, data):
    """Crea un mensaje con el protocolo PTT"""
    action_padded = action.ljust(12)[:12]
    data_padded = data.ljust(150)[:150]
    return f"PTT{action_padded}{data_padded}END"

def parse_response(raw_data):
    """Parsea la respuesta del servidor"""
    try:
        # El servidor envía: PTT(3) + ACTION(12) + DATA(150) + END(3) = 168 bytes
        if b"PTT" in raw_data and b"END" in raw_data:
            ptt_idx = raw_data.index(b"PTT")
            end_idx = raw_data.index(b"END")
            
            # PTT = 3 bytes, ACTION = 12 bytes
            action_start = ptt_idx + 3
            action_end = action_start + 12
            data_start = action_end
            data_end = end_idx
            
            # Extraer action y data
            action_bytes = raw_data[action_start:action_end]
            action = action_bytes.decode('utf-8', errors='ignore').strip()
            
            data_bytes = raw_data[data_start:data_end]
            data = data_bytes.decode('utf-8', errors='ignore').strip()
            
            return action, data
    except Exception as e:
        print(f"[ERROR] Error parseando: {e}")
    
    return None, None

def parse_telemetry(data):
    """Extrae los valores de telemetría del formato: speed=X;dir=Y;battery=Z;temp=W"""
    try:
        pattern = r'speed=([\d]+);dir=([\d]+);battery=([\d]+);temp=([\d.]+)'
        match = re.search(pattern, data)
        
        if match:
            speed = match.group(1)
            direction_deg = int(match.group(2))
            battery = match.group(3)
            temp = match.group(4)
            
            # Convertir dirección a cardinal
            direction = format_direction(direction_deg)
            
            return speed, temp, direction, battery
    except Exception as e:
        print(f"[ERROR] Error parseando telemetria: {e}")
    
    return None

def format_direction(degrees):
    """Convierte grados a dirección cardinal"""
    dirs = [
        (0, 22.5, "NORTE"), (22.5, 67.5, "NORESTE"),
        (67.5, 112.5, "ESTE"), (112.5, 157.5, "SURESTE"),
        (157.5, 202.5, "SUR"), (202.5, 247.5, "SUROESTE"),
        (247.5, 292.5, "OESTE"), (292.5, 337.5, "NOROESTE"),
        (337.5, 360, "NORTE")
    ]
    
    for min_deg, max_deg, name in dirs:
        if min_deg <= degrees < max_deg:
            return name
    return str(degrees)

# ====== Conexión y autenticación ======
def connect_and_auth(username, password):
    """Conecta al servidor y se autentica"""
    global client, authenticated
    
    try:
        client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        print(f"[CONNECT] Conectando a {SERVER_IP}:{SERVER_PORT}...")
        client.connect((SERVER_IP, SERVER_PORT))
        print("[CONNECT] Conectado al servidor")
        
        # Enviar LOGIN
        login_data = f"{username};{password}"
        message = create_message("LOGIN", login_data)
        print(f"[LOGIN] Autenticando como '{username}'...")
        client.sendall(message.encode('utf-8'))
        
        # Esperar respuesta
        response = client.recv(1024)
        action, data = parse_response(response)
        
        if action == "OK" and "AUTH_OK" in data:
            authenticated = True
            print(f"[LOGIN] Autenticacion exitosa")
            
            # Extraer role para mostrar
            if "role=ADMIN" in data:
                role = "ADMIN"
            elif "role=OBSERVER" in data:
                role = "OBSERVER"
            else:
                role = "UNKNOWN"
            
            print(f"[LOGIN] Rol: {role}")
            return True
        else:
            print(f"[LOGIN] Autenticacion fallida: {action} - {data}")
            return False
            
    except Exception as e:
        print(f"[ERROR] Error en conexion/autenticacion: {e}")
        import traceback
        traceback.print_exc()
        return False

# ====== Hilo de recepción de telemetría ======
def receive_thread():
    """Recibe y procesa datos de telemetría del servidor"""
    global running
    global response
    
    while running and authenticated:
        try:
            data = client.recv(1024)
            if not data:
                print("[ERROR] Servidor desconectado")
                running = False
                break
            
            action, content = parse_response(data)
            
            # Solo procesar mensajes de telemetría (DATA)
            if action == "DATA":
                parsed = parse_telemetry(content)
                if parsed:
                    speed, temp, direction, battery = parsed
                    
                    # Actualizar el estado del carro
                    car_module.car.updateState(speed, direction, battery, temp)
                    
                    response = "OK"
                    #print(response)
                    
                    # Actualizar la interfaz gráfica
                    tc.root.after(0, lambda s=speed, t=temp, d=direction, b=battery: 
                                  tc.update_telemetry(s, t, d, b))  
        except Exception as e:
            if running:
                print(e)
                response = "ERROR"
                print(response)
            break
        
def response_thread():
    #Agregar vairables de respuesta
    #El hilo receptro modifica esta variable
    #Cada x segundos el hilo de respuesta verifica si hay una respuesta pendiente por enviar
    #Envía la respuesta, la limpia y se duerme
    global response
    try:
        while running:
            if not response:
                print("[CLIENTE] no hay respuestas a enviar")
                time.sleep(2)    
            else:
                print(f"[STATUS]", response)
                pttresponse = "PTT" + "STATUS" + "      " + response + "END"
                client.sendall(pttresponse.encode('utf-8'))
                response = ""
    finally:
        client.close() 

# ====== Inicio del cliente ======
def start_client():
    """Inicia el cliente con autenticación"""
    # Credenciales de observer
    username = "observer"
    password = "observer123"
    
    print("=" * 50)
    print("  NetDrive - Cliente Observador")
    print("=" * 50)
    
    # Conectar y autenticar
    if connect_and_auth(username, password):
        # Iniciar hilo de recepción
        recv_thread = threading.Thread(target=receive_thread, daemon=True)
        resp_thread = threading.Thread(target=response_thread, daemon=True)
        
        recv_thread.start()
        resp_thread.start()
        
        print("[INIT] Cliente iniciado correctamente")
        print("[INIT] Esperando datos de telemetria...")
    else:
        print("[ERROR] No se pudo iniciar el cliente")
        return

# ====== Lanzar cliente ======
start_client()

# ====== Hilo principal de la interfaz ======
try:
    tc.root.mainloop()
except KeyboardInterrupt:
    print("\n[EXIT] Cerrando cliente...")
finally:
    running = False
    if client:
        client.close()
