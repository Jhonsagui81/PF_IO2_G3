import React from 'react';
import { FiRefreshCw } from 'react-icons/fi';
import { HiOutlineChartBar } from 'react-icons/hi';
import { FaHistory } from 'react-icons/fa';
import { AiOutlineHome } from 'react-icons/ai';

function Navbar({ onRefresh, setVistaActual }) {
  return (
    <div className="w-64 h-screen bg-gray-900 text-white shadow-lg fixed flex flex-col">
      <div className="p-6 font-bold text-3xl border-b border-gray-800">
        Parqueo USAC
      </div>
      <nav className="mt-6 flex-grow">
        <ul className="space-y-2">
          <li 
            onClick={() => setVistaActual('dashboard')}
            className="flex items-center p-4 rounded-lg hover:bg-blue-600 transition duration-200 cursor-pointer"
          >
            <AiOutlineHome className="mr-3 text-2xl" />
            <span className="transition duration-200">Dashboard</span>
          </li>
          <li 
            onClick={() => setVistaActual('historial')}
            className="flex items-center p-4 rounded-lg hover:bg-blue-600 transition duration-200 cursor-pointer"
          >
            <FaHistory className="mr-3 text-2xl" />
            <span className="transition duration-200">Historial</span>
          </li>
          <li 
            onClick={() => setVistaActual('estadisticas')}
            className="flex items-center p-4 rounded-lg hover:bg-blue-600 transition duration-200 cursor-pointer"
          >
            <HiOutlineChartBar className="mr-3 text-2xl" />
            <span className="transition duration-200">Estadísticas</span>
          </li>
        </ul>
      </nav>
      <div className="p-4 flex justify-center">
        <button
          className="bg-green-500 hover:bg-green-600 text-white font-bold py-2 px-4 rounded-lg inline-flex items-center transition duration-200"
          onClick={onRefresh}
        >
          <FiRefreshCw className="mr-2" />
          Refrescar
        </button>
      </div>
    </div>
  );
}

export default Navbar;
