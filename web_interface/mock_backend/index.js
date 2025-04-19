import express from "express";
import cors from "cors";
import { WebSocketServer } from "ws";
import { handleDoorMechanism } from "./door_handler.js";
import { handleSettings } from "./settings_handler.js";

let app = express();

export const wss = new WebSocketServer({ port: 3001 });

const corsOptions = {
  AccessControlAllowOrigin: "*",
  origin: "*",
  methods: "GET,HEAD,PUT,PATCH,POST,DELETE",
};
app.use(cors(corsOptions));

app.use(express.json());
app.use(express.urlencoded({ extended: true }));

const actionHandler = (msg) => {
  console.log("Action received:", msg);
  if (msg["type"] == "get_status") {
    wss.clients.forEach((client) => {
      client.send(
        JSON.stringify({
          type: "status_update",
          lockStatus: "unlocked",
          doorStatus: "closed",
          currentUser: "John Doe",
        })
      );
    });
  } else if (msg["type"] == "get_settings") {
    wss.clients.forEach((client) => {
      client.send(
        JSON.stringify({
          type: "settings_update",
          data: {
            wifi: {
              ap_enable: true,
              ap_ssid: "aaaaa",
              ap_pass: "aaaaa",
              sta_enable: true,
              sta_ssid: "aaaaa",
              sta_pass: "aaaaa",
            },
            lockdown: {
              enable: true,
              max_attempts: 3,
              duration: 5,
            },
            users: [
              {
                username: "i1",
                pin: "332",
                num_tags: 0,
                // admin: false,
              },
              {
                username: "i2",
                pin:"1255",
                num_tags: 2,
                admin: false,
              },
            ],
          },
        })
      );
    });
  }
};

wss.on("connection", (ws, req) => {
  console.log("Client connected");

  ws.on("message", (message) => {
    actionHandler(JSON.parse(message));
  });

  ws.on("close", () => {
    console.log("Client disconnected");
  });
});

app.post("/login", (req, res) => {
  console.log("Auth endpoint hit");
  res.status(200).json({ message: "Authentication successful" });
});

app.listen(3000, () => {
  console.log("Server is running on port 3000");
});
