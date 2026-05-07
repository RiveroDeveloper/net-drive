import socket
import threading
import customtkinter as ctk
from tkinter import messagebox
import re
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
user_role = None
username = None

# ====== Funciones del protocolo ======
def send_message(action, data):
    """Envía un mensaje con el protocolo PTT"""
    # El servidor C busca strings "PTT" y "END" con strstr(), por lo que NO puede tener \x00 en medio
    # Formato: "PTT" + ACTION(12 bytes padding con espacios) + DATA(150 bytes padding con espacios) + "END"
    
    # Usar padding con ESPACIOS en lugar de bytes nulos
    action_padded = action.ljust(12)[:12]
    data_padded = data.ljust(150)[:150]
    
    # Construir mensaje como string (sin bytes nulos intermedios)
    message = f"PTT{action_padded}{data_padded}END"
    
    print(f"[SEND] Mensaje a enviar ({len(message)} chars):")
    print(f"   Header: {message[:3]}")
    print(f"   Action: '{message[3:15]}'")
    print(f"   Data (primeros 30): '{message[15:45]}'")
    print(f"   Footer: {message[-3:]}")
    
    client.sendall(message.encode('utf-8'))

def parse_response(raw_data):
    """Parsea la respuesta del servidor"""
    try:
        # El servidor envía: PTT(3) + ACTION(12) + DATA(150) + END(3) = 168 bytes
        # Buscar PTT y END en los bytes raw
        if b"PTT" in raw_data and b"END" in raw_data:
            ptt_idx = raw_data.index(b"PTT")
            end_idx = raw_data.index(b"END")
            
            # Calcular offsets correctos
            # PTT = 3 bytes (no 4!)
            action_start = ptt_idx + 3  # CORREGIDO: PTT son solo 3 bytes
            action_end = action_start + 12  # ACTION son 12 bytes
            data_start = action_end  # DATA empieza después del ACTION
            data_end = end_idx  # DATA termina donde empieza END
            
            # Extraer y decodificar action (12 bytes)
            action_bytes = raw_data[action_start:action_end]
            action = action_bytes.decode('utf-8', errors='ignore').strip().rstrip('\x00')
            
            # Extraer y decodificar data (hasta END)
            data_bytes = raw_data[data_start:data_end]
            response_data = data_bytes.decode('utf-8', errors='ignore').strip().rstrip('\x00')
            
            print(f"[PARSE] Parse correcto - action='{action}', data='{response_data[:50]}'")
            return action, response_data
    except Exception as e:
        print(f"[ERROR] Error parseando respuesta: {e}")
        import traceback
        traceback.print_exc()
    
    return None, None

# ====== Funciones de red ======
def login(user, password):
    global authenticated, user_role, username, client
    
    try:
        client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        print(f"[CONNECT] Conectando a {SERVER_IP}:{SERVER_PORT}...")
        client.connect((SERVER_IP, SERVER_PORT))
        print(f"[CONNECT] Conectado al servidor")
        
        # Enviar LOGIN
        login_data = f"{user};{password}"
        print(f"[LOGIN] Enviando LOGIN: user='{user}', pass='{password}'")
        send_message("LOGIN", login_data)
        
        # Esperar respuesta (mantener como bytes para parse_response)
        print(f"[LOGIN] Esperando respuesta del servidor...")
        response = client.recv(1024)
        print(f"[LOGIN] Respuesta recibida ({len(response)} bytes): {response[:100]}")
        
        action, data = parse_response(response)
        print(f"[LOGIN] Parseado - action='{action}', data='{data}'")
        
        if action == "OK" and "AUTH_OK" in data:
            authenticated = True
            username = user
            
            # Extraer role
            if "role=ADMIN" in data:
                user_role = "ADMIN"
            elif "role=OBSERVER" in data:
                user_role = "OBSERVER"
            
            print(f"[LOGIN] Login exitoso - role={user_role}")
            return True, user_role
        else:
            print(f"[LOGIN] Login fallido - action={action}, data={data}")
            return False, None
            
    except Exception as e:
        print(f"[ERROR] Error en login: {e}")
        import traceback
        traceback.print_exc()
        return False, None

