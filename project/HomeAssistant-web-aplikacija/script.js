// Ovo mijenjamo na svakom drugom HA
var global_IP_and_PORT = '192.168.82.192:8123';

var global_POST_url = 'http://' + global_IP_and_PORT + '/api/webhook/command';
var global_GET_url_first_part = 'http://' + global_IP_and_PORT + '/api/states/';

var page_directory = '/local/mypages/history.html';

var history_page_url = 'http://' + global_IP_and_PORT + '/local/history.html' ;

// Ovo mijenjamo
var token = 'eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiI0Y2E1MjJlN2YyODg0OTAyYWE4N2I3ZGQwZmMyYWIwZSIsImlhdCI6MTY4NTg4MzQzMywiZXhwIjoyMDAxMjQzNDMzfQ.HL1RxfYvAJCQ1YuVCi3KGyoEBcEE9iApXL1j2M-K9oI';

// Za intervalni GET
var interval;

// Pri scakom ponovnom učitavanju stranice čitaj vrijednost slider-a i ispiši ga
document.addEventListener("DOMContentLoaded", function() {

	var slider = document.getElementById("servo-slider");
	var sliderValue = document.getElementById("slider-value-display");

	sliderValue.textContent = slider.value + '°';
});

// Pri svakom pomicanju slider-a ažuriraj ispisanu vrijednost
function updateSliderValue(value) {
	document.getElementById("slider-value-display").textContent = value + '%';
}

// Pri svakom kliku na gumb HISTORY 
function showHistory(sensor) {
		
	// GET zahtjev za history stranicu
	window.location.href = history_page_url + '?sensor=' + sensor;

}

// Funkcija za ispis svjetline
function brightness_description(brightness) {
    
    var number = parseInt(brightness);
    
    if(!isNaN(number)) {
        
        if(number > 2000) {
            return 'Dark';
        } else if(number > 1500 && number <= 2000) {
            return 'Dimly lit';
        } else {
            return 'Well lit';
        }
        
    } else {
        return 'unknown';
    }
    
}

// Svakim klikom na gumb SUBMIT
function submitServo() {

	// Izvuci vrijednost slider-a
	var value = document.getElementById('servo-slider').value;

    // Pretvori ga u stupnjeve
    var degrees = (value / 100) * 180;

	// Podatak
	var key_value = 'SERVO:' + degrees.toFixed(2) + '°';
	var data = { message: key_value };

	// POST zahtjev za postavljanjem pozicije servo-a
	fetch(global_POST_url, {
		method: 'POST',
		headers: {
			'Content-Type': 'application/json'
		},
		body: JSON.stringify(data)
		})
		.then(response => {
			if (!response.ok) console.error(); 
		})
		.catch(error => console.error(error));
}

// U slučaju da je tipka pritisnuta, pošalji POST zahtjev
function toggleLED(ledNumber) {

	// Koji prekidač je pritisnut
	var switchElement = document.getElementById('led' + ledNumber + '-switch');
	
	// Jeli prekidač upaljen ili ugašen
	var value = switchElement.checked ? 'ON' : 'OFF';

	// Podatak
	var key_value = 'LED' + ledNumber + ':' + value;
	var data = { message: key_value };

	// POST zahtjev za postavljanjem LED-ice
	fetch(global_POST_url, {
		method: 'POST',
		headers: {
			'Content-Type': 'application/json'
		},
		body: JSON.stringify(data)
		})
		.then(response => {
			if (!response.ok) console.error(); 
		})
		.catch(error => console.error(error));

}

// Niz imena senzora kojima pristupamo
const entity_ids = [
	'sensor.photoresistor0_sensor',
	'sensor.photoresistor1_sensor',
	'sensor.temperature_sensor',
	'sensor.humidity_sensor',
	'sensor.servo_sensor',
	'sensor.led0_sensor',
	'sensor.led1_sensor',
	'sensor.led2_sensor'
];

