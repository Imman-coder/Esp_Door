import React, { useEffect, useState } from "react";
import useSocket from "../hooks/useSocket";
import { SocketStatus } from "../contexts/SocketProvider";

function SettingsPage() {
  const { socket, socketStatus } = useSocket();

  const [settingsJson, setSettingsJson] = useState();
  const [selectedUserId, setSelectedUserId] = useState(0);

  const handleSocketMessages = (event) => {
    const data = JSON.parse(event.data);
    console.log("Received data:", data);
    if (data.type === "settings_update") {
      setSettingsJson(data.data);
    }
  };

  useEffect(() => {
    console.log(settingsJson);
  }, [settingsJson]);

  let handleSave = () => {
    console.log("Saving settings...");
    console.log({
        type: "save_settings",
        data: settingsJson,
      })
    socket.send(
      JSON.stringify({
        type: "save_settings",
        data: settingsJson,
      })
    );
  };

  const remDup = (u) => {

    u = Array.from(
      new Map(u.map(user => [user.username, user])).values()
    );

    return u;
  }

  useEffect(() => {
    if (!socket || socketStatus != SocketStatus.Connected) return;

    socket.addEventListener("message", handleSocketMessages);

    socket.send(
      JSON.stringify({ type: "get_settings" })
    );

    return () => {
      socket.removeEventListener("message", handleSocketMessages);
    };
  }, [socket, socketStatus]);

  return settingsJson ? (
    <>
      <div className="p-6 w-full">
        {settingsJson.wifi &&
          <div className="card bg-base-200 shadow-sm">
            <div className="card-body">
              <h2 className="card-title">Wifi Settings</h2>
              <div className="grid grid-cols-2 gap-y-4 items-center mt-4">
                <span>Access Point Mode</span>
                <input
                  type="checkbox"
                  checked={settingsJson.wifi.ap_enable}
                  onChange={(e) => {
                    setSettingsJson((prev) => ({
                      ...prev,
                      wifi: {
                        ...prev.wifi,
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
                  value={settingsJson.wifi.ap_ssid}
                  onChange={(e) => {
                    setSettingsJson((prev) => ({
                      ...prev,
                      wifi: {
                        ...prev.wifi,
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
                  value={settingsJson.wifi.ap_pass}
                  onChange={(e) => {
                    setSettingsJson((prev) => ({
                      ...prev,
                      wifi: {
                        ...prev.wifi,
                        ap_password: e.target.value,
                      },
                    }));
                  }}
                />
                <span>Station Mode</span>
                <input
                  type="checkbox"
                  className="checkbox checkbox-lg"
                  checked={settingsJson.wifi.sta_enable}
                  onChange={(e) => {
                    setSettingsJson((prev) => ({
                      ...prev,
                      wifi: {
                        ...prev.wifi,
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
                  value={settingsJson.wifi.sta_ssid}
                  onChange={(e) => {
                    setSettingsJson((prev) => ({
                      ...prev,
                      wifi: {
                        ...prev.wifi,
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
                  value={settingsJson.wifi.sta_pass}
                  onChange={(e) => {
                    setSettingsJson((prev) => ({
                      ...prev,
                      wifi: {
                        ...prev.wifi,
                        sta_password: e.target.value,
                      },
                    }));
                  }}
                />
              </div>
            </div>
          </div>
        }
        {settingsJson.users &&
          <div className="card bg-base-200 shadow-sm mt-6">
            <div className="card-body">
              <h2 className="card-title">Manage User</h2>
              <select
                className="select"
                onChange={(e) => {
                  let sid = e.target.selectedIndex;
                  if (sid > settingsJson.users.length)
                    settingsJson.users.push({ username: "", new_user: true, pin: "", num_tags: 0, admin: false })
                  setSelectedUserId(sid);
                }}
                value={selectedUserId == 0 ? "Select a user" : settingsJson.users[selectedUserId - 1].username}
              // defaultValue="Select a user"
              >
                <option disabled={true} >Select a user</option>
                {settingsJson && settingsJson.users.map(user =>
                  <option key={user.username} value={user.username}>{user.username} {user.delete && "(Delete)"}</option>
                )}
                <option value="Add new"> Add new </option>
              </select>
              <div className="grid grid-cols-2 gap-y-4 items-center mt-4">

                {selectedUserId != 0 && <>
                  <span>Username</span>
                  <input
                    type="text"
                    placeholder="Type here"
                    className="input w-full"
                    value={settingsJson.users[selectedUserId - 1].username}
                    onChange={(e) => {
                      let u = settingsJson.users[selectedUserId - 1]
                      u.username = e.target.value;

                      setSettingsJson((prev) => ({
                        ...prev,
                      }));
                    }}
                  />
                  <span>Pin</span>
                  <input
                    type="text"
                    placeholder="Type here"
                    className="input w-full"
                    value={settingsJson.users[selectedUserId - 1].pin}
                    onChange={(e) => {
                      let u = settingsJson.users[selectedUserId - 1]
                      u.pin = e.target.value;
                      setSettingsJson((prev) => ({
                        ...prev,
                      }));
                    }}
                  />
                  <span>Admin</span>
                  <input
                    type="checkbox"
                    className="checkbox checkbox-lg"
                    checked={settingsJson.users[selectedUserId - 1].admin || false}
                    onChange={(e) => {
                      settingsJson.users[selectedUserId - 1].admin = e.target.checked
                      setSettingsJson((prev) => ({
                        ...prev
                      }));
                    }}
                  />
                  <span>Registered NFC</span>
                  <input
                    type="text"
                    value={settingsJson.users[selectedUserId - 1].num_tags}
                    disabled
                    className="input"
                  />
                </>}
              </div>
              {
                selectedUserId != 0 && <>
                  <div
                    onClick={() => {
                      settingsJson.users[selectedUserId - 1].delete = !settingsJson.users[selectedUserId - 1].delete;
                      setSettingsJson((p) => ({ ...p }));
                    }}
                    className="btn btn-error mr-4 mt-4 text-white"
                  >
                    {settingsJson.users[selectedUserId - 1].delete && "Cancel "}Delete user
                  </div>
                </>
              }
            </div>
          </div>
        }
        {settingsJson.lockdown &&
          <div className="card bg-base-200 shadow-sm mt-6">
            <div className="card-body">
              <h2 className="card-title">Lockdown Preferences</h2>
              <div className="grid grid-cols-2 gap-y-4 items-center mt-4">
                <span>Enable Lockdown Mode</span>
                <input
                  type="checkbox"
                  className="checkbox checkbox-lg"
                  checked={settingsJson.lockdown.enable}
                  onChange={(e) => {
                    setSettingsJson((prev) => ({
                      ...prev,
                      lockdown: {
                        ...prev.lockdown,
                        enable: e.target.checked,
                      }
                    }));
                  }}
                />
                <span>Max Attempt</span>
                <input
                  type="number"
                  value={settingsJson.lockdown.max_attempts}
                  placeholder="3"
                  className="input w-full"
                  onChange={(e) => {
                    setSettingsJson((prev) => ({
                      ...prev,
                      lockdown: {
                        ...prev.lockdown,
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
                  value={settingsJson.lockdown.duration}
                  onChange={(e) => {
                    setSettingsJson((prev) => ({
                      ...prev,
                      lockdown: {
                        ...prev.lockdown,
                        duration: e.target.value,
                      },
                    }));
                  }}
                />
              </div>
              {/* <div className="btn btn-neutral">Cancel Lockdown</div> */}
            </div>
          </div>
        }
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
