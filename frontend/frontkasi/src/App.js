import './App.css';
import Console from './Console/Console';
import Video from './Video/Video';
import Sidebar from './Sidebar/Sidebar';
import Navbar from './Navbar/Navbar';
import DroneModel from './DroneModel/DroneModel';
import Map2D from './Map2D/Map2D';
import TelemetryClient from './TelemetryClient';
import { useEffect, useState } from 'react';
import Telemetry from './Telemetry';


const params = {
  'roll': 1.23
}

const attitude = {
  'roll': 32.3,
  'pitch': 0,
  'yaw': 0,
}


function App() {
  const [telemetry, setTelemetry] = useState(new Telemetry());

  const client = new TelemetryClient(setTelemetry);

  useEffect(() => {
    client.start();
  }, []);

  return (
    <div className='container-fluid'>
      <Navbar telemetry={telemetry}/>

      <div className='container-fluid'>
        <div className='row'>
          <div className='d-flex flex-row p-1'>
            <div className='bg-primary'>
              <Video />
            </div>
              <Map2D />
              <Sidebar telemetry={telemetry}/>
            </div>
        </div>
        <div className='row'>
          <div className='col-5 p-1'>
            <Console />
          </div>
          <div className='col-2 p-1'>
            <DroneModel roll={attitude.roll} pitch={attitude.pitch} yaw={attitude.yaw}/>
          </div>
          <div className='col-5 p-1'>
            <Console />
          </div>
        </div>

      </div>
    </div>
  );
}

export default App;
