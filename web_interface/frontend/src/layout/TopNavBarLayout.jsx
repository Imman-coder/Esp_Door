import React, { useEffect } from "react";
import { Outlet, NavLink } from "react-router-dom";
import useSocket from "../hooks/useSocket";
import { SocketStatus } from "../contexts/SocketProvider";

function TopNavBarLayout() {
  const { socketStatus } = useSocket();

  useEffect(() => {
    console.log(socketStatus);
  }, [socketStatus]);

  return (
    <div className="h-screen flex flex-col">
      <div className="navbar bg-base-100 shadow-sm">
        <div className="flex-1">
          <NavLink to="/" className="btn btn-ghost text-xl">
            ESP Door
          </NavLink>
        </div>
        <div className="flex-none">
          <ul className="menu menu-horizontal px-1">
            <li>
              <NavLink
                className={({ isActive }) => (isActive ? " bg-gray-600" : "")}
                to="/"
              >
                Home
              </NavLink>
            </li>
            <li>
              <NavLink
                className={({ isActive }) => (isActive ? " bg-gray-600" : "")}
                to="/settings"
              >
                Settings
              </NavLink>
            </li>
            <li>
              <details>
                <summary>Parent</summary>
                <ul className="bg-base-100 rounded-t-none p-2">
                  <li>
                    <a>Link 1</a>
                  </li>
                  <li>
                    <a>Logout</a>
                  </li>
                </ul>
              </details>
            </li>
          </ul>
        </div>
      </div>
      <div className={`w-full flex justify-center text-xl ${socketStatus == SocketStatus.Connected ? "bg-green-500 text-black " : "bg-red-700 text-white"} `}>
        {socketStatus}
      </div>
      <div className="relative overflow-auto grow">
        <Outlet />
      </div>
    </div>
  );
}

export default TopNavBarLayout;
