import React, { useState, useEffect } from 'react';
import { MapContainer, TileLayer, Marker, Popup, useMap } from 'react-leaflet';
import L from 'leaflet';
import 'leaflet-rotatedmarker';
import 'leaflet/dist/leaflet.css';

// Custom icon for the drone
const droneIcon = new L.Icon({
  iconUrl: 'https://t4.ftcdn.net/jpg/02/23/31/39/360_F_223313966_LwIvXbmXjMerJkGsqnYgQKvRwbahJb1U.jpg', // replace with your drone icon URL
  iconSize: [32, 32],
  iconAnchor: [16, 16],
});

const UpdateDronePosition = ({ position, heading }) => {
  const map = useMap();
  useEffect(() => {
    const marker = L.marker(position, { icon: droneIcon, rotationAngle: heading }).addTo(map);
    marker.setRotationAngle(heading);
    return () => {
      map.removeLayer(marker);
    };
  }, [map, position, heading]);
  return null;
};


function Map2D() {
  return (
    <MapContainer
      className="markercluster-map w-100"
      center={[51.0, 19.0]}
      zoom={4}
      maxZoom={18}
      style={{ height: "500px" }} // Add this line
    >
      <TileLayer
        url="https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png"
        attribution='&copy; <a href="http://osm.org/copyright">OpenStreetMap</a> contributors'
      />

    </MapContainer>
  );
}


export default Map2D;
