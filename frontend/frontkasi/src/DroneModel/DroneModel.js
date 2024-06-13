import React, { useRef, useEffect } from 'react';
import * as THREE from 'three';
import './DroneModel.css';

function DroneModel({ roll, pitch, yaw }) {
    const mountRef = useRef(null);

    useEffect(() => {
        const mount = mountRef.current;

        const scene = new THREE.Scene();
        const camera = new THREE.PerspectiveCamera(75, mount.clientWidth / mount.clientHeight, 0.1, 1000);
        const renderer = new THREE.WebGLRenderer();

        renderer.setSize(mount.clientWidth, mount.clientHeight);
        mount.appendChild(renderer.domElement);

        const geometry = new THREE.BoxGeometry();
        const material = new THREE.MeshBasicMaterial({ color: 0x00ff00 });
        const cube = new THREE.Mesh(geometry, material);
        scene.add(cube);

        camera.position.z = 5;

        const animate = () => {
            requestAnimationFrame(animate);

            cube.rotation.x = pitch;
            cube.rotation.y = yaw;
            cube.rotation.z = roll;

            renderer.render(scene, camera);
        };
        animate();

        return () => {
            mount.removeChild(renderer.domElement);
        };
    }, [roll, pitch, yaw]);

    return <div ref={mountRef} className='drone-model'></div>;
}

export default DroneModel;
