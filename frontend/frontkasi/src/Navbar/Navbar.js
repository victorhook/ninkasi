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

function Navbar() {
    return (
        <nav className='row'>
            <div className='d-flex'>
                <NavCard title={'Link'} value={'Connected'}/>
                <Battery percentage={50}/>
                <NavCard title={'Height'} value={'1.23m'}/>
                <NavCard title={'Mission'} value={mission}/>
                <NavCard title={'State'} value={state}/>
                <NavCard title={'Flight time'} value={flightTime}/>
            </div>
      </nav>
    );
}

export default Navbar;