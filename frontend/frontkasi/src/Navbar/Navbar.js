import './Navbar.css';
import Battery from './Battery';
import NavCard from './NavCard';

/*
CONNECTED
MISSION
STATE
BATTERY
HEIGHT
*/

const mission = "IdleMission"
const state = "Idle"
const flightTime = '1.23 min'

function Navbar({telemetry}) {
    return (
        <nav className='row'>
            <div className='d-flex'>
                <NavCard title={'Link'} value={'Connected'}/>
                <Battery voltage={telemetry.bat_voltage} percentage={telemetry.bat_battery_remaining}/>
                <NavCard title={'Height'} value={'1.23m'}/>
                <NavCard title={'Mission'} value={mission}/>
                <NavCard title={'State'} value={state}/>
                <NavCard title={'Flight time'} value={flightTime}/>
            </div>
      </nav>
    );
}

export default Navbar;