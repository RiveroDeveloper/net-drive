import socket
import threading
import telemetry_client as tc
import car as car_module
import re
import time
import sys

# ====== Client config ======
SERVER_IP = "127.0.0.1"
SERVER_PORT = 2000

if len(sys.argv) >= 2:
    try:
        SERVER_PORT = int(sys.argv[1])
        print(f"[CONFIG] Using port {SERVER_PORT} from arguments")
    except ValueError:
        print(f"[ERROR] Invalid port: {sys.argv[1]}")
        print(f"Usage: python {sys.argv[0]} [port]")
        sys.exit(1)

client = None
authenticated = False
running = True
response = ""

# ====== PTT helpers ======
def create_message(action, data):
    """Build a PTT wire message."""
    action_padded = action.ljust(12)[:12]
    data_padded = data.ljust(150)[:150]
    return f"PTT{action_padded}{data_padded}END"

def parse_response(raw_data):
    """Parse server frame."""
    try:
        if b"PTT" in raw_data and b"END" in raw_data:
            ptt_idx = raw_data.index(b"PTT")
            end_idx = raw_data.index(b"END")
            
            action_start = ptt_idx + 3
            action_end = action_start + 12
            data_start = action_end
            data_end = end_idx
            
            action_bytes = raw_data[action_start:action_end]
            action = action_bytes.decode('utf-8', errors='ignore').strip()
            
            data_bytes = raw_data[data_start:data_end]
            data = data_bytes.decode('utf-8', errors='ignore').strip()
            
            return action, data
    except Exception as e:
        print(f"[ERROR] Parse error: {e}")
    
    return None, None

def parse_telemetry(data):
    """Parse DATA: speed=X;dir=Y;battery=Z;temp=W"""
    try:
        pattern = r'speed=([\d]+);dir=([\d]+);battery=([\d]+);temp=([\d.]+)'
        match = re.search(pattern, data)
        
        if match:
            speed = match.group(1)
            direction_deg = int(match.group(2))
            battery = match.group(3)
            temp = match.group(4)
            
            direction = format_direction(direction_deg)
            
            return speed, temp, direction, battery
    except Exception as e:
        print(f"[ERROR] Telemetry parse error: {e}")
    
    return None

def format_direction(degrees):
    """8-point compass labels (matches web UI)."""
    labels = ['N', 'NE', 'E', 'SE', 'S', 'SW', 'W', 'NW']
    idx = round(degrees / 45) % 8
    return labels[idx]

# ====== Connect + LOGIN ======
def connect_and_auth(username, password):
    """Connect to server and authenticate."""
    global client, authenticated
    
    try:
        client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        print(f"[CONNECT] Connecting to {SERVER_IP}:{SERVER_PORT}...")
        client.connect((SERVER_IP, SERVER_PORT))
        print("[CONNECT] Connected to server")
        
        login_data = f"{username};{password}"
        message = create_message("LOGIN", login_data)
        print(f"[LOGIN] Authenticating as '{username}'...")
        client.sendall(message.encode('utf-8'))
        
        response_pkt = client.recv(1024)
        action, data = parse_response(response_pkt)
        
        if action == "OK" and "AUTH_OK" in data:
            authenticated = True
            print("[LOGIN] Authentication successful")
            
            if "role=ADMIN" in data:
                role = "ADMIN"
            elif "role=OBSERVER" in data:
                role = "OBSERVER"
            else:
                role = "UNKNOWN"
            
            print(f"[LOGIN] Role: {role}")
            return True
        else:
            print(f"[LOGIN] Authentication failed: {action} - {data}")
            return False
            
    except Exception as e:
        print(f"[ERROR] Connect/auth error: {e}")
        import traceback
        traceback.print_exc()
        return False

def receive_thread():
    """Receive DATA from server."""
    global running
    global response
    
    while running and authenticated:
        try:
            data = client.recv(1024)
            if not data:
                print("[ERROR] Server closed connection")
                running = False
                break
            
            action, content = parse_response(data)
            
            if action == "DATA":
                parsed = parse_telemetry(content)
                if parsed:
                    speed, temp, direction, battery = parsed
                    
                    car_module.car.updateState(speed, direction, battery, temp)
                    
                    response = "OK"
                    
                    tc.root.after(0, lambda s=speed, t=temp, d=direction, b=battery: 
                                  tc.update_telemetry(s, t, d, b))  
        except Exception as e:
            if running:
                print(e)
                response = "ERROR"
                print(response)
            break
        
def response_thread():
    global response
    try:
        while running:
            if not response:
                print("[CLIENT] No pending STATUS to send")
                time.sleep(2)    
            else:
                print(f"[STATUS]", response)
                pttresponse = "PTT" + "STATUS" + "      " + response + "END"
                client.sendall(pttresponse.encode('utf-8'))
                response = ""
    finally:
        client.close() 

def start_client():
    """Start observer (built-in credentials)."""
    username = "observer"
    password = "observer123"
    
    print("=" * 50)
    print("  NetDrive - Observer client")
    print("=" * 50)
    
    if connect_and_auth(username, password):
        recv_thread = threading.Thread(target=receive_thread, daemon=True)
        resp_thread = threading.Thread(target=response_thread, daemon=True)
        
        recv_thread.start()
        resp_thread.start()
        
        print("[INIT] Client started")
        print("[INIT] Waiting for telemetry...")
    else:
        print("[ERROR] Could not start client")
        return

start_client()

try:
    tc.root.mainloop()
except KeyboardInterrupt:
    print("\n[EXIT] Shutting down...")
finally:
    running = False
    if client:
        client.close()
