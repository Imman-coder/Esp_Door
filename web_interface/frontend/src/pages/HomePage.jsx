import React, { useEffect, useState } from "react";
import useSocket from "../hooks/useSocket";
import { SocketStatus } from "../contexts/SocketProvider";

function HomePage() {
  const [isLockOpened, setLockOpened] = useState();
  const [isDoorOpened, setDoorOpened] = useState();
  const { socket, socketStatus } = useSocket();

  const lockDoor = () => {
    socket.send(JSON.stringify({ type: "command", action_type: "lock" }));
  };

  const unlockDoor = () => {
    socket.send(JSON.stringify({ type: "command", action_type: "unlock" }));
  };

  /**
   * {
   *  action: "door_state" | "lock_state",
   *  state: "locked" | "unlocked" (lock state)
   *  state: "opened" | "closed"   (door state)
   * }
   */
  const handleSocketMessages = (event) => {
    const data = JSON.parse(event.data);
    console.log("Received data:", data);  
    if (data.type === "status_update") {
      setDoorOpened(data.door_status === "closed");
      setLockOpened(data.lock_status === "unlocked")
    }
  };

  useEffect(() => {
    if (!socket || socketStatus != SocketStatus.Connected) return;

    socket.addEventListener("message", handleSocketMessages);

    socket.send(JSON.stringify({ type: "get_status" }));

    return () => {
      socket.removeEventListener("message", handleSocketMessages);
    };
  }, [socket, socketStatus]);

  return (
    <div className="flex flex-col">
      <span>
        Lock State:
        <span className={!isLockOpened ? "text-green-400" : "text-red-700"}>
          {isLockOpened === undefined ? "   -" :(isLockOpened ? " Unlocked" : " Locked")}
        </span>
      </span>
      <span>
        Door State:
        <span className={!isDoorOpened ? "text-green-400" : "text-red-700"}>
          { isDoorOpened === undefined ? "  -" : (isDoorOpened ? " Opened" : " Closed")}
        </span>
      </span>
      {isLockOpened ? (
        <div onClick={lockDoor} className="btn btn-primary">
          Lock Door
        </div>
      ) : (
        <div onClick={unlockDoor} className="btn btn-primary">
          Unlock Door
        </div>
      )}
    </div>
  );
}

export default HomePage;
