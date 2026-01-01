#ifndef WEB_PAGES_H
#define WEB_PAGES_H

#include <Arduino.h>

// --- HTML Content ---
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>WiFi Config</title>
  <style>
    body { font-family: sans-serif; background: #222; color: #fff; padding: 20px; text-align: center; }
    h1 { margin-bottom: 20px; }
    button { background: #007bff; color: white; padding: 10px 20px; border: none; border-radius: 5px; cursor: pointer; font-size: 16px; margin: 5px; }
    button:hover { background: #0056b3; }
    button.reset { background: #dc3545; }
    button.reset:hover { background: #a71d2a; }
    input { padding: 10px; border-radius: 5px; border: none; width: 80%; max-width: 300px; margin: 10px 0; }
    #networks ul { list-style: none; padding: 0; }
    #networks li { display: flex; justify-content: space-between; align-items: center; background: #333; margin: 5px auto; padding: 10px; width: 80%; max-width: 300px; cursor: pointer; border-radius: 5px; }
    #networks li:hover { background: #444; }
    .wifi-bars { display: flex; align-items: flex-end; height: 16px; width: 24px; gap: 2px; }
    .wifi-bars div { background: #555; width: 4px; border-radius: 1px; }
    .wifi-bars div.active { background: #00dbde; }
    .bar1 { height: 4px; }
    .bar2 { height: 8px; }
    .bar3 { height: 12px; }
    .bar4 { height: 16px; }

  </style>
</head>
<body>
  <h1>WiFi Configuration</h1>
  <button onclick="scanNetworks()">Scan Networks</button>
  <div id="networks"></div>
  <br>
  <input type="text" id="ssid" placeholder="SSID"><br>
  <input type="password" id="password" placeholder="Password"><br>
  <button onclick="saveConfig()">Save & Connect</button>
  <br><br>
  <button class="reset" onclick="resetConfig()">Reset Settings</button>

  <script>
    function scanNetworks() {
      document.getElementById('networks').innerHTML = "Scanning...";
      fetch('/scan').then(res => res.json()).then(data => {
        let html = "<ul>";
        data.forEach(net => {
          let bars = 1;
          if (net.rssi >= -60) bars = 4;
          else if (net.rssi >= -70) bars = 3;
          else if (net.rssi >= -80) bars = 2;
          
          let barsHtml = `<div class="wifi-bars">
            <div class="bar1 ${bars>=1?'active':''}"></div>
            <div class="bar2 ${bars>=2?'active':''}"></div>
            <div class="bar3 ${bars>=3?'active':''}"></div>
            <div class="bar4 ${bars>=4?'active':''}"></div>
          </div>`;

          html += `<li onclick="document.getElementById('ssid').value = '${net.ssid}'">
            <span>${net.ssid}</span>
            ${barsHtml}
          </li>`;
        });

        html += "</ul>";
        document.getElementById('networks').innerHTML = html;
      });
    }

    function saveConfig() {
      const ssid = document.getElementById('ssid').value;
      const pass = document.getElementById('password').value;
      if(!ssid) return alert("SSID required!");
      
      const formData = new FormData();
      formData.append("ssid", ssid);
      formData.append("password", pass);

      fetch('/save', { method: 'POST', body: formData }).then(res => {
        alert("Saved! Rebooting...");
      });
    }

    function resetConfig() {
      if(confirm("Forget WiFi settings?")) {
        fetch('/reset', { method: 'POST' }).then(res => alert("Reset! Rebooting..."));
      }
    }
  </script>
</body>
</html>
)rawliteral";

const char dashboard_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Device Dashboard</title>
  <style>
    /* Global Box Sizing */
    * { box-sizing: border-box; }
    body { font-family: 'Segoe UI', sans-serif; background: #222; color: #fff; margin: 0; padding: 0; text-align: center; }

    
    /* Navigation */
    nav { background: #333; overflow: hidden; display: flex; justify-content: center; border-bottom: 2px solid #444; }
    nav button { background: inherit; border: none; outline: none; cursor: pointer; padding: 14px 20px; transition: 0.3s; font-size: 17px; color: #aaa; }
    nav button:hover { background: #444; color: #fff; }
    nav button.active { background: #444; color: #00dbde; border-bottom: 3px solid #00dbde; }

    /* Content */
    .tab-content { display: none; padding: 20px; animation: fadeEffect 0.5s; }
    @keyframes fadeEffect { from {opacity: 0;} to {opacity: 1;} }

    /* Cards & Stats */
    .card { background: #333; padding: 20px; border-radius: 10px; display: inline-block; text-align: left; width: 90%; max-width: 400px; margin-top: 20px; box-shadow: 0 4px 8px rgba(0,0,0,0.3); }
    .stat { margin: 15px 0; border-bottom: 1px solid #444; padding-bottom: 5px; }
    .stat:last-child { border: none; }
    .label { color: #aaa; font-size: 0.9em; margin-bottom: 2px; }
    .value { font-size: 1.2em; font-weight: bold; color: #00dbde; }

    /* Buttons */
    .button-row { width: 90%; max-width: 400px; margin: 20px auto 0; display: flex; justify-content: space-between; }
    .action-btn { background: #007bff; color: white; padding: 12px 20px; border: none; border-radius: 5px; cursor: pointer; font-size: 16px; width: 48%; }
    .action-btn:hover { background: #0056b3; }

    .reset { background: #dc3545; }
    .reset:hover { background: #a71d2a; }
    .restart { background: #28a745; }
    .restart:hover { background: #218838; }

    /* WiFi Icon */
    .wifi-icon { position: relative; display: inline-block; width: 30px; height: 30px; margin-left: 10px; vertical-align: middle; }
    .bar { position: absolute; bottom: 0; width: 6px; background: #555; border-radius: 2px; transition: background 0.3s; }
    .bar-1 { left: 0; height: 6px; }
    .bar-2 { left: 8px; height: 14px; }
    .bar-3 { left: 16px; height: 22px; }
    .bar-4 { left: 24px; height: 30px; }
    .signal-1 .bar-1 { background: #00dbde; }
    .signal-2 .bar-1, .signal-2 .bar-2 { background: #00dbde; }
    .signal-3 .bar-1, .signal-3 .bar-2, .signal-3 .bar-3 { background: #00dbde; }
    .signal-4 .bar-1, .signal-4 .bar-2, .signal-4 .bar-3, .signal-4 .bar-4 { background: #00dbde; }
    .flex-row { display: flex; align-items: center; }
  </style>
</head>
<body>

  <nav>
    <button class="tab-link active" onclick="openTab(event, 'Status')">Status</button>
    <button class="tab-link" onclick="openTab(event, 'Settings')">Settings</button>
    <button class="tab-link" onclick="openTab(event, 'About')">About</button>
  </nav>

  <!-- STATUS TAB -->
  <div id="Status" class="tab-content" style="display: block;">
    <div class="card">
      <div class="stat"><div class="label">Connected Network</div><div class="value">%SSID%</div></div>
      <div class="stat"><div class="label">IP Address</div><div class="value">%IP%</div></div>
      <div class="stat">
        <div class="label">Signal Strength</div>
        <div class="flex-row">
          <span class="value" id="rssi-val">%RSSI%</span> <span class="value"> dBm</span>
          <div id="wifi-icon" class="wifi-icon signal-0">
            <div class="bar bar-1"></div>
            <div class="bar bar-2"></div>
            <div class="bar bar-3"></div>
            <div class="bar bar-4"></div>
          </div>
        </div>
      </div>
      <div class="stat"><div class="label">Device MAC</div><div class="value">%MAC%</div></div>
    </div>


    <div class="button-row">
      <button class="action-btn restart" onclick="restartDev()">Restart</button>
      <button class="action-btn reset" onclick="resetConfig()">Reset</button>
    </div>

  </div>

  <!-- SETTINGS TAB -->
  <div id="Settings" class="tab-content">
    <div class="card">
      <h3>Transmission Config</h3>
      <input type="text" id="t_host" placeholder="Host / IP" style="margin:5px 0; width:100%; color:black;">
      <input type="number" id="t_port" placeholder="Port (9091)" style="margin:5px 0; width:100%; color:black;">
      <input type="text" id="t_path" placeholder="Path (/transmission/rpc)" style="margin:5px 0; width:100%; color:black;">
      <input type="text" id="t_user" placeholder="Username" style="margin:5px 0; width:100%; color:black;">
      <input type="password" id="t_pass" placeholder="Password" style="margin:5px 0; width:100%; color:black;">

      <div style="display:flex; justify-content:space-between; margin-top:10px;">
        <button class="action-btn" onclick="saveTrans()" style="width:48%;">Save</button>
        <button class="action-btn" onclick="testTrans(this)" style="background:#e67e22; width:48%;">Test</button>
      </div>
      <p id="test_status" style="text-align:center; margin-top:10px; font-weight:bold;"></p>
    </div>


    <div class="button-row">
      <button class="action-btn restart" onclick="restartDev()">Restart</button>
      <button class="action-btn reset" onclick="alert('Ez a gomb jelenleg nem csinál semmit.')">Reset</button>
    </div>
  </div>


  <!-- ABOUT TAB -->
  <div id="About" class="tab-content">
    <div class="card">
      <h3>About Device</h3>
      <div class="stat"><div class="label">Version</div><div class="value">0.2.6</div></div>











      <div class="stat"><div class="label">Developer</div><div class="value">Adam Pretz</div></div>
      <div class="stat"><div class="label">Build Date</div><div class="value">2026. jan. 01.</div></div>
    </div>
  </div>


  <script>
    function openTab(evt, tabName) {
      var i, tabcontent, tablinks;
      tabcontent = document.getElementsByClassName("tab-content");
      for (i = 0; i < tabcontent.length; i++) {
        tabcontent[i].style.display = "none";
      }
      tablinks = document.getElementsByClassName("tab-link");
      for (i = 0; i < tablinks.length; i++) {
        tablinks[i].className = tablinks[i].className.replace(" active", "");
      }
      document.getElementById(tabName).style.display = "block";
      evt.currentTarget.className += " active";
    }

    // Status Updates
    function updateSignal() {
      fetch('/status').then(res => res.json()).then(data => {
        const rssi = data.rssi;
        const rssiEl = document.getElementById('rssi-val');
        if(rssiEl) rssiEl.innerText = rssi;
        
        const icon = document.getElementById('wifi-icon');
        if(icon) {
          icon.className = 'wifi-icon';
          if (rssi >= -60) icon.classList.add('signal-4');
          else if (rssi >= -70) icon.classList.add('signal-3');
          else if (rssi >= -80) icon.classList.add('signal-2');
          else icon.classList.add('signal-1');
        }
      }).catch(e => console.log(e));
    }

    setInterval(updateSignal, 2000);
    updateSignal();
    loadTrans(); // Load settings on startup

    function loadTrans() {
      fetch('/getParams').then(res => res.json()).then(data => {
        document.getElementById('t_host').value = data.host || "";
        document.getElementById('t_port').value = data.port || "9091";
        document.getElementById('t_path').value = data.path || "/transmission/rpc";
        document.getElementById('t_user').value = data.user || "";
        document.getElementById('t_pass').value = data.pass || "";
      }).catch(e => console.log("No params loaded"));
    }

    function saveTrans() {
      const formData = new FormData();
      formData.append("host", document.getElementById('t_host').value);
      formData.append("port", document.getElementById('t_port').value);
      formData.append("path", document.getElementById('t_path').value);
      formData.append("user", document.getElementById('t_user').value);
      formData.append("pass", document.getElementById('t_pass').value);

      fetch('/saveParams', { method: 'POST', body: formData })
        .then(res => res.text())
        .then(msg => alert(msg))
        .catch(e => alert("Error saving"));
    }

    function testTrans(btn) {
      const oldText = btn.innerText;
      const status = document.getElementById('test_status');
      
      btn.innerText = "Testing...";
      btn.disabled = true;
      btn.style.opacity = "0.5";
      status.innerText = ""; // Clear previous

      const formData = new FormData();
      formData.append("host", document.getElementById('t_host').value);
      formData.append("port", document.getElementById('t_port').value);
      formData.append("path", document.getElementById('t_path').value);
      formData.append("user", document.getElementById('t_user').value);
      formData.append("pass", document.getElementById('t_pass').value);

      fetch('/testTransmission', { method: 'POST', body: formData })
        .then(res => res.text())
        .then(msg => {
          if(msg.includes("Success")) {
            status.style.color = "#2ecc71"; // Emerald Green
            status.innerText = msg;
          } else {
            status.style.color = "#e74c3c"; // Alizarin Red
            status.innerText = msg;
          }
        })

        .catch(e => {
            status.style.color = "#e74c3c";
            status.innerText = "Network Error";
        })
        .finally(() => {
          btn.innerText = oldText;
          btn.disabled = false;
          btn.style.opacity = "1";
        });
    }




    function restartDev() {
      if(confirm("Restart device?")) fetch('/restart', { method: 'POST' }).then(res => alert("Rebooting..."));
    }
    function resetConfig() {
      if(confirm("Forget ALL WiFi settings and reset?")) fetch('/reset', { method: 'POST' }).then(res => alert("Rebooting into AP Mode..."));
    }
  </script>
</body>
</html>
)rawliteral";

#endif
