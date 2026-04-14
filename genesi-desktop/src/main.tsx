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
import { ThemeProvider } from "./ThemeContext";
import { DisplayProvider } from "./DisplayContext";

const searchParams = new URLSearchParams(window.location.search);
const appName = searchParams.get("app");

const renderApp = () => {
  // Se "app" não estiver na URL, renderiza o Desktop inteiro (App.tsx)
  if (!appName) return <App />;

  // Caso contrário, renderiza a janela isolada (WebviewWindow nativa do Tauri)
  return (
    <div className="w-full h-full bg-transparent overflow-hidden">
      {appName === "taskmgr" && <TaskManager apps={[]} onCloseApp={() => window.close()} />}
      {appName === "files" && <FileExplorer initialPath={searchParams.get("path") || "Home"} />}
      {appName === "settings" && <SettingsApp />}
      {appName === "terminal" && <TerminalApp />}
      {appName === "browser" && <BrowserApp />}
      {appName === "image-viewer" && <ImageViewer filePath={searchParams.get("path") || ""} fileName={searchParams.get("name") || ""} />}
      {appName === "video-player" && <VideoPlayer filePath={searchParams.get("path") || ""} fileName={searchParams.get("name") || ""} />}
      {appName === "text-editor" && <TextEditor filePath={searchParams.get("path") || ""} fileName={searchParams.get("name") || ""} />}
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
