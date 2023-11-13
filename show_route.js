// script.js
// expects a center obj like : lat: { 48.8566, lng: 2.3522 }
function initMap(center_lat_lng) {
    // Create a map centered on Paris
    const map = new google.maps.Map(document.getElementById('map'), {
        center: center_lat_lng,
        zoom: 12,
    });

    // Fetch JSON data with coordinates
    fetch('path-to-your-json-file.json')
        .then(response => response.json())
        .then(data => {
            const pathCoordinates = data.map(point => ({
                lat: point.latitude,
                lng: point.longitude,
            }));

            // Create a Polyline to display the route
            const route = new google.maps.Polyline({
                path: pathCoordinates,
                geodesic: true,
                strokeColor: '#FF0000',
                strokeOpacity: 1.0,
                strokeWeight: 2,
            });

            // Set the Polyline on the map
            route.setMap(map);
        })
        .catch(error => console.error('Error fetching data:', error));
}
