// Ovo mijenjamo
var global_IP_and_PORT = '192.168.82.192:8123';

var global_POST_url = 'http://' + global_IP_and_PORT + '/api/webhook/command';

var global_GET_url_first_part = 'http://' + global_IP_and_PORT + '/api/history/';

// Ovo mijenjamo
var token = 'eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiI0Y2E1MjJlN2YyODg0OTAyYWE4N2I3ZGQwZmMyYWIwZSIsImlhdCI6MTY4NTg4MzQzMywiZXhwIjoyMDAxMjQzNDMzfQ.HL1RxfYvAJCQ1YuVCi3KGyoEBcEE9iApXL1j2M-K9oI';

// Globalna varijabla za spremanje parametra
var urlParams = new URLSearchParams(window.location.search);
var sensorParam = urlParams.get('sensor');

// Funkcija za dohvaćanje i spremanje parametra
function setSensorParam() {
	var headingElement = document.getElementById('myHeading');
	headingElement.textContent = 'History of ' + sensorParam;
}

// Poziv funkcije pri kraju učitavanja stranice
document.addEventListener('DOMContentLoaded', function() { setSensorParam() });

// Vrijeme potrebno za radit upit za sva promjena stanja u intervalu od prije jedne minute
var formattedTime;

function setTimeParam() {

	var oneMinuteAgo = new Date();
	oneMinuteAgo.setMinutes(oneMinuteAgo.getMinutes() - 10);
	
	formattedTime = oneMinuteAgo.toISOString();
	
	console.log(formattedTime);

}

setTimeParam();

var sql_like_request = 'period/' + formattedTime + '?filter_entity_id=sensor.' + sensorParam + '_sensor';

// Dohvati JSON podatke - zadnjih 10 po vremenskoj oznaci
fetch(global_GET_url_first_part + sql_like_request, {
	headers: {
		'Accept': 'application/json',
		'Authorization': 'Bearer ' + token}
	})
	.then(response => response.json())
	.then(json => {
		
		// Pristupamo unutarnjem nizu
		const sensorData = json[0];
		const lastTenElements = sensorData.slice(-10).reverse(); // Zadnjih 10 elemenata

		console.log(sensorData);

		lastTenElements.forEach((item, index) => {
			const divId = `sensor_record${index}`;
			const div = document.getElementById(divId);

			if (div) {

				const friendlyName = item.attributes?.friendly_name || 'N/A';
				const state = item.state || 'N/A';
				const lastChanged = item.last_changed || 'N/A';

				div.innerHTML = `
				<strong>Friendly Name:</strong> ${friendlyName} <br>
				<strong>State:</strong> ${state} <br>
				<strong>Last Changed:</strong> ${lastChanged} <br>
				`;
        
			}
		});

	})
	.catch(error => {
		console.error('Error fetching sensor data:', error);
});
