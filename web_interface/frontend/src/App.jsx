import { useState } from 'react'
import { BrowserRouter, Routes, Route } from 'react-router-dom'
import './App.css'
import HomePage from './pages/HomePage'
import TopNavBarLayout from './layout/TopNavBarLayout'
import LoginPage from './pages/LoginPage'
import SettingsPage from './pages/SettingsPage'
import UpdatePage from './pages/UpdatePage'
import ProtectedRouteLayout from './layout/ProtectedRouteLayout'

function App() {

  return (
    <BrowserRouter>
      <Routes >
        <Route path="/" element={<TopNavBarLayout />}>
          <Route path="/" element={<ProtectedRouteLayout />}>
            <Route index element={<HomePage />} />
            <Route path='/update' element={<UpdatePage />} />
            <Route path='/settings' element={<SettingsPage />} />
          </Route>
        </Route>
        <Route path='/login' element={<LoginPage />} />
      </Routes>
    </BrowserRouter>
  )
}

export default App
