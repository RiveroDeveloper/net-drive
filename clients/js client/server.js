const WebSocket = require('ws');
const net = require('net');

const WS_PORT = 8080;
const TCP_HOST = '127.0.0.1';
let TCP_PORT = 2000;  // Puerto por defecto

// Leer puerto desde argumentos de línea de comandos
if (process.argv.length >= 3) {
    const portArg = parseInt(process.argv[2]);
    if (portArg > 0 && portArg <= 65535) {
        TCP_PORT = portArg;
        console.log(`[CONFIG] Usando puerto TCP ${TCP_PORT} desde argumentos`);
    } else {
        console.error(`[ERROR] Puerto inválido: ${process.argv[2]}`);
        console.error(`Uso: node server.js [puerto_tcp_servidor]`);
        process.exit(1);
    }
}

// WebSocket Server
const wss = new WebSocket.Server({ port: WS_PORT });

console.log(`[WS SERVER] Escuchando en puerto ${WS_PORT}`);
console.log(`[INFO] Bridge WebSocket <-> TCP iniciado`);
console.log(`[INFO] Conectando a servidor TCP en ${TCP_HOST}:${TCP_PORT}`);
console.log('[INFO] Abre el navegador en: http://localhost:3000');

wss.on('connection', (ws) => {
    console.log('[WS] Cliente conectado desde navegador');
    
    // TCP Client
    const tcpClient = new net.Socket();
    
    tcpClient.connect(TCP_PORT, TCP_HOST, () => {
        console.log('[TCP] Conectado al servidor C');
    });
    
    // WS -> TCP: Forward messages from browser to C server
    ws.on('message', (data) => {
        const message = data.toString();
        console.log(`[WS -> TCP] ${message.substring(0, 50)}`);
        tcpClient.write(message);
    });
    
    // TCP -> WS: Forward messages from C server to browser
    tcpClient.on('data', (data) => {
        const message = data.toString();
        console.log(`[TCP -> WS] ${message.substring(0, 50)}`);
        
        if (ws.readyState === WebSocket.OPEN) {
            ws.send(message);
        }
    });
    
    // Handle disconnections
    ws.on('close', () => {
        console.log('[WS] Cliente desconectado');
        tcpClient.end();
    });
    
    tcpClient.on('close', () => {
        console.log('[TCP] Conexión cerrada');
        ws.close();
    });
    
    tcpClient.on('error', (err) => {
        console.error('[TCP] Error:', err.message);
        ws.close();
    });
});
