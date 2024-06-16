import './Console.css';

import { useState, useEffect } from 'react';

function Console({text}) {

    const [value, setValue] = useState('');

    // Update the state when the text prop changes
    useEffect(() => {
        setValue(text.join('\n'));
    }, [text]);

    const handleChange = (event) => {
        setValue(event.target.value);
    };

    return (
        <div className='bg-primary console p-2'>
            <textarea
                className='col-12'
                value={value}
                onChange={handleChange}
            />
        </div>
    );
}

export default Console;