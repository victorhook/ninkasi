import './Sidebar.css';


function Param({name, value}) {
    return (
        <div className='row'>
            <div className='col-6'>{name}</div>
            <div className='col-6'>{value}</div>
        </div>
    );
}

export default Param;