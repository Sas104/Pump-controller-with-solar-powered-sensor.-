// index.h
const char* webpage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="icon" href="data:,">
    <style>
        html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center; }
        .switch { position: relative; display: inline-block; width: 120px; height: 68px; }
        .switch input { display: none; }
        .slider { position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; background-color: green; transition: .4s; border-radius: 68px; width: 100%; height: 100%; }
        .slider:before { position: absolute; content: ''; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: white; transition: .4s; border-radius: 50%; }
        input:checked + .slider { background-color: red; }
        input:focus + .slider { box-shadow: 0 0 1px #2196F3; }
        input:checked + .slider:before { transform: translateX(52px); }
    </style>
</head>
<body>
    <h1>Pump Control</h1>
    <label class="switch">
        <input type="checkbox" id="pumpSwitch" onchange="togglePump(this.checked)">
        <span class="slider"></span>
    </label>
    <p>Current State: <span id="pumpState">Loading...</span></p>
    <script>
        function togglePump(checked) {
            var xhttp = new XMLHttpRequest();
            if (checked) {
                xhttp.open("GET", "/12/on", true);
            } else {
                xhttp.open("GET", "/12/off", true);
            }
            xhttp.send();
        }

        function updateState() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function() {
                if (this.readyState == 4 && this.status == 200) {
                    var response = this.responseText.trim();
                    document.getElementById("pumpState").textContent = response;
                    document.getElementById("pumpSwitch").checked = (response == "on");
                }
            };
            xhttp.open("GET", "/state", true);
            xhttp.send();
        }

        // Update state every 2 seconds
        setInterval(updateState, 2000);
        window.onload = updateState; // Initial state update
    </script>
</body>
</html>
)rawliteral";