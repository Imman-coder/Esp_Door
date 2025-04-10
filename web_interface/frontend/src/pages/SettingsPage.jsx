import React, { useEffect, useState } from "react";
import useSocket from "../hooks/useSocket";
import { SocketStatus } from "../contexts/SocketProvider";

function SettingsPage() {
  const { socket, socketStatus } = useSocket();

  const [settingsJson, setSettingsJson] = useState();

  const handleSocketMessages = (event) => {
    const data = JSON.parse(event.data);
    console.log("Received data:", data);
    if (data.type === "settings") {
      setSettingsJson(data.data);
    }
  };

  // useEffect(() => {
  //   console.log(settingsJson);
  // }, [settingsJson]);

  let handleSave = () => {
    console.log("Saving settings...");
    socket.send(
      JSON.stringify({
        action: "update_settings",
        action_type: "settings",
        data: settingsJson,
      })
    );
  };

  useEffect(() => {
    if (!socket || socketStatus != SocketStatus.Connected) return;

    socket.addEventListener("message", handleSocketMessages);

    socket.send(
      JSON.stringify({ action: "get_settings", action_type: "settings" })
    );

    return () => {
      socket.removeEventListener("message", handleSocketMessages);
    };
  }, [socket, socketStatus]);

  return settingsJson ? (
    <>
      <div className="p-6 w-full">
        <div className="card bg-base-200 shadow-sm">
          <div className="card-body">
            <h2 className="card-title">Wifi Settings</h2>
            <div className="grid grid-cols-2 gap-y-4 items-center mt-4">
              <span>Access Point Mode</span>
              <input
                type="checkbox"
                checked={settingsJson.settings.ap_enable}
                onChange={(e) => {
                  setSettingsJson((prev) => ({
                    ...prev,
                    settings: {
                      ...prev.settings,
                      ap_enable: e.target.checked,
                    },
                  }));
                }}
                className="checkbox checkbox-lg"
              />
              <span>Access Point SSID</span>
              <input
                type="text"
                placeholder="Type here"
                className="input w-full"
                value={settingsJson.settings.ap_ssid}
                onChange={(e) => {
                  setSettingsJson((prev) => ({
                    ...prev,
                    settings: {
                      ...prev.settings,
                      ap_ssid: e.target.value,
                    },
                  }));
                }}
              />
              <span>Access Poing Password</span>
              <input
                type="text"
                placeholder="Type here"
                className="input w-full"
                value={settingsJson.settings.ap_password}
                onChange={(e) => {
                  setSettingsJson((prev) => ({
                    ...prev,
                    settings: {
                      ...prev.settings,
                      ap_password: e.target.value,
                    },
                  }));
                }}
              />
              <span>Station Mode</span>
              <input
                type="checkbox"
                className="checkbox checkbox-lg"
                checked={settingsJson.settings.sta_enable}
                onChange={(e) => {
                  setSettingsJson((prev) => ({
                    ...prev,
                    settings: {
                      ...prev.settings,
                      sta_enable: e.target.checked,
                    },
                  }));
                }}
              />
              <span>Station Mode SSID</span>
              <input
                type="text"
                placeholder="Type here"
                className="input w-full"
                value={settingsJson.settings.sta_ssid}
                onChange={(e) => {
                  setSettingsJson((prev) => ({
                    ...prev,
                    settings: {
                      ...prev.settings,
                      sta_ssid: e.target.value,
                    },
                  }));
                }}
              />
              <span>Station Mode Password</span>
              <input
                type="text"
                placeholder="Type here"
                className="input w-full"
                value={settingsJson.settings.sta_password}
                onChange={(e) => {
                  setSettingsJson((prev) => ({
                    ...prev,
                    settings: {
                      ...prev.settings,
                      sta_password: e.target.value,
                    },
                  }));
                }}
              />
            </div>
          </div>
        </div>
        <div className="card bg-base-200 shadow-sm mt-6">
          <div className="card-body">
            <h2 className="card-title">Pin Settings</h2>
            <div className="grid grid-cols-2 gap-y-4 items-center mt-4">
              <span>Master Pin</span>
              <input
                type="text"
                placeholder="Type here"
                className="input w-full"
                value={settingsJson.user.pin}
                onChange={(e) => {
                  setSettingsJson((prev) => ({
                    ...prev,
                    user: {
                      ...prev.user,
                      pin: e.target.value,
                    },
                  }));
                }}
              />
              <span>Enable NFC</span>
              <input
                type="checkbox"
                className="checkbox checkbox-lg"
                checked={settingsJson.user.enable_nfc}
                onChange={(e) => {
                  setSettingsJson((prev) => ({
                    ...prev,
                    user: {
                      ...prev.user,
                      enable_nfc: e.target.checked,
                    },
                  }));
                }}
              />
              <span>Registered NFC</span>
              <input
                type="text"
                value={settingsJson.user.nfc?.length}
                disabled 
                className="input"
              />
            </div>
            <div className="btn btn-primary mt-4">Register NFC</div>
          </div>
        </div>
        <div className="card bg-base-200 shadow-sm mt-6">
          <div className="card-body">
            <h2 className="card-title">Lockdown Preferences</h2>
            <div className="grid grid-cols-2 gap-y-4 items-center mt-4">
              <span>Enable Lockdown Mode</span>
              <input
                type="checkbox"
                className="checkbox checkbox-lg"
                checked={settingsJson.settings.lockdown_enable}
                onChange={(e) => {
                  setSettingsJson((prev) => ({
                    ...prev,
                    settings: {
                      ...prev.settings,
                      lockdown_enable: e.target.checked,
                    }
                  }));
                }}
              />
              <span>Max Attempt</span>
              <input
                type="number"
                value={settingsJson.settings.max_attempts}
                placeholder="3"
                className="input w-full"
                onChange={(e) => {
                  setSettingsJson((prev) => ({
                    ...prev,
                    settings: {
                    ...prev.settings,
                    max_attempts: e.target.value,
                  }
                  }));
                }}
              />
              <span>Lockdown Time (in minutes)</span>
              <input
                type="number"
                placeholder="3"
                className="input w-full"
                value={settingsJson.settings.lockdown_duration}
                onChange={(e) => {
                  setSettingsJson((prev) => ({
                    ...prev,
                    settings: {
                      ...prev.settings,
                      lockdown_duration: e.target.value,
                    },
                  }));
                }}
              />
            </div>
            <div className="btn btn-neutral">Cancel Lockdown</div>
          </div>
        </div>
        <div onClick={handleSave} className={`btn btn-primary mt-6 card`}>
          Save
        </div>
      </div>
    </>
  ) : (
    <></>
  );
}

export default SettingsPage;
