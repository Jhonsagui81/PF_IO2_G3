import React, { useEffect, useState } from "react";
import { FaParking, FaUserGraduate, FaExternalLinkAlt } from "react-icons/fa";
import { FiDollarSign } from "react-icons/fi";
import {
  LineChart,
  Line,
  XAxis,
  YAxis,
  Tooltip,
  CartesianGrid,
} from "recharts";
import { PieChart, Pie, Cell, Legend } from "recharts";

function Dashboard() {
  const [currentTime, setCurrentTime] = useState("");
  const [parkStatus, setParkStatus] = useState({
    disponible: 0,
    estudiantes: 0,
    externos: 0,
  });
  const [entries, setEntries] = useState([]);
  const [exits, setExits] = useState([]);
  const totalSpaces = 5; // Total de espacios en el parqueo
  const occupiedSpaces = totalSpaces - parkStatus.disponible;

  useEffect(() => {
    // Actualizar la hora actual
    const updateTime = () => {
      const now = new Date();
      setCurrentTime(now.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' }));
    };

    const interval = setInterval(updateTime, 60000);
    updateTime(); // Llamada inicial
    return () => clearInterval(interval);
  }, []);

  useEffect(() => {
    const fetchData = async () => {
      try {
        
        const parkStatusResponse = await fetch('http://127.0.0.1:5000/status');
        const statusData = await parkStatusResponse.json();
        setParkStatus(statusData);

        const entriesResponse = await fetch('http://127.0.0.1:5000/entradas');
        const entriesData = await entriesResponse.json();
        setEntries(entriesData);

        const exitsResponse = await fetch('http://127.0.0.1:5000/salidas');
        const exitsData = await exitsResponse.json();
        setExits(exitsData);

      } catch (error) {
        console.error("Error fetching data:", error);
      }
    };

    fetchData();
  }, []);

  const isFull = occupiedSpaces >= totalSpaces;

  const getHourlyData = (data, action) => {
    const hourlyCount = Array(18).fill(0); // 5 AM a 10 PM
    const today = new Date()
          .toLocaleDateString("es-GT", {
            year: "numeric",
            month: "2-digit",
            day: "2-digit",
          })
          .split("/")
          .reverse()
          .join("-"); // Obtener la fecha actual en formato YYYY-MM-DD
    console.log(today);

    data.forEach((entry) => {
      const entryDate = entry.fecha_hora.split(" ")[0]; // Extraer solo la fecha
      const date = new Date(entry.fecha_hora);
      const hour = date.getHours();

      // Comprobar si la fecha de la entrada es hoy y la hora está dentro del rango
      if (entryDate === today && hour >= 5 && hour < 23) {
        const index = hour - 5; // Ajustar índice (5 AM es el índice 0)
        hourlyCount[index]++;
      }
    });

    return hourlyCount.map((count, index) => ({
      hora: `${index + 5}:00`, // Formato de hora
      count,
      action,
    }));
  };

  const entriesByHour = getHourlyData(entries, "Entradas");
  const exitsByHour = getHourlyData(exits, "Salidas");

  console.log(entriesByHour);
    console.log(exitsByHour);

  // Datos para la gráfica de pie
  const pieData = [
    { name: "Estudiantes", value: parkStatus.estudiantes },
    { name: "Externos", value: parkStatus.externos },
    { name: "Disponibles", value: parkStatus.disponible },
  ];

  const COLORS = ["#4caf50", "#845EC2", "#2196f3"]; // Colores para la gráfica de pie

  return (
    <div className="p-6 space-y-6">
      <h2 className="text-3xl font-bold text-gray-800 mb-4">
        Resumen del Parqueo
      </h2>

      {/* Hora actual*/}
      <div className="mb-4 p-4 bg-gradient-to-r from-green-400 to-blue-500 rounded-lg shadow-lg text-white text-center">
        <span className="text-2xl font-semibold">Hora actual:</span>
        <div className="text-5xl font-bold">{currentTime}</div>
      </div>

      {isFull && (
        <div className="bg-red-600 text-white p-4 rounded-lg mb-4">
          <strong>¡Alerta!</strong> El parqueo está lleno.
        </div>
      )}

      {/* Tarjeta de cuota fija */}
      <div className="bg-gray-800 text-white p-6 rounded-lg shadow-lg flex justify-between items-center transform hover:scale-105 transition duration-300">
        <div>
          <h3 className="text-lg font-semibold">Cuota Fija del Parqueo</h3>
          <p className="text-4xl font-bold mt-2">Q3.00</p>
        </div>
        <FiDollarSign className="text-5xl opacity-50" />
      </div>

      {/* Tarjetas de resumen */}
      <div className="grid grid-cols-1 md:grid-cols-3 gap-6">
        {/* Tarjeta de espacios disponibles */}
        <div className="bg-gradient-to-r from-blue-500 to-blue-700 text-white p-6 rounded-lg shadow-lg transform hover:scale-105 transition duration-300">
          <div className="flex justify-between items-center">
            <div>
              <h3 className="text-lg font-semibold">Espacios Disponibles</h3>
              <p className="text-4xl font-bold mt-2">{parkStatus.disponible}</p>
            </div>
            <FaParking className="text-5xl opacity-50" />
          </div>
        </div>

        {/* Tarjeta de estudiantes */}
        <div className="bg-gradient-to-r from-green-500 to-green-700 text-white p-6 rounded-lg shadow-lg transform hover:scale-105 transition duration-300">
          <div className="flex justify-between items-center">
            <div>
              <h3 className="text-lg font-semibold">Estudiantes</h3>
              <p className="text-4xl font-bold mt-2">
                {parkStatus.estudiantes}
              </p>
            </div>
            <FaUserGraduate className="text-5xl opacity-50" />
          </div>
        </div>

        {/* Tarjeta de externos */}
        <div className="bg-gradient-to-r from-purple-500 to-purple-700 text-white p-6 rounded-lg shadow-lg transform hover:scale-105 transition duration-300">
          <div className="flex justify-between items-center">
            <div>
              <h3 className="text-lg font-semibold">Externos</h3>
              <p className="text-4xl font-bold mt-2">{parkStatus.externos}</p>
            </div>
            <FaExternalLinkAlt className="text-5xl opacity-50" />
          </div>
        </div>
      </div>

      {/* Gráfico de pie de ocupación */}
      <div className="bg-white p-6 rounded-lg shadow-lg text-center">
        <h3 className="text-lg font-semibold mb-4">
          Distribución de Ocupación del Parqueo
        </h3>
        <div className="flex justify-center">
          <PieChart width={600} height={300}>
            <Pie
              data={pieData}
              cx="50%"
              cy="50%"
              innerRadius={60}
              outerRadius={80}
              fill="#8884d8"
              dataKey="value"
              //Mostrar etiquetas en el gráfico, con el porcentaje de ocupación y las cantidades
              label={({ name, percent, value }) =>
                `${name} (${(percent * 100).toFixed(0)}% - ${value})`
              }
            >
              {pieData.map((entry, index) => (
                <Cell
                  key={`cell-${index}`}
                  fill={COLORS[index % COLORS.length]}
                />
              ))}
            </Pie>
            <Tooltip />
            <Legend />
          </PieChart>
        </div>
      </div>

      {/* Gráficos de ocupación */}
<div className="bg-white p-6 rounded-lg shadow-lg">
  <h3 className="text-2xl font-bold text-gray-800 mb-6 text-center">
    Ocupación a lo largo del día
  </h3>

  {/* Gráfico de Entradas */}
  <div className="bg-gray-100 rounded-lg shadow-md p-4 mb-6">
    <h4 className="text-xl font-semibold mb-2">Entradas</h4>
    <LineChart
      width={500}
      height={300}
      data={entriesByHour}
      className="mx-auto"
    >
      <XAxis dataKey="hora" />
      <YAxis />
      <Tooltip />
      <CartesianGrid strokeDasharray="3 3" />
      <Line type="monotone" dataKey="count" stroke="#82ca9d" strokeWidth={3} />
    </LineChart>
  </div>

  {/* Gráfico de Salidas */}
  <div className="bg-gray-100 rounded-lg shadow-md p-4">
    <h4 className="text-xl font-semibold mb-2">Salidas</h4>
    <LineChart
      width={500}
      height={300}
      data={exitsByHour}
      className="mx-auto"
    >
      <XAxis dataKey="hora" />
      <YAxis />
      <Tooltip />
      <CartesianGrid strokeDasharray="3 3" />
      <Line type="monotone" dataKey="count" stroke="#ff6347" strokeWidth={3} />
    </LineChart>
  </div>
</div>

    </div>
  );
}

export default Dashboard;
