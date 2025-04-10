import { StrictMode } from "react";
import { createRoot } from "react-dom/client";
import "./index.css";
import App from "./App.jsx";
import { AuthProvider } from "./contexts/AuthProvider.jsx";
import { SocketProvider } from "./contexts/SocketProvider.jsx";

createRoot(document.getElementById("root")).render(
  // <StrictMode>
    <AuthProvider>
      <SocketProvider>
        <App />
      </SocketProvider>
    </AuthProvider>
  // </StrictMode>
);