def send_command(cmd):
    """Envía un comando al servidor"""
    try:
        send_message("COMMAND", cmd)
        
        # Esperar respuesta, puede recibir telemetría antes de la respuesta
        # Reintentar hasta 3 veces para encontrar la respuesta del comando
        for _ in range(3):
            response = client.recv(1024)
            action, data = parse_response(response)
            
            # Ignorar mensajes DATA (telemetría)
            if action == "DATA":
                continue
            
            if action == "OK":
                return True, "Comando ejecutado"
            elif action == "DENIED":
                return False, "Permiso denegado"
            else:
                return False, f"Respuesta inesperada: {action}"
        
        return False, "No se recibió respuesta del comando"
    except Exception as e:
        return False, str(e)

def list_users():
    """Lista los usuarios conectados"""
    try:
        print("[LIST] Solicitando lista de usuarios...")
        send_message("LIST", "")
        
        # Esperar respuesta, puede recibir telemetría antes
        # Reintentar hasta 3 veces para encontrar la respuesta de LIST
        for _ in range(3):
            response = client.recv(1024)
            action, data = parse_response(response)
            
            # Ignorar mensajes DATA (telemetría)
            if action == "DATA":
                continue
            
            print(f"[LIST] Respuesta LIST - action='{action}', data='{data}'")
            
            if action == "OK":
                # Parsear formato CSV: "user1:ADMIN:192.168.1.1,user2:OBSERVER:192.168.1.2"
                users = []
                
                # Dividir por comas
                user_entries = data.split(',')
                
                for entry in user_entries:
                    entry = entry.strip()
                    if not entry or 'NONE' in entry:
                        continue
                    
                    # Dividir por dos puntos
                    parts = entry.split(':')
                    if len(parts) >= 3:
                        users.append({
                            'user': parts[0],
                            'role': parts[1],
                            'ip': parts[2],
                            'port': 'N/A'  # El nuevo formato no incluye puerto
                        })
                        print(f"  [USER] Usuario: {parts[0]} - {parts[1]} - {parts[2]}")
                
                print(f"[LIST] Total usuarios: {len(users)}")
                return users
            else:
                # Recibió una respuesta pero no es OK
                print(f"[LIST] Error obteniendo lista - action='{action}'")
        
        # Si llegó aquí, no recibió respuesta válida después de 3 intentos
        print("[LIST] No se recibió respuesta de LIST")
        return []
    except Exception as e:
        print(f"[ERROR] Error listando usuarios: {e}")
        import traceback
        traceback.print_exc()
        return []

