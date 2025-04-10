import { wss } from "./index.js";

let doorState = "locked"; // Initial state of the door
let lockState = "locked"; // Initial state of the lock

let doorLockTimer = null; // Timer for the door lock mechanism

export const handleDoorMechanism = async (action) => {

    switch (action) {
        case "unlock_door":
            console.log("Unlocking door...");
            lockState = "unlocked";

            // simulate door closing after 5 seconds
            doorLockTimer = setTimeout(() => {
                handleDoorMechanism("lock_door");
            },5000); // 5 seconds timer to lock the door again

            wss.clients.forEach((client) => {
                client.send(
                    JSON.stringify({
                        action: "lock_state",
                        state: "unlocked",
                    })
                );
            });

            break;

        case "lock_door":
            console.log("Locking door...");
            lockState = "locked";
            wss.clients.forEach((client) => {
                client.send(
                    JSON.stringify({
                        action: "lock_state",
                        state: "locked",
                    })
                );
            });
            break;

        case "refresh":{
            console.log("Refreshing door state...");
            wss.clients.forEach((client) => {
                client.send(
                    JSON.stringify({
                        action: "door_state",
                        state: doorState,
                    })
                );
                client.send(
                    JSON.stringify({
                        action: "lock_state",
                        state: lockState,
                    })
                );
            });
            break;
        }
    
        default:
            break;
    }

}