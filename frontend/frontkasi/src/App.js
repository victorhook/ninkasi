import './App.css';
import Console from './Console/Console';
import Video from './Video/Video';
import Sidebar from './Sidebar/Sidebar';
import Navbar from './Navbar/Navbar';
import DroneModel from './DroneModel/DroneModel';

const params = {
  'roll': 1.23
}

const attitude = {
  'roll': 32.3,
  'pitch': 0,
  'yaw': 0,
}


function App() {

  return (
    <div className='container-fluid'>
      <Navbar />

      <div className='container-fluid'>
        <div className='row'>
          <div className='d-flex flex-row p-1'>
            <div className='bg-primary'>
              <Video />
            </div>
              <Sidebar params={params}/>
              <DroneModel roll={attitude.roll} pitch={attitude.pitch} yaw={attitude.yaw}/>
          </div>
        </div>
        <div className='row'>
          <div className='col-6 p-1'>
            <Console />
          </div>
          <div className='col-6 p-1'>
            <Console />
          </div>
        </div>

      </div>
    </div>
  );
}

export default App;
