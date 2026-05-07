@echo off
echo ========================================
echo   CLIENTE DE TELEMETRIA - JAVASCRIPT
echo ========================================
echo.

echo [1/3] Verificando Node.js...
where node >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo ERROR: Node.js no esta instalado
    echo Descarga Node.js desde: https://nodejs.org/
    pause
    exit /b 1
)

echo [OK] Node.js instalado
echo.

echo [2/3] Instalando dependencias...
call npm install
if %ERRORLEVEL% neq 0 (
    echo ERROR: Fallo la instalacion de dependencias
    pause
    exit /b 1
)

echo [OK] Dependencias instaladas
echo.

echo [3/3] Iniciando servicios...
echo.
echo  - Bridge WebSocket-TCP: http://localhost:8080
echo  - Interfaz web: http://localhost:3000
echo.
echo IMPORTANTE: Asegurate de que el servidor C este corriendo
echo.

start "WebSocket Bridge" cmd /k "node server.js"
timeout /t 2 /nobreak >nul
start "HTTP Server" cmd /k "npx http-server -p 3000 -o"

echo.
echo ========================================
echo   SERVICIOS INICIADOS
echo ========================================
echo.
echo Presiona cualquier tecla para detener todos los servicios...
pause >nul

taskkill /FI "WindowTitle eq WebSocket Bridge*" /F >nul 2>nul
taskkill /FI "WindowTitle eq HTTP Server*" /F >nul 2>nul

echo.
echo Servicios detenidos.
pause

