import React from "react";
import ReactDOM from "react-dom/client";
import App from "./App";
import TaskManager from "./TaskManager";
import FileExplorer from "./FileExplorer";
import SettingsApp from "./SettingsApp";
import TerminalApp from "./TerminalApp";
import BrowserApp from "./BrowserApp";
import ImageViewer from "./ImageViewer";
import VideoPlayer from "./VideoPlayer";
import TextEditor from "./TextEditor";
import WindowFrame from "./WindowFrame";
import { ThemeProvider } from "./ThemeContext";
import { DisplayProvider } from "./DisplayContext";
import { Activity, Folder, Settings, Terminal, Globe, Image, Play, List } from 'lucide-react';

const searchParams = new URLSearchParams(window.location.search);
const appName = searchParams.get("app");

console.log("🚀 Genesi OS - Initializing...");
console.log("App name:", appName);
console.log("Window location:", window.location.href);

const renderApp = () => {
  // Se "app" não estiver na URL, renderiza o Desktop inteiro (App.tsx)
  if (!appName) {
    console.log("✅ Rendering full desktop (App.tsx)");
    return <App />;
  }

  console.log("✅ Rendering isolated window:", appName);
  // Caso contrário, renderiza a janela isolada com as decorações do cliente (WindowFrame)
  return (
    <div className="w-screen h-screen bg-transparent overflow-hidden p-2">
      {appName === "taskmgr" && <WindowFrame title="Task Manager" icon={Activity}><TaskManager apps={[]} onCloseApp={() => window.close()} /></WindowFrame>}
      {appName === "files" && <WindowFrame title="File Explorer" icon={Folder}><FileExplorer initialPath={searchParams.get("path") || "Home"} /></WindowFrame>}
      {appName === "settings" && <WindowFrame title="Settings" icon={Settings}><SettingsApp /></WindowFrame>}
      {appName === "terminal" && <WindowFrame title="Terminal" icon={Terminal}><TerminalApp /></WindowFrame>}
      {appName === "browser" && <WindowFrame title="Browser" icon={Globe}><BrowserApp /></WindowFrame>}
      {appName === "image-viewer" && <WindowFrame title="Image Viewer" icon={Image}><ImageViewer filePath={searchParams.get("path") || ""} fileName={searchParams.get("name") || ""} /></WindowFrame>}
      {appName === "video-player" && <WindowFrame title="Video Player" icon={Play}><VideoPlayer filePath={searchParams.get("path") || ""} fileName={searchParams.get("name") || ""} /></WindowFrame>}
      {appName === "text-editor" && <WindowFrame title="Text Editor" icon={List}><TextEditor filePath={searchParams.get("path") || ""} fileName={searchParams.get("name") || ""} /></WindowFrame>}
    </div>
  );
};

ReactDOM.createRoot(document.getElementById("root") as HTMLElement).render(
  <React.StrictMode>
    <DisplayProvider>
      <ThemeProvider>
        {renderApp()}
      </ThemeProvider>
    </DisplayProvider>
  </React.StrictMode>,
);
