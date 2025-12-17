"use client"

import { useState, useEffect } from "react"
import { Button } from "@/components/ui/button"
import { Input } from "@/components/ui/input"
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from "@/components/ui/card"
import { Alert, AlertDescription } from "@/components/ui/alert"
import { Loader2, Server, Play, Square, RefreshCw, Upload, Download, Users, Plus, Search, FileText, FolderOpen, X } from "lucide-react"

const API_BASE = "http://127.0.0.1:8080"

interface ServerStatus {
  server_running: boolean
  available_chunks: number
}

interface Peer {
  ip: string
  port: number
}

interface FileInfo {
  filename: string
  chunkIndex: number
  chunkSize: number
  checksum: string
}

interface PeerWithFiles {
  ip: string
  port: number
  files: FileInfo[]
}

interface LocalFile {
  filename: string
}

interface Message {
  type: "success" | "error"
  text: string
}

export default function P2PServerDashboard() {
  const [status, setStatus] = useState<ServerStatus | null>(null)
  const [loading, setLoading] = useState(false)
  const [actionLoading, setActionLoading] = useState<string | null>(null)
  const [filename, setFilename] = useState("")
  const [message, setMessage] = useState<Message | null>(null)
  const [peers, setPeers] = useState<Peer[]>([])
  const [showAddPeer, setShowAddPeer] = useState(false)
  const [newPeerIp, setNewPeerIp] = useState("")
  const [newPeerPort, setNewPeerPort] = useState("8080")
  const [peerFiles, setPeerFiles] = useState<PeerWithFiles[]>([])
  const [showBrowseFiles, setShowBrowseFiles] = useState(false)
  const [sharedFiles, setSharedFiles] = useState<LocalFile[]>([])
  const [downloadedFiles, setDownloadedFiles] = useState<LocalFile[]>([])
  const [showDownloadDialog, setShowDownloadDialog] = useState(false)

  const fetchStatus = async () => {
    setLoading(true)
    setMessage(null)

    try {
      const response = await fetch(`${API_BASE}/status`)
      if (!response.ok) throw new Error("Failed to fetch status")

      const data = await response.json()
      setStatus(data)
    } catch (error) {
      setMessage({
        type: "error",
        text: `Failed to fetch status: ${error instanceof Error ? error.message : "Unknown error"}`,
      })
    } finally {
      setLoading(false)
    }
  }

  const startServer = async () => {
    setActionLoading("start")
    setMessage(null)

    try {
      const response = await fetch(`${API_BASE}/server/start`, { method: "POST" })
      if (!response.ok) throw new Error("Failed to start server")

      setMessage({ type: "success", text: "Server started successfully" })
      await fetchStatus()
    } catch (error) {
      setMessage({
        type: "error",
        text: `Failed to start server: ${error instanceof Error ? error.message : "Unknown error"}`,
      })
    } finally {
      setActionLoading(null)
    }
  }

  const stopServer = async () => {
    setActionLoading("stop")
    setMessage(null)

    try {
      const response = await fetch(`${API_BASE}/server/stop`, { method: "POST" })
      if (!response.ok) throw new Error("Failed to stop server")

      setMessage({ type: "success", text: "Server stopped successfully" })
      await fetchStatus()
    } catch (error) {
      setMessage({
        type: "error",
        text: `Failed to stop server: ${error instanceof Error ? error.message : "Unknown error"}`,
      })
    } finally {
      setActionLoading(null)
    }
  }

  const handleFileUpload = async (event: React.ChangeEvent<HTMLInputElement>) => {
    const file = event.target.files?.[0]
    if (!file) return

    setActionLoading("upload")
    setMessage(null)

    try {
      console.log("Uploading file:", file.name, "Size:", file.size)
      console.log("API_BASE:", API_BASE)
      
      // Test server connectivity first
      console.log("Testing server connectivity...")
      const testResponse = await fetch(`${API_BASE}/status`)
      console.log("Server test response:", testResponse.status)
      
      if (!testResponse.ok) {
        throw new Error("Server is not responding. Please make sure the C++ server is running.")
      }
      
      console.log("Server is responding, proceeding with upload...")
      
      const response = await fetch(`${API_BASE}/files/upload`, {
        method: "POST",
        headers: {
          "X-Filename": file.name,
        },
        body: file,
      })

      console.log("Upload response status:", response.status)
      const responseText = await response.text()
      console.log("Upload response:", responseText)

      if (!response.ok) {
        throw new Error(`Upload failed (${response.status}): ${responseText}`)
      }

      const result = JSON.parse(responseText)
      setMessage({ 
        type: "success", 
        text: `File "${file.name}" has been added to shared folder successfully!` 
      })
      
      // Auto-dismiss success message after 3 seconds
      setTimeout(() => {
        setMessage(null)
      }, 3000)
      
      await fetchSharedFiles()
      await fetchStatus()
    } catch (error) {
      console.error("Upload error:", error)
      
      let errorMessage = "Unknown error"
      if (error instanceof Error) {
        errorMessage = error.message
        if (error.message.includes("Failed to fetch")) {
          errorMessage = "Cannot connect to server. Please ensure the C++ server is running on port 8080."
        }
      }
      
      setMessage({
        type: "error",
        text: `Failed to upload file: ${errorMessage}`,
      })
    } finally {
      setActionLoading(null)
      // Reset file input
      event.target.value = ""
    }
  }

  const fetchSharedFiles = async () => {
    try {
      console.log("Fetching shared files...")
      const response = await fetch(`${API_BASE}/files/shared`)
      if (!response.ok) throw new Error("Failed to fetch shared files")

      const data = await response.json()
      console.log("Shared files response:", data)
      setSharedFiles(data)
    } catch (error) {
      console.error("Failed to fetch shared files:", error)
    }
  }

  const fetchDownloadedFiles = async () => {
    try {
      const response = await fetch(`${API_BASE}/files/downloads`)
      if (!response.ok) throw new Error("Failed to fetch downloaded files")

      const data = await response.json()
      setDownloadedFiles(data)
    } catch (error) {
      console.error("Failed to fetch downloaded files:", error)
    }
  }

  const downloadFileFromPeers = async (filename: string) => {
    setActionLoading("downloadFromPeers")
    setMessage(null)

    try {
      const response = await fetch(`${API_BASE}/files/download-from-peers`, {
        method: "POST",
        body: filename,
      })

      if (!response.ok) throw new Error("Failed to download file")

      setMessage({ type: "success", text: `Download started for "${filename}"` })
      setShowDownloadDialog(false)
      await fetchDownloadedFiles()
      await fetchStatus()
    } catch (error) {
      setMessage({
        type: "error",
        text: `Failed to download file: ${error instanceof Error ? error.message : "Unknown error"}`,
      })
    } finally {
      setActionLoading(null)
    }
  }

  const openDownloadDialog = async () => {
    await browseFiles()
    setShowDownloadDialog(true)
  }

  const fetchPeers = async () => {
    try {
      const response = await fetch(`${API_BASE}/peers`)
      if (!response.ok) throw new Error("Failed to fetch peers")

      const data = await response.json()
      setPeers(data)
    } catch (error) {
      setMessage({
        type: "error",
        text: `Failed to fetch peers: ${error instanceof Error ? error.message : "Unknown error"}`,
      })
    }
  }

  const addPeer = async () => {
    if (!newPeerIp.trim() || !newPeerPort.trim()) {
      setMessage({ type: "error", text: "Please enter both IP and port" })
      return
    }

    setActionLoading("addPeer")
    setMessage(null)

    try {
      const response = await fetch(`${API_BASE}/peers/add`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ ip: newPeerIp.trim(), port: parseInt(newPeerPort) }),
      })

      if (!response.ok) throw new Error("Failed to add peer")

      setMessage({ type: "success", text: `Peer ${newPeerIp}:${newPeerPort} added successfully` })
      setNewPeerIp("")
      setNewPeerPort("8080")
      setShowAddPeer(false)
      await fetchPeers()
    } catch (error) {
      setMessage({
        type: "error",
        text: `Failed to add peer: ${error instanceof Error ? error.message : "Unknown error"}`,
      })
    } finally {
      setActionLoading(null)
    }
  }

  const browseFiles = async () => {
    setActionLoading("browseFiles")
    setMessage(null)

    try {
      const response = await fetch(`${API_BASE}/files/browse`)
      if (!response.ok) throw new Error("Failed to browse files")

      const data = await response.json()
      setPeerFiles(data.peers || [])
      setShowBrowseFiles(true)
      setMessage({ type: "success", text: "Files browsed successfully" })
    } catch (error) {
      setMessage({
        type: "error",
        text: `Failed to browse files: ${error instanceof Error ? error.message : "Unknown error"}`,
      })
    } finally {
      setActionLoading(null)
    }
  }

  const discoverPeers = async () => {
    setActionLoading("discoverPeers")
    setMessage(null)

    try {
      const response = await fetch(`${API_BASE}/peers/discover`, { method: "POST" })
      if (!response.ok) throw new Error("Failed to discover peers")

      setMessage({ type: "success", text: "Peer discovery completed" })
      await fetchPeers()
    } catch (error) {
      setMessage({
        type: "error",
        text: `Failed to discover peers: ${error instanceof Error ? error.message : "Unknown error"}`,
      })
    } finally {
      setActionLoading(null)
    }
  }

  useEffect(() => {
    fetchStatus()
    fetchPeers()
    fetchSharedFiles()
    fetchDownloadedFiles()
  }, [])

  const isServerRunning = status?.server_running ?? false

  return (
    <div className="min-h-screen bg-background p-6">
      <div className="mx-auto max-w-7xl space-y-6">
        <div className="flex items-center justify-between">
          <div>
            <h1 className="text-3xl font-bold tracking-tight">P2P Server Dashboard</h1>
            <p className="text-muted-foreground">Manage your peer-to-peer file sharing server</p>
          </div>
          <Server className="h-10 w-10 text-muted-foreground" />
        </div>

        {/* Message Alert - Moved to top */}
        {message && (
          <Alert 
            variant={message.type === "error" ? "destructive" : "default"}
            className={message.type === "success" ? "border-green-200 bg-green-50 text-green-800" : ""}
          >
            <AlertDescription className="font-medium">{message.text}</AlertDescription>
          </Alert>
        )}

        {/* Top Row - Server Status and File Operations side by side */}
        <div className="grid gap-6 lg:grid-cols-2">
          {/* Status Card */}
          <Card>
            <CardHeader>
              <CardTitle className="flex items-center gap-2">
                Server Status
                {loading && <Loader2 className="h-4 w-4 animate-spin" />}
              </CardTitle>
              <CardDescription>Current state of your P2P server</CardDescription>
            </CardHeader>
            <CardContent className="space-y-4">
              <div className="grid gap-4 sm:grid-cols-2">
                <div className="space-y-1">
                  <p className="text-sm text-muted-foreground">Status</p>
                  <div className="flex items-center gap-2">
                    <div className={`h-3 w-3 rounded-full ${isServerRunning ? "bg-green-500" : "bg-red-500"}`} />
                    <p className="text-lg font-semibold">{isServerRunning ? "Running" : "Stopped"}</p>
                  </div>
                </div>
                <div className="space-y-1">
                  <p className="text-sm text-muted-foreground">Available Chunks</p>
                  <p className="text-lg font-semibold">{status?.available_chunks ?? 0}</p>
                </div>
              </div>

              <div className="flex flex-wrap gap-2">
                <Button onClick={startServer} disabled={isServerRunning || actionLoading !== null} className="gap-2">
                  {actionLoading === "start" ? (
                    <Loader2 className="h-4 w-4 animate-spin" />
                  ) : (
                    <Play className="h-4 w-4" />
                  )}
                  Start Server
                </Button>
                <Button
                  onClick={stopServer}
                  disabled={!isServerRunning || actionLoading !== null}
                  variant="destructive"
                  className="gap-2"
                >
                  {actionLoading === "stop" ? (
                    <Loader2 className="h-4 w-4 animate-spin" />
                  ) : (
                    <Square className="h-4 w-4" />
                  )}
                  Stop Server
                </Button>
                <Button
                  onClick={fetchStatus}
                  disabled={loading || actionLoading !== null}
                  variant="outline"
                  className="gap-2 bg-transparent"
                >
                  {loading ? <Loader2 className="h-4 w-4 animate-spin" /> : <RefreshCw className="h-4 w-4" />}
                  Refresh Status
                </Button>
              </div>
            </CardContent>
          </Card>

          {/* File Operations Card */}
          <Card>
          <CardHeader>
            <CardTitle>File Operations</CardTitle>
            <CardDescription>Upload files to share or download from peers</CardDescription>
          </CardHeader>
          <CardContent className="space-y-6">
            {/* Upload/Share Section */}
            <div className="space-y-3">
              <h4 className="text-sm font-medium">Share Files</h4>
              <div className="flex flex-col gap-3">
                <div className="relative">
                  <input
                    type="file"
                    id="fileUpload"
                    className="hidden"
                    onChange={handleFileUpload}
                    disabled={!isServerRunning || actionLoading !== null}
                  />
                  <Button
                    onClick={() => document.getElementById("fileUpload")?.click()}
                    disabled={!isServerRunning || actionLoading !== null}
                    className="w-full gap-2"
                  >
                    {actionLoading === "upload" ? (
                      <Loader2 className="h-4 w-4 animate-spin" />
                    ) : (
                      <FolderOpen className="h-4 w-4" />
                    )}
                    Choose File to Share
                  </Button>
                </div>
                
                {sharedFiles.length > 0 && (
                  <div className="rounded-lg border p-3">
                    <div className="flex items-center justify-between mb-2">
                      <p className="text-xs font-medium text-muted-foreground">
                        Shared Files ({sharedFiles.length})
                      </p>
                      <Button
                        onClick={fetchSharedFiles}
                        variant="ghost"
                        size="sm"
                        className="h-6 w-6 p-0"
                        disabled={actionLoading !== null}
                      >
                        <RefreshCw className="h-3 w-3" />
                      </Button>
                    </div>
                    <div className="space-y-1 max-h-32 overflow-y-auto">
                      {sharedFiles.map((file, index) => (
                        <div key={index} className="text-xs p-1 rounded bg-muted/50">
                          {file.filename}
                        </div>
                      ))}
                    </div>
                  </div>
                )}
                
                {sharedFiles.length === 0 && (
                  <div className="rounded-lg border p-3 text-center">
                    <p className="text-xs text-muted-foreground">No files shared yet</p>
                    <Button
                      onClick={fetchSharedFiles}
                      variant="ghost"
                      size="sm"
                      className="mt-1 text-xs"
                      disabled={actionLoading !== null}
                    >
                      <RefreshCw className="h-3 w-3 mr-1" />
                      Refresh
                    </Button>
                  </div>
                )}
              </div>
            </div>

            {/* Download Section */}
            <div className="space-y-3">
              <h4 className="text-sm font-medium">Download Files</h4>
              <div className="flex flex-col gap-3">
                <Button
                  onClick={openDownloadDialog}
                  disabled={!isServerRunning || actionLoading !== null}
                  variant="secondary"
                  className="w-full gap-2"
                >
                  {actionLoading === "browseFiles" ? (
                    <Loader2 className="h-4 w-4 animate-spin" />
                  ) : (
                    <Download className="h-4 w-4" />
                  )}
                  Browse & Download from Peers
                </Button>

                {downloadedFiles.length > 0 && (
                  <div className="rounded-lg border p-3">
                    <p className="text-xs font-medium text-muted-foreground mb-2">
                      Downloaded Files ({downloadedFiles.length})
                    </p>
                    <div className="space-y-1 max-h-32 overflow-y-auto">
                      {downloadedFiles.map((file, index) => (
                        <div key={index} className="text-xs p-1 rounded bg-muted/50">
                          {file.filename}
                        </div>
                      ))}
                    </div>
                  </div>
                )}
              </div>
            </div>
          </CardContent>
        </Card>
        </div>

        {/* Peer Management Card - Full width below */}
        <Card>
          <CardHeader>
            <CardTitle className="flex items-center gap-2">
              <Users className="h-5 w-5" />
              Peer Management
            </CardTitle>
            <CardDescription>View and manage connected peers</CardDescription>
          </CardHeader>
          <CardContent className="space-y-4">
            <div className="space-y-2">
              <div className="flex items-center justify-between">
                <p className="text-sm font-medium">Known Peers ({peers.length})</p>
                <Button
                  onClick={() => setShowAddPeer(!showAddPeer)}
                  variant="outline"
                  size="sm"
                  className="gap-2"
                  disabled={actionLoading !== null}
                >
                  <Plus className="h-4 w-4" />
                  Add Peer
                </Button>
              </div>

              {showAddPeer && (
                <div className="rounded-lg border p-4 space-y-3 bg-muted/50">
                  <div className="grid gap-3 sm:grid-cols-2">
                    <div className="space-y-2">
                      <label htmlFor="peerIp" className="text-sm font-medium">
                        IP Address
                      </label>
                      <Input
                        id="peerIp"
                        placeholder="e.g., 192.168.1.100"
                        value={newPeerIp}
                        onChange={(e) => setNewPeerIp(e.target.value)}
                        disabled={actionLoading !== null}
                      />
                    </div>
                    <div className="space-y-2">
                      <label htmlFor="peerPort" className="text-sm font-medium">
                        Port
                      </label>
                      <Input
                        id="peerPort"
                        placeholder="8080"
                        value={newPeerPort}
                        onChange={(e) => setNewPeerPort(e.target.value)}
                        disabled={actionLoading !== null}
                      />
                    </div>
                  </div>
                  <div className="flex gap-2">
                    <Button
                      onClick={addPeer}
                      disabled={!newPeerIp.trim() || !newPeerPort.trim() || actionLoading !== null}
                      size="sm"
                      className="gap-2"
                    >
                      {actionLoading === "addPeer" ? (
                        <Loader2 className="h-4 w-4 animate-spin" />
                      ) : (
                        <Plus className="h-4 w-4" />
                      )}
                      Add
                    </Button>
                    <Button
                      onClick={() => {
                        setShowAddPeer(false)
                        setNewPeerIp("")
                        setNewPeerPort("8080")
                      }}
                      variant="outline"
                      size="sm"
                      disabled={actionLoading !== null}
                    >
                      Cancel
                    </Button>
                  </div>
                </div>
              )}

              <div className="rounded-lg border">
                {peers.length === 0 ? (
                  <div className="p-4 text-center text-sm text-muted-foreground">
                    No peers configured. Add a peer to get started.
                  </div>
                ) : (
                  <div className="divide-y">
                    {peers.map((peer, index) => (
                      <div key={index} className="p-3 flex items-center justify-between hover:bg-muted/50">
                        <div className="flex items-center gap-3">
                          <div className="h-2 w-2 rounded-full bg-blue-500" />
                          <div>
                            <p className="text-sm font-medium">{peer.ip}</p>
                            <p className="text-xs text-muted-foreground">Port: {peer.port}</p>
                          </div>
                        </div>
                      </div>
                    ))}
                  </div>
                )}
              </div>
            </div>

            <div className="grid gap-2 sm:grid-cols-3">
              <Button
                onClick={fetchPeers}
                variant="outline"
                size="sm"
                className="gap-2"
                disabled={loading || actionLoading !== null}
              >
                {loading ? <Loader2 className="h-4 w-4 animate-spin" /> : <RefreshCw className="h-4 w-4" />}
                Refresh
              </Button>
              <Button
                onClick={browseFiles}
                variant="outline"
                size="sm"
                className="gap-2"
                disabled={actionLoading !== null}
              >
                {actionLoading === "browseFiles" ? (
                  <Loader2 className="h-4 w-4 animate-spin" />
                ) : (
                  <FileText className="h-4 w-4" />
                )}
                Browse Files
              </Button>
              <Button
                onClick={discoverPeers}
                variant="outline"
                size="sm"
                className="gap-2"
                disabled={actionLoading !== null}
              >
                {actionLoading === "discoverPeers" ? (
                  <Loader2 className="h-4 w-4 animate-spin" />
                ) : (
                  <Search className="h-4 w-4" />
                )}
                Discover
              </Button>
            </div>
          </CardContent>
        </Card>

        {/* Download Dialog */}
        {showDownloadDialog && (
          <Card>
            <CardHeader>
              <CardTitle className="flex items-center justify-between">
                <div className="flex items-center gap-2">
                  <Download className="h-5 w-5" />
                  Download Files from Peers
                </div>
                <Button
                  onClick={() => setShowDownloadDialog(false)}
                  variant="ghost"
                  size="sm"
                >
                  <X className="h-4 w-4" />
                </Button>
              </CardTitle>
              <CardDescription>Select files to download from connected peers</CardDescription>
            </CardHeader>
            <CardContent>
              {peerFiles.length === 0 ? (
                <div className="text-center text-sm text-muted-foreground py-8">
                  <FileText className="h-12 w-12 mx-auto mb-2 opacity-50" />
                  <p>No files found on any peers</p>
                  <p className="text-xs mt-1">Make sure peers are online and sharing files</p>
                </div>
              ) : (
                <div className="space-y-4 max-h-96 overflow-y-auto">
                  {peerFiles.map((peer, peerIndex) => (
                    <div key={peerIndex} className="rounded-lg border p-4">
                      <div className="flex items-center gap-2 mb-3">
                        <div className="h-2 w-2 rounded-full bg-green-500" />
                        <p className="font-medium">{peer.ip}:{peer.port}</p>
                        <span className="text-xs text-muted-foreground">
                          ({peer.files.length} files)
                        </span>
                      </div>
                      {peer.files.length === 0 ? (
                        <p className="text-sm text-muted-foreground ml-4">No files available</p>
                      ) : (
                        <div className="ml-4 space-y-2">
                          {peer.files.map((file, fileIndex) => (
                            <div key={fileIndex} className="flex items-center justify-between p-3 rounded-lg bg-muted/30 hover:bg-muted/50 transition-colors">
                              <div className="flex-1">
                                <p className="text-sm font-medium">{file.filename}</p>
                                <p className="text-xs text-muted-foreground">
                                  Chunk {file.chunkIndex} - {(file.chunkSize / 1024).toFixed(1)} KB
                                </p>
                              </div>
                              <Button
                                onClick={() => downloadFileFromPeers(file.filename)}
                                size="sm"
                                className="gap-2 ml-3"
                                disabled={actionLoading !== null}
                              >
                                {actionLoading === "downloadFromPeers" ? (
                                  <Loader2 className="h-3 w-3 animate-spin" />
                                ) : (
                                  <Download className="h-3 w-3" />
                                )}
                                Download
                              </Button>
                            </div>
                          ))}
                        </div>
                      )}
                    </div>
                  ))}
                </div>
              )}
            </CardContent>
          </Card>
        )}
      </div>
    </div>
  )
}
