// main.js
const { app, BrowserWindow } = require('electron')
const { spawn } = require('child_process')
const path = require('path')

let mainWindow
let backendProcess

function createWindow() {
    mainWindow = new BrowserWindow({
        width: 1200,
        height: 800,
        webPreferences: {
            nodeIntegration: false,
            contextIsolation: true
        },
        title: 'P2P File Sharing System',
        show: false // Don't show until ready
    })

    console.log('Starting C++ backend...')
    
    // Start the C++ backend
    try {
        backendProcess = spawn('p2p.exe', [], {
            cwd: __dirname,
            stdio: 'pipe'
        })

        backendProcess.stdout.on('data', (data) => {
            console.log(`Backend: ${data}`)
        })

        backendProcess.stderr.on('data', (data) => {
            console.error(`Backend Error: ${data}`)
        })

        backendProcess.on('error', (error) => {
            console.error('Failed to start backend:', error)
        })

        backendProcess.on('close', (code) => {
            console.log(`Backend process exited with code ${code}`)
        })

        console.log('Backend started, waiting for initialization...')
        
        // Wait for backend to start, then load the web interface
        setTimeout(() => {
            console.log('Loading web interface...')
            mainWindow.loadURL('http://127.0.0.1:8080')
            mainWindow.show() // Show window after loading
        }, 3000)

    } catch (error) {
        console.error('Error starting backend:', error)
    }
}

app.whenReady().then(() => {
    createWindow()
})

app.on('window-all-closed', () => {
    // On macOS, keep app running even when all windows are closed
    if (process.platform !== 'darwin') {
        app.quit()
    }
})

app.on('activate', () => {
    // On macOS, re-create window when dock icon is clicked
    if (BrowserWindow.getAllWindows().length === 0) {
        createWindow()
    }
})

app.on('before-quit', () => {
    console.log('Shutting down backend...')
    // Clean shutdown of C++ backend
    if (backendProcess && !backendProcess.killed) {
        backendProcess.kill('SIGTERM')
        
        // Force kill after 5 seconds if it doesn't shut down gracefully
        setTimeout(() => {
            if (backendProcess && !backendProcess.killed) {
                console.log('Force killing backend...')
                backendProcess.kill('SIGKILL')
            }
        }, 5000)
    }
})

// Handle any uncaught exceptions
process.on('uncaughtException', (error) => {
    console.error('Uncaught Exception:', error)
})

process.on('unhandledRejection', (reason, promise) => {
    console.error('Unhandled Rejection at:', promise, 'reason:', reason)
})
