#include <Constant.hpp>

#ifdef ENABLE_CAPTIVE_MODE
const char HtmlTemplates::HTML_CONTENT[] = R"html(
<!DOCTYPE html>
<html>
<head>
  <title>Prime HomeLink</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="/styles.css">
  <script src="/script.js"></script>
</head>
<body>
  <!-- <div class="logo">
    <h1><u>Prime HomeLink</u></h1>
      <img src="/logo.png" alt="Company Logo"> 
    <h2>WiFi Configuration Portal</h2>
  </div> -->
  <div class="company-header">
    <div class="company-name">Prime HomeLink</div>
    <div class="company-tagline">WiFi Configuration Portal</div>
  </div>
  <button onclick="scanWifi()" class="btn btn-refresh" id="refreshBtn">
    <span class="refresh-icon" id="refreshIcon">&#x21bb;</span>
    Refresh Networks
  </button>
  <div id="wifiList"></div>

  <div id="modal" class="modal">
    <div class="modal-content">
      <h3>WiFi Connection Disclaimer</h3>
      <p>This WiFi network will be saved permanently and cannot be changed later.</p>
      <p>Do not change the WiFi password if any.</p>

      <div id="ssidDisplay" style="margin-bottom: 10px; font-weight: bold;"></div>

      <div id="passwordSection" class="password-modal" style="display: none;">
        <label for="wifiPassword">Password:</label>
        <div style="position: relative;">
          <input type="password" id="wifiPassword" class="password-input" placeholder="Enter WiFi password" style="padding-right: 40px;">
          <button id="togglePasswordBtn" style="position: absolute; right: 5px; top: 50%; transform: translateY(-50%); border: none; background: none; cursor: pointer;">
            &#128065;
          </button>
        </div>
      </div>

      <div>
        <button id="confirmBtn" class="btn btn-primary">Proceed</button>
        <button id="cancelBtn" class="btn btn-secondary">Cancel</button>
      </div>
    </div>
  </div>
</body>
</html>
)html";

