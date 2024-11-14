const char* webpage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="icon" href="data:,">
    <style>
        /* Basic styles for the HTML document */
        html { 
            font-family: 'Arial', sans-serif; 
            display: inline-block; 
            margin: 0px auto; 
            text-align: center; 
            background: linear-gradient(to bottom, #e0f7fa, #ffffff); /* Light blue to white gradient */
            height: 100vh;
        }
        h1 {
            color: #00796b; /* Heading color */
        }
        
        /* Styles for the toggle switch */
        .switch { 
            position: relative; 
            display: inline-block; 
            width: 120px; 
            height: 68px; 
        }
        .switch input { display: none; } /* Hide the default checkbox */
        .slider { 
            position: absolute; 
            cursor: pointer; 
            top: 0; 
            left: 0; 
            right: 0; 
            bottom: 0; 
            background-color: #4caf50; /* Green background when off */
            transition: .4s; 
            border-radius: 68px; 
            width: 100%; 
            height: 100%; 
        }
        .slider:before { 
            position: absolute; 
            content: ''; 
            height: 52px; 
            width: 52px; 
            left: 8px; 
            bottom: 8px; 
            background-color: white; /* Circle color */
            transition: .4s; 
            border-radius: 50%; 
        }
        input:checked + .slider { background-color: #f44336; } /* Red background when on */
        input:checked + .slider:before { transform: translateX(52px); } /* Move the circle when checked */

        /* Pump state message styling */
        .pump-state {
            font-size: 20px; /* Text size for the pump state */
            color: navy; /* Fixed navy blue color */
            margin: 20px 0; /* Space around the text */
            font-weight: bold; /* Bold text */
        }

        /* Duration message styling */
        .duration-message {
            font-size: 16px; /* Smaller font size for duration */
            color: #555; /* Gray color for the duration */
            margin: 10px 0; /* Space around the text */
        }

        /* Styles for the larger graphic indicator */
        .indicator { 
            height: 60px; 
            width: 60px; 
            background-color: red; /* Initial color */
            border-radius: 50%; /* Circular shape */
            display: inline-block; 
            margin: 20px auto; /* Center the indicator */
        }
        .indicator.green { 
            background-color: green; /* Color when the external switch is off */
        }
        .indicator.blink { animation: blink 0.5s infinite; } /* Blinking animation */

        /* Keyframes for blinking animation */
        @keyframes blink {
            0% { opacity: 1; } /* Fully visible */
            50% { opacity: 0; } /* Invisible */
            100% { opacity: 1; } /* Fully visible */
        }

        /* External switch message styling with red/green color */
        .switch-message {
            font-size: 20px; /* Message text size */
            color: red; /* Default red color when switch is on */
            margin-top: 10px; /* Space above the message */
        }
        .switch-message.green { color: green; } /* Green color when external switch is off */
        .blink { animation: blink 0.5s infinite; } /* Blinking animation for text */

        /* Smaller font style for fixed duration section */
        .fixed-duration {
            font-size: 16px; /* Smaller font size for fixed duration section */
            color: #00796b; /* Consistent color */
            margin: 10px 0; /* Space around the text */
        }

    </style>
</head>
<body>
    <h1>Pump Control</h1>
    
    <!-- Larger graphic indicator for external switch -->
    <span id="switchIndicator" class="indicator"></span>
    <p class="switch-message" id="switchMessage">External Switch State: Loading...</p>

    <!-- Duration message -->
    <p class="duration-message" id="durationMessage">Duration: Loading...</p>

    <label class="switch">
        <input type="checkbox" id="pumpSwitch" onchange="togglePump(this.checked)"> <!-- Checkbox for pump control -->
        <span class="slider"></span> <!-- Slider for visual representation -->
    </label>

    <!-- Pump state message (click to turn on/off) -->
    <p class="pump-state" id="pumpState">Loading...</p>

    <!-- New section to turn on the pump for a specified duration -->
    <h2 class="fixed-duration">Turn On Pump for Fixed Duration</h2>
    <form id="durationForm">
        <label for="duration">Select Duration:</label>
        <select id="duration" name="duration">
            <option value="120">2 Minutes</option>
            <option value="180">3 Minutes</option>
            <option value="300">5 Minutes</option>
        </select>
        <button type="button" onclick="turnOnForDuration()">Turn ON for Selected Duration</button>
    </form>

<script>
    let blinkTimer = null; // Timer for managing blinking effect

// Function to toggle the pump state based on the switch
function togglePump(checked) {
    var xhttp = new XMLHttpRequest();

    // Allow the pump to be turned off regardless of external switch state
    if (checked) {
        // Prevent turning on the pump if the external switch is on
        if (document.getElementById('switchMessage').textContent.includes('On')) {
            blinkIndicator(); // Start blinking effect for both indicator and message
            document.getElementById('pumpSwitch').checked = false; // Reset pump switch visually
            return; // Exit function to prevent turning on the pump
        }
        xhttp.open("GET", "/12/on", true); // Turn on the pump
    } else {
        // Always allow turning off the pump
        xhttp.open("GET", "/12/off", true); // Turn off the pump
    }
    xhttp.send(); // Send the request
}

// Function to turn on the pump for a specified duration
function turnOnForDuration() {
    const duration = document.getElementById('duration').value; // Get selected duration
    const xhttp = new XMLHttpRequest();
    xhttp.open("GET", "/pump/on?duration=" + duration, true); // Pass duration as a query parameter
    xhttp.send();
}

// Function to update the state of the pump and external switch
function updateState() {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
            var response = JSON.parse(this.responseText); // Parse JSON response
            
            // Update pump state message based on response
            let pumpStateElement = document.getElementById("pumpState");
            pumpStateElement.textContent = response.pump == "on" ? "Click to turn off" : "Click to turn on";

            // Update duration message
            let durationElement = document.getElementById("durationMessage");
            durationElement.textContent = response.switch == "on"
                ? "Pump is turned on for " + response.duration 
                : "Pump turned off " + response.duration + " ago"; // Show duration when off

            // Set the checkbox state based on response
            document.getElementById("pumpSwitch").checked = (response.pump == "on");

            // Update switch state and indicator color
            var indicator = document.getElementById("switchIndicator");
            var switchMessage = document.getElementById("switchMessage");
            if (response.switch == "on") {
                indicator.classList.remove("green");
                indicator.classList.add("red");
                switchMessage.textContent = "Pump is On"; // Updated message
                switchMessage.classList.remove("green");
                switchMessage.classList.add("red");
            } else {
                indicator.classList.remove("red");
                indicator.classList.add("green");
                switchMessage.textContent = "Pump is Off"; // Updated message
                switchMessage.classList.remove("red");
                switchMessage.classList.add("green");
            }

            // Stop blinking if the external switch is off
            if (response.switch === 'off' && blinkTimer) {
                clearTimeout(blinkTimer);
                blinkTimer = null;
                indicator.classList.remove("blink"); // Remove blink class
                switchMessage.classList.remove("blink"); // Remove blink class
            }
        }
    };
    xhttp.open("GET", "/state", true); // Request current state
    xhttp.send(); // Send the request
}

// Function to blink the indicator and message when user bypasses external switch
function blinkIndicator() {
    var indicator = document.getElementById("switchIndicator");
    var switchMessage = document.getElementById("switchMessage");

    // Add blink class to both indicator and message
    indicator.classList.add("blink");
    switchMessage.classList.add("blink");
    
    // Clear any previous blink timer
    if (blinkTimer) {
        clearTimeout(blinkTimer);
    }

    // Set timer for blinking duration (2 seconds)
    blinkTimer = setTimeout(function() {
        // Remove blink class after 2 seconds
        indicator.classList.remove("blink");
        switchMessage.classList.remove("blink");
        blinkTimer = null; // Clear the timer
    }, 2000); // Blink duration
}

// Update state every 1 second
setInterval(updateState, 1000); // Regular state updates
window.onload = updateState; // Initial state update when the page loads
</script></body>
</html>
)rawliteral";
