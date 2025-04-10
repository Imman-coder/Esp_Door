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
  // Handle the action here
  if (msg.action_type == "door") {
    handleDoorMechanism(msg.action);
  } else if(msg.action_type == "settings") {
    handleSettings(msg);
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