// Napravi prvi GET upit
// Za svaki entitet
entity_ids.forEach((entity_id => {
   
	// Stvori url za svaki senzor
	const url = global_GET_url_first_part + entity_id;
   	
	// Šalji GET zahtjev za svaki senzor
	fetch(url, {
		headers: {
			'Accept': 'application/json',
			'Authorization': 'Bearer ' + token
		}
	})
	.then(response => response.json())
	.then(data => {
	    
	    // Inicijalno stanje checkbox-a
	    if (entity_id.startsWith('sensor.led')) {
	        
	        // Izvuci broj
			var ledNumber = entity_id.slice(10, -7);
			
			var checkbox = document.getElementById('led' + ledNumber + '-switch');
		    if(data.state === 'ON') checkbox.checked = true;
		    if(data.state === 'OFF') checkbox.checked = false;
	    
	        document.getElementById(entity_id).textContent = data.state;
	    
	    }
	    
	    // Inicijalno stanje klizača
	    else if (entity_id === 'sensor.servo_sensor') {
	        
	        // Klizač i broj pokraj klizača
			var servoSlider = document.getElementById('servo-slider');
			var sliderValueDisplay = document.getElementById('slider-value-display');
			var slider_value_degrees = parseFloat(data.state);
			console.log(data.state);
			console.log(sliderValueDisplay);
			var slider_value_percentage = 50;
			
			// Ako je pretvorba uspjela
			if(!isNaN(slider_value_degrees)) {
    			    
    			    slider_value_percentage = (slider_value_degrees.toFixed(2) / 180) * 100;
    			    updateSliderValue(slider_value_percentage.toFixed(2));
    			    servoSlider.value = slider_value_percentage.toFixed(2);
			        
			} else {
			        
			        updateSliderValue(slider_value_percentage.toFixed(2));
    			    servoSlider.value = slider_value_percentage.toFixed(2);
			        
			}
	        
	        if(data.state !== 'unknown') document.getElementById(entity_id).textContent = slider_value_percentage.toFixed(2)+ '%';
	        else document.getElementById(entity_id).textContent = data.state;
	        
	    } 
	    
	    // Inicijalno stanje svjetlosnih senzora
	    else if(entity_id === 'sensor.photoresistor0_sensor' || entity_id === 'sensor.photoresistor1_sensor') {
			    
			var brightness = brightness_description(data.state);
			document.getElementById(entity_id).textContent = brightness;
			    
		}  else {
	        document.getElementById(entity_id).textContent = data.state;
	    }
	    
	})
	.catch(error => console.error(error));
}));    

// Periodično šalji GET upit da dobiješ vrijednosti senzora (2 sekunde)
interval = setInterval(() => {
    
	// Za svaki entitet
	entity_ids.forEach((entity_id => {
    
		// Stvori url za svaki senzor
		const url = global_GET_url_first_part + entity_id;
    	
		// Šalji GET zahtjev za svaki senzor
		fetch(url, {
			headers: {
				'Accept': 'application/json',
				'Authorization': 'Bearer ' + token
			}
		})
		.then(response => response.json())
		.then(data => {
		    
		    // U slučaju da je podatak u vezi servo-a
			if(entity_id === 'sensor.servo_sensor') {
			    
			    var servoSlider = document.getElementById('servo-slider');
			    var sliderValueDisplay = document.getElementById('slider-value-display');
			    var slider_value_degrees = parseFloat(data.state);
			    var slider_value_percentage;
    			
    			if(!isNaN(slider_value_degrees)) {
    			    
    			    slider_value_percentage = (slider_value_degrees / 180) * 100;
    			    document.getElementById(entity_id).textContent = slider_value_percentage.toFixed(2) + '%';
			        
			    } else {
			        
			        document.getElementById(entity_id).textContent = data.state;
			        
			    }
			    
			} else if(entity_id === 'sensor.photoresistor0_sensor' || entity_id === 'sensor.photoresistor1_sensor') {
			    
			    var brightness = brightness_description(data.state);
			    document.getElementById(entity_id).textContent = brightness;
			    
			} else {
			    
			    document.getElementById(entity_id).textContent = data.state;
			
			    
			}
		})
		.catch(error => console.error(error));
	}));
    
}, 5000);

