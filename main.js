// main.js
const { app, BrowserWindow, Menu, dialog } = require('electron')
const { spawn } = require('child_process')
const path = require('path')

let mainWindow
let backendProcess

function createWindow() {
    mainWindow = new BrowserWindow({
        width: 1400,
        height: 900,
        webPreferences: {
            nodeIntegration: false,
            contextIsolation: true,
            webSecurity: false // Allow loading local files
        },
        title: 'P2P File Sharing System',
        show: false // Don't show until ready
    })

    // Create application menu
    const template = [
        {
            label: 'View',
            submenu: [
                {
                    label: 'Enhanced Dashboard',
                    click: () => {
                        loadDashboard()
                    }
                },
                {
                    label: 'Backend Server Interface',
                    click: () => {
                        loadClassicInterface()
                    }
                },
                { type: 'separator' },
                {
                    label: 'Reload',
                    accelerator: 'CmdOrCtrl+R',
                    click: () => {
                        mainWindow.reload()
                    }
                },
                {
                    label: 'Toggle Developer Tools',
                    accelerator: 'F12',
                    click: () => {
                        mainWindow.webContents.toggleDevTools()
                    }
                }
            ]
        },
        {
            label: 'Help',
            submenu: [
                {
                    label: 'About',
                    click: () => {
                        dialog.showMessageBox(mainWindow, {
                            type: 'info',
                            title: 'About P2P File Sharing',
                            message: 'P2P File Sharing System v1.0',
                            detail: 'A decentralized peer-to-peer file sharing application with modern dashboard and transfer history.'
                        })
                    }
                }
            ]
        }
    ]

    const menu = Menu.buildFromTemplate(template)
    Menu.setApplicationMenu(menu)

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
        
        // Wait for backend to start, then load the enhanced dashboard by default
        setTimeout(() => {
            console.log('Loading enhanced dashboard...')
            loadDashboard()
            mainWindow.show() // Show window after loading
        }, 3000)

    } catch (error) {
        console.error('Error starting backend:', error)
    }
}

function loadDashboard() {
    const dashboardPath = path.join(__dirname, 'web', 'index.html')
    console.log('Loading dashboard from:', dashboardPath)
    mainWindow.loadFile(dashboardPath)
    mainWindow.setTitle('P2P File Sharing - Enhanced Dashboard')
}

function loadClassicInterface() {
    console.log('Loading backend server interface...')
    mainWindow.loadURL('http://127.0.0.1:8080')
    mainWindow.setTitle('P2P File Sharing - Backend Server Interface')
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
