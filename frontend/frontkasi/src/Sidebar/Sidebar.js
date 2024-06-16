import './Sidebar.css';
import Param from './Param';


function Sidebar({telemetry}) {
    return (
        <div className='bg-secondary sidebar'>
            <h3>SIDEBAR</h3>
            <Param name={'Uptime'} value={telemetry.uptime / 1000} />
            <Param name={'Temperature'} value={telemetry.temperature} />
            <hr className='m-0' />
            <Param name={'Roll'} value={telemetry.roll} />
            <Param name={'Pitch'} value={telemetry.pitch} />
            <Param name={'Yaw'} value={telemetry.yaw} />
            <Param name={'Altitude'} value={telemetry.altitude} />
            <Param name={'Altitude (rel)'} value={telemetry.relative_altitude} />
            <hr className='m-0' />
            <Param name={'GPS Sats'} value={telemetry.gps_satellites_visible} />
            <Param name={'GPS Hdop'} value={telemetry.gps_eph} />
            <Param name={'GPS Vdop'} value={telemetry.gps_epv} />
            <Param name={'GPS Fix'} value={telemetry.gps_fix_type} />
            <hr className='m-0' />
            <Param name={'Voltage'} value={telemetry.bat_voltage} />
            <Param name={'Current'} value={telemetry.bat_current} />
            <Param name={'Consumed'} value={telemetry.bat_current_consumed} />
            <Param name={'Remaining'} value={telemetry.bat_battery_remaining} />
        </div>
    );
}

export default Sidebar;