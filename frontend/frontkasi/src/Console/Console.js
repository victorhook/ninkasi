import './Console.css';


function Console({title = 'Unknown'}) {
    return (
        <div className='bg-primary console p-2'>
            <textarea className='col-12' />
        </div>
    );
}

export default Console;