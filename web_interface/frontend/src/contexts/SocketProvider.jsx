import React, { createContext, useEffect, useState } from "react";
import useAuth from "../hooks/useAuth";

const SocketContext = createContext();

export const SocketStatus = {
  Connected: "Connected",
  Reconnecting: "Reconnecting",
  Disconnected: "Disconnected",
};

export function SocketProvider({ children }) {
  let socketUrl = 
                import.meta.env.VITE_SOCKET_URL || 
                ("ws://" + window.location.host +"/ws");
                console.log(socketUrl)
  const { auth } = useAuth();

  const [socket, setSocket] = useState();
  const [socketStatus, setSocketStatus] = useState(SocketStatus.Reconnecting);

  useEffect(() => {
    const connect = () => {
      let socket = new WebSocket(socketUrl);

      socket.addEventListener("open", () => {
        setSocketStatus(SocketStatus.Connected);
      });

      socket.addEventListener("close", () => {
        setSocketStatus(SocketStatus.Reconnecting);
        setTimeout(() => {
          connect();
        }, 1000);
      });

      socket.addEventListener("error", () => {
        setSocketStatus(SocketStatus.Reconnecting);
        setTimeout(() => {
          connect();
        }, 1000);
      });

      setSocket(socket);
    };
    if (auth) {
      connect();
    } else {
      if (socket) {
        setSocketStatus(SocketStatus.Disconnected);
        socket.close();
        setSocket();
      }
    }
    return () => {
      if (socket) {
        socket.close();
        setSocketStatus(SocketStatus.Disconnected);
      }
    };

  }, [auth]);

  return (
    <SocketContext.Provider value={{socket, socketStatus}}>
      {children}
    </SocketContext.Provider>
  );
}

export default SocketContext;
