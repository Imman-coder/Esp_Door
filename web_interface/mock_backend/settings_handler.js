import { wss } from "./index.js";

let userJson = {
  pin: "000",
  enable_nfc: false,
  tags: [""],
};

let settingsJson = {
  ap_enable: false,
  ap_ssid: "ESP Door",
  ap_password: "",
  sta_enable: true,
  sta_ssid: "MBBS-HOSTEL",
  sta_password: "",
  lockdown_enable: true,
  mdns_enable: true,
  mdns_name: "esp_door",
  ota_enable: true,
  max_attempts: 3,
  lockdown_duration: 5,
};

export const handleSettings = async (event) => {
  switch (event.action) {
    case "get_settings":
        wss.clients.forEach((client) => {
          client.send(JSON.stringify({ type: "settings", data: { settings:settingsJson, user:userJson } }));
        });
      break;
    case "update_settings":
      // Simulate updating settings in a database or file
        console.log("Updating settings...");
        console.log(event.data);
        settingsJson = {
            ...settingsJson,
            ...event.data.settings,
        }
        userJson = {
            ...userJson,
            ...event.data.user,
        }
      break;
    default:
      break;
  }
};