const char HtmlTemplates::CSS[]=R"css(body { font-family: Arial, sans-serif; max-width: 800px; margin: 0 auto; padding: 20px; }
    .company-header {
      text-align: center;
      margin: 20px 0 30px;
      font-family: 'Arial', sans-serif;
      background: linear-gradient(135deg, #0066ff, #00ccff);
      -webkit-background-clip: text;
      background-clip: text;
      color: transparent;
      text-shadow: 0 2px 4px rgba(0,0,0,0.1);
      animation: glow 2s ease-in-out infinite alternate;
    }
    
    .company-name {
      font-size: 2.5rem;
      font-weight: 800;
      letter-spacing: 1px;
      margin-bottom: 5px;
    }
    
    .company-tagline {
      font-size: 1rem;
      font-weight: 300;
      letter-spacing: 3px;
      text-transform: uppercase;
    }
    
    @keyframes glow {
      from {
        text-shadow: 0 0 5px rgba(0,102,255,0.3);
      }
      to {
        text-shadow: 0 0 10px rgba(0,204,255,0.6);
      }
    }
    .logo { text-align: center; margin-bottom: 20px; }
    .logo img { max-width: 200px; }
    .wifi-list { list-style: none; padding: 0; }
    .wifi-item { padding: 10px; border: 1px solid #ddd; margin-bottom: 5px; border-radius: 5px; cursor: pointer; }
    .wifi-item:hover { background-color: #f5f5f5; }
    .wifi-name { font-weight: bold; }
    .wifi-security { float: right; color: #666; }
    .wifi-signal { float: right; margin-right: 10px; }
    .loading { text-align: center; padding: 20px; }
    .modal { display: none; position: fixed; top: 0; left: 0; width: 100%; height: 100%; background-color: rgba(0,0,0,0.5); }
    .modal-content { 
      background: white; 
      margin: 10% auto; 
      padding: 20px; 
      width: 90%; 
      max-width: 400px; 
      border-radius: 5px; 
      box-sizing: border-box; 
      overflow: hidden;
    }
    .btn { padding: 10px 15px; margin: 5px; border: none; border-radius: 4px; cursor: pointer; transition: all 0.3s; }
    .btn:hover { transform: translateY(-2px); box-shadow: 0 2px 5px rgba(0,0,0,0.2); }
    .btn-primary { background-color: #007bff; color: white; }
    .btn-secondary { background-color: #6c757d; color: white; }
    .btn-refresh { 
      background-color: #28a745; 
      color: white;
      display: flex;
      align-items: center;
      justify-content: center;
      gap: 8px;
      margin: 20px auto;
      padding: 10px 20px;
    }
    .refresh-icon {
      animation: spin 1s linear infinite;
    }
    @keyframes spin {
      0% { transform: rotate(0deg); }
      100% { transform: rotate(360deg); }
    }
    .password-modal {
      padding: 15px;
      margin-top: 15px;
      border-top: 1px solid #eee;
    }
    .password-input {
      width: 100%;
      box-sizing: border-box;
      padding: 8px;
      margin: 10px 0;
      border: 1px solid #ddd;
      border-radius: 4px;
      font-size: 16px;
    })css";

const char HtmlTemplates::JS[] = R"js(
    let selectedSsid = '';
    let isSecure = false;

    function showModal(ssid, secure) {
      selectedSsid = ssid;
      isSecure = secure;
      document.getElementById('ssidDisplay').textContent = ssid;
      const passwordSection = document.getElementById('passwordSection');
      passwordSection.style.display = secure ? 'block' : 'none';
      document.getElementById('wifiPassword').value = '';
      document.getElementById('modal').style.display = 'block';
    }

    function hideModal() {
      document.getElementById('modal').style.display = 'none';
    }

    function connectToWifi() {
      const password = isSecure ? document.getElementById('wifiPassword').value : '';
      if (isSecure && password.length < 8) {
        alert('Password must be at least 8 characters');
        return;
      }
      const confirmBtn = document.getElementById('confirmBtn');
      confirmBtn.textContent = 'Connecting...';
      confirmBtn.disabled = true;
      fetch('/connect', {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body: 'ssid=' + encodeURIComponent(selectedSsid) + '&password=' + encodeURIComponent(password)
      })
      .then(response => {
        if (!response.ok) throw new Error('Network response was not ok');
        return response.text();
      })
      .then(data => {
        alert(data.includes('success') ? 'Connected successfully!' : 'Connection failed');
        if(data.includes('success')) {
          setTimeout(() => { window.location.href = '/'; }, 2000);
        }
      })
      .catch(error => {
        alert('Error: ' + error.message);
      })
      .finally(() => {
        confirmBtn.textContent = 'Connect';
        confirmBtn.disabled = false;
      });
    }

    function scanWifi() {
      const wifiList = document.getElementById('wifiList');
      const refreshBtn = document.getElementById('refreshBtn');
      const refreshIcon = document.getElementById('refreshIcon');
      wifiList.innerHTML = '<div class="loading">Scanning WiFi networks...</div>';
      refreshBtn.disabled = true;
      refreshIcon.style.animation = 'spin 1s linear infinite';
      fetch('/scan')
      .then(response => {
        if (!response.ok) throw new Error('Network response was not ok');
        return response.json();
      })
      .then(data => {
        let html = '';
        if(data.length === 0) {
          html = '<p>No WiFi networks found</p>';
        } else {
          html = '<ul class="wifi-list">';
          data.forEach(network => {
            const escapedSsid = network.ssid.replace(/[&<>"'`=\/]/g, function(s) {
              return {'&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;', '/': '&#x2F;', '`': '&#x60;', '=': '&#x3D;'}[s];
            });
            html += `
              <li class="wifi-item" onclick="showModal('${escapedSsid}', ${network.secure})">
                <span class="wifi-name">${escapedSsid}</span>
                <span class="wifi-security">${network.secure ? '&#x1F512;' : '&#x1F310;'}</span>
                <span class="wifi-signal">${getSignalStrengthIcon(network.rssi)}</span>
              </li>`;
          });
          html += '</ul>';
        }
        wifiList.innerHTML = html;
      })
      .catch(error => {
        console.error('Error:', error);
        wifiList.innerHTML = '<p>Error scanning networks</p>';
      })
      .finally(() => {
        refreshBtn.disabled = false;
        refreshIcon.style.animation = 'none';
      });
    }

    function getSignalStrengthIcon(rssi) {
        if (rssi >= -50) return '&#x1F7E9;&#x1F7E9;&#x1F7E9;&#x1F7E9;';  
        if (rssi >= -60) return '&#x1F7E9;&#x1F7E9;&#x1F7E9;&#x2B1C;';  
        if (rssi >= -70) return '&#x1F7E9;&#x1F7E9;&#x2B1C;&#x2B1C;';    
        if (rssi >= -80) return '&#x1F7E9;&#x2B1C;&#x2B1C;&#x2B1C;';     
      return '&#x2B1C;&#x2B1C;&#x2B1C;&#x2B1C;';                   
    }

    function setupPasswordToggle() {
      const passwordInput = document.getElementById('wifiPassword');
      const togglePasswordBtn = document.getElementById('togglePasswordBtn');
      if (togglePasswordBtn && passwordInput) {
        togglePasswordBtn.addEventListener('mousedown', () => {
          passwordInput.type = 'text';
        });
        togglePasswordBtn.addEventListener('mouseup', () => {
          passwordInput.type = 'password';
        });
        togglePasswordBtn.addEventListener('mouseleave', () => {
          passwordInput.type = 'password';
        });
        togglePasswordBtn.addEventListener('touchstart', (e) => {
          e.preventDefault();
          passwordInput.type = 'text';
        });
        togglePasswordBtn.addEventListener('touchend', () => {
          passwordInput.type = 'password';
        });
      }
    }

    document.addEventListener('DOMContentLoaded', function() {
      const confirmBtn = document.getElementById('confirmBtn');
      const cancelBtn = document.getElementById('cancelBtn');
      const refreshBtn = document.getElementById('refreshBtn');
      if (confirmBtn) confirmBtn.addEventListener('click', connectToWifi);
      if (cancelBtn) cancelBtn.addEventListener('click', hideModal);
      if (refreshBtn) refreshBtn.addEventListener('click', scanWifi);
      
      setupPasswordToggle();
      
      scanWifi();
    });
)js";
#endif