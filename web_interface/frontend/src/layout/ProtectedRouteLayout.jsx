import React, { useEffect } from 'react'
import { Outlet, useNavigate } from 'react-router-dom'
import useAuth from '../hooks/useAuth'

function ProtectedRouteLayout() {

    const { auth } = useAuth();
    const navigate = useNavigate();

    useEffect(() => {
        if (!auth) {
            navigate("login");
        }
    }, [auth])

    return (
        <Outlet />
    )
}

export default ProtectedRouteLayout;