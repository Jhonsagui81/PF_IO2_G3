import React, { useState } from 'react';
import Navbar from './components/Navbar'; 
import Dashboard from './components/Dashboard'; 
import Historial from './components/Historial'; 
import Estadisticas from './components/Estadisticas';

const App = () => {
  const [vistaActual, setVistaActual] = useState('dashboard'); 

  const handleRefresh = () => {
    //Recargamos la pagina
    window.location.reload();
  };

  return (
    <div className="bg-gray-100 min-h-screen flex">
      <Navbar onRefresh={handleRefresh} setVistaActual={setVistaActual} />      
      <div className="ml-64 p-6 w-full">
        {vistaActual === 'dashboard' && <Dashboard />}
        {vistaActual === 'historial' && <Historial />}
        {vistaActual === 'estadisticas' && <Estadisticas />}
      </div>
    </div>
  );
};


export default App;

