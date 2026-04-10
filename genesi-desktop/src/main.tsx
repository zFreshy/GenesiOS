import React from "react";
import ReactDOM from "react-dom/client";
import App from "./App";
import { ThemeProvider } from "./ThemeContext";
import { DisplayProvider } from "./DisplayContext";

ReactDOM.createRoot(document.getElementById("root") as HTMLElement).render(
  <React.StrictMode>
    <DisplayProvider>
      <ThemeProvider>
        <App />
      </ThemeProvider>
    </DisplayProvider>
  </React.StrictMode>,
);
