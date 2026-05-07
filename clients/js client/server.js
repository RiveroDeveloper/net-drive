const WebSocket = require('ws');
const net = require('net');

const WS_PORT = 8080;
const TCP_HOST = '127.0.0.1';
let TCP_PORT = 2000;

if (process.argv.length >= 3) {
    const portArg = parseInt(process.argv[2]);
    if (portArg > 0 && portArg <= 65535) {
        TCP_PORT = portArg;
        console.log(`[CONFIG] Using TCP port ${TCP_PORT} from arguments`);
    } else {
        console.error(`[ERROR] Invalid port: ${process.argv[2]}`);
        console.error(`Usage: node server.js [tcp_server_port]`);
        process.exit(1);
    }
}

const wss = new WebSocket.Server({ port: WS_PORT });

console.log(`[WS SERVER] Listening on port ${WS_PORT}`);
console.log(`[INFO] WebSocket <-> TCP bridge started`);
console.log(`[INFO] Connecting to C server at ${TCP_HOST}:${TCP_PORT}`);
console.log('[INFO] Open browser: http://localhost:3000');

wss.on('connection', (ws) => {
    console.log('[WS] Browser client connected');
    
    const tcpClient = new net.Socket();
    
    tcpClient.connect(TCP_PORT, TCP_HOST, () => {
        console.log('[TCP] Connected to C server');
    });
    
    ws.on('message', (data) => {
        const message = data.toString();
        console.log(`[WS -> TCP] ${message.substring(0, 50)}`);
        tcpClient.write(message);
    });
    
    tcpClient.on('data', (data) => {
        const message = data.toString();
        console.log(`[TCP -> WS] ${message.substring(0, 50)}`);
        
        if (ws.readyState === WebSocket.OPEN) {
            ws.send(message);
        }
    });
    
    ws.on('close', () => {
        console.log('[WS] Browser client disconnected');
        tcpClient.end();
    });
    
    tcpClient.on('close', () => {
        console.log('[TCP] Connection closed');
        ws.close();
    });
    
    tcpClient.on('error', (err) => {
        console.error('[TCP] Error:', err.message);
        ws.close();
    });
});
