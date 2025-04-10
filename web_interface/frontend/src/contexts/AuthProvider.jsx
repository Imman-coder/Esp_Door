import { createContext, useEffect, useState } from "react";
import axios from "axios"

const AuthContext = createContext();

import React from "react";

export function AuthProvider({ children }) {
  let v = localStorage.getItem("auth")
  console.log(v)
  if(v) v = JSON.parse(v);
  const [auth, setAuth] = useState(v); // {username:"admin", password:"123"} OR undefined

  useEffect(() => {
    if(auth)
      localStorage.setItem("auth", JSON.stringify(auth))
  }, [auth]);

  return (
    <AuthContext.Provider value={{auth, setAuth}}>
      {children}
    </AuthContext.Provider>
  );
}

export default AuthContext;
