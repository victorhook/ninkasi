import React, { useRef, useEffect } from 'react';
import './Video.css';

function Video() {
    const videoRef = useRef(null);

    useEffect(() => {
        const ws = new WebSocket('ws://ninkasi:2348');

        ws.onmessage = (event) => {
            const video = videoRef.current;
            if (video) {
                video.src = `data:image/jpeg;base64,${event.data}`;
            }
        };

        return () => {
            ws.close();
        };
    }, []);

    return (
        <div className='bg-primary video'>
            <img ref={videoRef} className='video-element' alt="Video Stream" />
        </div>
    );
}

export default Video;
