@echo off
echo ========================================
echo   TELEMETRY CLIENT - JAVASCRIPT
echo ========================================
echo.

echo [1/3] Checking Node.js...
where node >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo ERROR: Node.js is not installed
    echo Download from: https://nodejs.org/
    pause
    exit /b 1
)

echo [OK] Node.js found
echo.

echo [2/3] Installing dependencies...
call npm install
if %ERRORLEVEL% neq 0 (
    echo ERROR: npm install failed
    pause
    exit /b 1
)

echo [OK] Dependencies installed
echo.

echo [3/3] Starting services...
echo.
echo  - WebSocket-TCP bridge: ws port 8080
echo  - Web UI: http://localhost:3000
echo.
echo IMPORTANT: Start the C server first.
echo.

start "WebSocket Bridge" cmd /k "node server.js"
timeout /t 2 /nobreak >nul
start "HTTP Server" cmd /k "npx http-server -p 3000 -o"

echo.
echo ========================================
echo   SERVICES STARTED
echo ========================================
echo.
echo Press any key to stop services shown below...
pause >nul

taskkill /FI "WindowTitle eq WebSocket Bridge*" /F >nul 2>nul
taskkill /FI "WindowTitle eq HTTP Server*" /F >nul 2>nul

echo.
echo Services stopped.
pause
