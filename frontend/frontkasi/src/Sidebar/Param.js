import './Sidebar.css';


function Param({name, value, decimals = 2}) {
    // Function to format the value to the specified number of decimals
    const formatValue = (value, decimals) => {
        if (typeof value === 'number' && typeof decimals === 'number') {
            return value.toFixed(decimals);
        }
        return value;
    };

    return (
        <div className='row'>
            <div className='col-6'>{name}</div>
            <div className='col-6'>{formatValue(value, decimals)}</div>
        </div>
    );
}

export default Param;