# ====== Interfaz gráfica ======
class AdminPanel:
    def __init__(self):
        self.root = ctk.CTk()
        self.root.title("NetDrive - Panel de Administrador")
        self.root.geometry("700x600")
        self.root.resizable(False, False)
        
        ctk.set_appearance_mode("dark")
        ctk.set_default_color_theme("blue")
        
        self.show_login()
    
    def show_login(self):
        """Pantalla de login"""
        login_frame = ctk.CTkFrame(self.root)
        login_frame.pack(fill="both", expand=True, padx=50, pady=100)
        
        title = ctk.CTkLabel(login_frame, text="NetDrive Admin", font=("Verdana", 24, "bold"))
        title.pack(pady=20)
        
        ctk.CTkLabel(login_frame, text="Usuario:", font=("Verdana", 12)).pack(pady=5)
        self.username_entry = ctk.CTkEntry(login_frame, width=300)
        self.username_entry.pack(pady=5)
        self.username_entry.insert(0, "admin")
        
        ctk.CTkLabel(login_frame, text="Contraseña:", font=("Verdana", 12)).pack(pady=5)
        self.password_entry = ctk.CTkEntry(login_frame, width=300, show="*")
        self.password_entry.pack(pady=5)
        self.password_entry.insert(0, "admin123")
        
        login_btn = ctk.CTkButton(login_frame, text="Iniciar Sesión", command=self.do_login, width=300)
        login_btn.pack(pady=20)
        
        info_label = ctk.CTkLabel(login_frame, text="Usuarios: admin/admin123, observer/observer123", 
                                   font=("Verdana", 10), text_color="gray")
        info_label.pack(pady=10)
    
    def do_login(self):
        user = self.username_entry.get()
        password = self.password_entry.get()
        
        success, role = login(user, password)
        
        if success:
            if role != "ADMIN":
                messagebox.showerror("Error", "Solo administradores pueden acceder a este panel")
                return
            
            # Limpiar ventana y mostrar panel admin
            for widget in self.root.winfo_children():
                widget.destroy()
            
            self.show_admin_panel()
        else:
            messagebox.showerror("Error", "Credenciales inválidas")
    
    def show_admin_panel(self):
        """Panel principal de administrador"""
        # Header
        header = ctk.CTkFrame(self.root, fg_color="#2C2C2C")
        header.pack(fill="x", padx=0, pady=0)
        
        title = ctk.CTkLabel(header, text=f"Panel Admin - {username}", 
                             font=("Verdana", 20, "bold"))
        title.pack(pady=15)
        
        # Frame principal
        main_frame = ctk.CTkFrame(self.root)
        main_frame.pack(fill="both", expand=True, padx=20, pady=20)
        
        # Sección de comandos
        cmd_frame = ctk.CTkFrame(main_frame)
        cmd_frame.pack(fill="x", padx=10, pady=10)
        
        ctk.CTkLabel(cmd_frame, text="Control del Vehículo", 
                     font=("Verdana", 16, "bold")).pack(pady=10)
        
        buttons_grid = ctk.CTkFrame(cmd_frame)
        buttons_grid.pack(pady=10)
        
        commands = [
            ("Acelerar", "SPEED UP"),
            ("Frenar", "SLOW DOWN"),
            ("Girar Izquierda", "TURN LEFT"),
            ("Girar Derecha", "TURN RIGHT")
        ]
        
        for i, (label, cmd) in enumerate(commands):
            btn = ctk.CTkButton(buttons_grid, text=label, width=150,
                                command=lambda c=cmd: self.execute_command(c))
            btn.grid(row=i//2, column=i%2, padx=10, pady=5)
        
        # Sección de usuarios conectados
        users_frame = ctk.CTkFrame(main_frame)
        users_frame.pack(fill="both", expand=True, padx=10, pady=10)
        
        ctk.CTkLabel(users_frame, text="Usuarios Conectados", 
                     font=("Verdana", 16, "bold")).pack(pady=10)
        
        # Tabla de usuarios
        self.users_text = ctk.CTkTextbox(users_frame, height=200)
        self.users_text.pack(fill="both", expand=True, padx=10, pady=10)
        
        refresh_btn = ctk.CTkButton(users_frame, text="Actualizar Lista", 
                                     command=self.refresh_users)
        refresh_btn.pack(pady=10)
        
        # Cargar usuarios inicial
        self.refresh_users()
    
    def execute_command(self, cmd):
        success, message = send_command(cmd)
        
        if success:
            messagebox.showinfo("Éxito", f"Comando '{cmd}' ejecutado correctamente")
        else:
            messagebox.showerror("Error", f"No se pudo ejecutar: {message}")
    
    def refresh_users(self):
        users = list_users()
        
        self.users_text.delete("1.0", "end")
        
        if users:
            self.users_text.insert("1.0", f"{'Usuario':<20} {'Rol':<15} {'Dirección IP':<20}\n")
            self.users_text.insert("end", "="*60 + "\n")
            
            for user in users:
                line = f"{user['user']:<20} {user['role']:<15} {user['ip']:<20}\n"
                self.users_text.insert("end", line)
        else:
            self.users_text.insert("1.0", "No se pudo obtener la lista de usuarios")
    
    def run(self):
        self.root.mainloop()

# ====== Inicio ======
if __name__ == "__main__":
    app = AdminPanel()
    app.run()

