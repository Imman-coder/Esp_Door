import React, { useState } from "react";
import useAuth from "../hooks/useAuth";
import axios from "../hooks/useAxios";
import { useNavigate } from "react-router-dom";
// import ThemeSwitcher from "./components/ThemeSwitcher";

function LoginPage() {
  const [username, setUsername] = useState("");
  const [password, setPassword] = useState("");
  const [errorMsg, setErrorMsg] = useState("");
  const navigate = useNavigate();

  const { setAuth } = useAuth();

  const handleSubmit = async (e) => {
    console.log(JSON.stringify({username, password}))
    e.preventDefault();
    axios
      .post("/login", { username, password }, {useCredentials: true})
      .then((response) => {
        setAuth({ username, password });
        navigate("/", { replace: true });
      })
      .catch((error) => {
        setUsername("")
        setPassword("")
      });
  };

  return (
    <div className="bg-base-200 min-h-screen flex flex-col">
      {/* <div className="p-4 flex justify-end">
        <ThemeSwitcher />
      </div> */}
      <div className="hero h-full grow">
        <div className="hero-content flex-col lg:flex-row-reverse">
          <div className="text-center lg:text-left">
            <h1 className="text-5xl font-bold">Login now!</h1>
            <p className="py-6">
              Web interface for ESP Door project. Make you way through in to
              configure it.
            </p>
          </div>
          <div className="card bg-base-100 w-full max-w-sm shrink-0 shadow-2xl">
            <div className="card-body">
              <div className="fieldset">
                <label className="fieldset-label">Username</label>
                <input
                  value={username}
                  type="username"
                  className="input"
                  placeholder="Username"
                  onChange={(e) => {
                    setUsername(e.target.value);
                  }}
                />
                <label className="fieldset-label">Password</label>
                <input
                  value={password}
                  type="password"
                  className="input"
                  placeholder="Password"
                  onChange={(e) => {
                    setPassword(e.target.value);
                  }}
                />
                <div>
                  <a className="link link-hover">Forgot password?</a>
                </div>
                <button onClick={handleSubmit} className="btn btn-neutral mt-4">
                  Login
                </button>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}

export default LoginPage;
