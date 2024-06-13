import './Sidebar.css';
import Param from './Param';


function Sidebar({params}) {
    console.log(params.roll)
    return (
        <div className='bg-secondary sidebar'>
            <h3>SIDEBAR</h3>
            <Param name={'Roll'} value={params.roll} />
            <Param name={'Pitch'} value={params.roll} />
            <Param name={'Yaw'} value={params.roll} />
            <Param name={'Height'} value={params.roll} />
            <hr className='m-0' />
            <Param name={'GPS Sats'} value={params.roll} />
            <Param name={'GPS Hdop'} value={params.roll} />
            <Param name={'GPS Status'} value={params.roll} />
            <hr className='m-0' />
            <Param name={'Voltage'} value={params.roll} />
            <Param name={'Current'} value={params.roll} />
        </div>
    );
}

export default Sidebar;