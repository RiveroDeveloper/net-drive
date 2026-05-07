# NetDrive - Autonomous Vehicle Telemetry System

## Project description

NetDrive is a full telemetry and control stack for an autonomous vehicle. Multiple users can connect at once to view live telemetry; administrators can also send control commands to the vehicle.

## Architecture

Three main parts:

### 1. Server (C)
- Multi-threaded TCP server in C
- Many concurrent client connections
- Token- and role-based authentication (Admin/Observer)
- Sessions with IP/port tracking
- Telemetry broadcast every 10 seconds to authenticated clients
- Vehicle command handling
- Configurable file logging (default `server.log`)
- Configurable TCP port and log file
- Client STATUS monitoring with disconnect on repeated errors

### 2. Python clients
- **Observer**: live telemetry display
- **Admin**: commands and user management
- CustomTkinter UI
- Auto login to the server
- STATUS OK/ERROR health checks
- Background thread sending STATUS every 2 seconds

### 3. JavaScript client
- Single web UI with login
- UI adapts to Admin vs Observer
- WebSocket–TCP bridge to the C server
- Telemetry view and admin controls
- STATUS OK/ERROR every 2 seconds

## PTT protocol (Proprietary Telemetry Transfer)

Text-based application-layer protocol for this project.

### Message layout

```
PTT + ACTION (12 bytes) + DATA (150 bytes) + END
```

- **PTT**: 3-byte header  
- **ACTION**: 12 space-padded bytes, message type  
- **DATA**: 150 space-padded bytes, payload  
- **END**: 3-byte trailer  

Total: **168 bytes** per frame.

### Actions

- **LOGIN**: `username;password`
- **OK** / **ERROR** / **DENIED**: server responses
- **DATA**: `speed=X;dir=Y;battery=Z;temp=W`
- **COMMAND**: vehicle command string
- **LIST**: list connected users (Admin)
- **STATUS**: `OK` or `ERROR` from client

### Flow

1. Client connects to TCP port 2000 (default).  
2. Client sends **LOGIN**.  
3. Server answers **OK** (token + role) or **ERROR**.  
4. Server sends **DATA** every 10 s to authenticated clients.  
5. Client sends **STATUS** every 2 s.  
6. Server counts consecutive errors; drops client after 3.  
7. Admin clients may send **COMMAND**.  
8. On disconnect, server clears the session.

## Authentication

### Default users

| Username | Password     | Role     |
|----------|--------------|----------|
| admin    | admin123     | ADMIN    |
| observer | observer123  | OBSERVER |
| user1    | pass1        | OBSERVER |
| root     | root         | ADMIN    |

### Roles

**ADMIN** — Live telemetry, vehicle commands, user list, admin panel.  

**OBSERVER** — Live telemetry only; no commands or admin panel.

## STATUS verification

Each client keeps a `response` flag: `""`, `"OK"`, or `"ERROR"`. A thread every 2 s sends **STATUS** and clears the flag. The server tracks consecutive **ERROR** values and disconnects after more than two in a row; **OK** resets the counter.

## Telemetry fields (example)

- Speed 0–120 km/h  
- Direction 0–359° (often shown as cardinals)  
- Battery 0–100%  
- Temperature (°C)

## Control commands

- **SPEED UP** — +10 km/h (max 120)  
- **SLOW DOWN** — −10 km/h (min 0)  
- **TURN LEFT** / **TURN RIGHT** — ±45°  

Battery drops 1% per command; temperature reacts to movement.

## Requirements

- **Server**: GCC, pthread, BSD sockets  
- **Python**: 3.7+, `customtkinter`, `Pillow`  
- **Web**: Node 14+, npm, modern browser  

## Build and run

### C server (Makefile)

```bash
cd server
make
make run
# or: ./server 3000 my_log.txt
make clean
```

Manual compile:

```bash
gcc -o server main.c car.c auth.c protocol.c logger.c -pthread
./server
./server 3000 custom.log
```

Arguments: `[port] [logfile]` — defaults `2000` and `server.log`.

### Python observer

```bash
cd clients/python_client
pip install -r requirements.txt
python client.py
python client.py 3000
```

Uses `observer` / `observer123`.

### Python admin

```bash
python admin_client.py
python admin_client.py 3000
```

Default admin: `admin` / `admin123`.

### Web client

The folder name is `clients/js client` (space). Quote it in shells.

```bash
cd "clients/js client"
npm install
# Terminal 1 – WebSocket bridge (TCP default 2000)
node server.js
node server.js 3000
# Terminal 2 – static files
npx http-server -p 3000
# Open http://localhost:3000
```

### Ports

- TCP server: **2000** (default)  
- WebSocket bridge: **8080**  
- HTTP (your choice): e.g. **3000**  

All clients must target the **same TCP port** as the C server.

## Project layout

```
netride/
├── server/
│   ├── main.c, car.c, auth.c, protocol.c, logger.c, *.h, Makefile
│   └── server.log          # generated
├── clients/
│   ├── python_client/
│   │   ├── client.py, admin_client.py, telemetry_client.py, car.py
│   │   ├── requirements.txt
│   │   └── images/auto.png
│   └── js client/
│       ├── index.html, server.js, package.json
│       └── images/auto.png
├── .gitignore
└── README.md
```

## Technical notes

- One thread per client; dedicated telemetry broadcast thread.  
- Mutexes: `clients_lock`, `car_lock`, `sessions_lock`.  
- Sessions keyed by socket fd; unique token per user.  
- PTT validation, STATUS error counting, `log_printf()` for console + file.

## Sample logs (English messages after translation)

```
[SERVER] Listening on port 2000...
[SERVER] New client connected (fd=4)
[THREAD] Client connected (fd=4) from 127.0.0.1:12345
[AUTH] Login successful: admin (role=2) fd=4
[TELEMETRY] Sent to admin (fd=4)
[COMMAND] Executed 'SPEED UP' by admin fd=4
```

## Troubleshooting

| Issue | Check |
|--------|--------|
| Won’t compile | `gcc --version`, pthread, MinGW/WSL on Windows |
| Python won’t connect | Server running, host/port, firewall |
| Web client | Bridge + HTTP server + C server running; hard refresh |
| Commands ignored | User must be ADMIN; server logs |
| Random disconnect | `server.log`, STATUS ERROR count (3 strikes) |
| No log file | Write permissions; `logger.c` linked in binary |

## Extending

- **Users**: edit `users_db` in `server/auth.c` and `users_db_count`.  
- **Commands**: `updateCarTelemetry()` in `server/car.c` + UI buttons.  
- **Telemetry interval**: `sleep(10)` in `send_telemetry()` in `server/main.c`.
