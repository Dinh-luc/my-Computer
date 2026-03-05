#ifndef _WEB_UI_H
#define _WEB_UI_H

#include <Arduino.h>

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Mini OS Dashboard</title>
    <style>
        :root {
            --bg-color: #121212; --panel-bg: #1e1e1e; --primary: #f39c12;
            --text-main: #ffffff; --text-sub: #aaaaaa; --border: #333333;
        }
        body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background-color: var(--bg-color); color: var(--text-main); margin: 0; padding: 20px; display: flex; justify-content: center; }
        .container { width: 100%; max-width: 600px; background-color: var(--panel-bg); border-radius: 12px; overflow: hidden; box-shadow: 0 8px 16px rgba(0,0,0,0.5); }
        .header { text-align: center; padding: 20px; border-bottom: 1px solid var(--border); }
        .header h1 { margin: 0; color: var(--primary); font-size: 24px; }
        .header p { margin: 5px 0 0; color: var(--text-sub); font-size: 14px; }
        
        .tabs { display: flex; border-bottom: 1px solid var(--border); }
        .tab-btn { flex: 1; padding: 15px; background: none; border: none; color: var(--text-sub); font-size: 16px; cursor: pointer; transition: 0.3s; outline: none; }
        .tab-btn:hover { background-color: rgba(255,255,255,0.05); }
        .tab-btn.active { color: var(--primary); border-bottom: 3px solid var(--primary); font-weight: bold; }
        
        .tab-content { padding: 30px 20px; display: none; }
        .tab-content.active { display: block; }
        
        .upload-area { border: 2px dashed var(--border); border-radius: 8px; padding: 30px 20px; text-align: center; margin-bottom: 15px; transition: 0.3s; cursor: pointer; }
        .upload-area:hover { border-color: var(--primary); }
        .upload-area input[type="file"] { display: none; }
        
        .btn { background-color: var(--primary); color: #000; border: none; padding: 12px 25px; border-radius: 6px; font-size: 16px; font-weight: bold; cursor: pointer; width: 100%; transition: 0.3s; }
        .btn:hover { opacity: 0.9; }
        .btn:disabled { background-color: var(--border); color: var(--text-sub); cursor: not-allowed; }
        
        .progress-container { width: 100%; background-color: var(--border); border-radius: 6px; margin-top: 15px; overflow: hidden; display: none; }
        .progress-bar { height: 10px; background-color: var(--primary); width: 0%; transition: width 0.2s; }
        .status { margin-top: 10px; font-size: 14px; text-align: center; color: var(--text-sub); }
        .warning { color: #e74c3c; font-size: 13px; margin-top: 15px; text-align: center; }
        
        #preview { max-width: 160px; max-height: 128px; margin: 10px auto; display: none; border: 1px solid var(--border); border-radius: 4px;}
        
        /* Style cho phần cấu hình ảnh mới */
        .img-options { background-color: #2a2a2a; padding: 15px; border-radius: 8px; margin-bottom: 20px; display: none; text-align: left; }
        .img-options label { cursor: pointer; display: block; margin-bottom: 10px; font-size: 15px; }
        .custom-size { margin-top: 10px; padding-left: 25px; display: none; }
        .custom-size input { width: 60px; padding: 6px; background: #121212; border: 1px solid var(--border); color: white; border-radius: 4px; text-align: center;}
    </style>
</head>
<body>

<div class="container">
    <div class="header">
        <h1>Mini OS Sync</h1>
        <p>Kết nối máy tính & ESP32-S3</p>
    </div>

    <div class="tabs">
        <button class="tab-btn active" onclick="switchTab('fileTab')">File & Hình ảnh</button>
        <button class="tab-btn" onclick="switchTab('otaTab')">Cập nhật OTA</button>
    </div>

    <div id="fileTab" class="tab-content active">
        <div class="upload-area" onclick="document.getElementById('fileInput').click()">
            <p id="fileLabel" style="font-weight: bold; font-size: 18px; margin: 0 0 5px 0;">Nhấn hoặc Kéo thả File vào đây</p>
            <p style="font-size: 13px; color: var(--text-sub); margin: 0;">Hỗ trợ: JPG, PNG, MP3, WAV</p>
            <input type="file" id="fileInput" accept="image/jpeg, image/png, audio/mpeg, audio/wav">
            <img id="preview" alt="Preview">
        </div>

        <div id="imgOptionsBlock" class="img-options">
            <label>
                <input type="radio" name="imgMode" value="wallpaper" checked onchange="toggleSizeOptions()"> 
                <span id="wallLabel">Hình nền (Tự động Crop - File JPG)</span>
            </label>
            <label>
                <input type="radio" name="imgMode" value="icon" onchange="toggleSizeOptions()"> 
                Icon / Ảnh tùy chỉnh (Giữ nền trong suốt - File PNG)
            </label>
            <div id="customSizeInputs" class="custom-size">
                Rộng: <input type="number" id="iconW" value="32"> px 
                &nbsp;&nbsp;x&nbsp;&nbsp;
                Cao: <input type="number" id="iconH" value="32"> px
            </div>
        </div>

        <button class="btn" id="btnUploadFile" onclick="uploadFile()">Gửi vào Thẻ nhớ</button>
        <div class="progress-container" id="fileProgressCont"><div class="progress-bar" id="fileProgressBar"></div></div>
        <div class="status" id="fileStatus"></div>
    </div>

    <div id="otaTab" class="tab-content">
        <div class="upload-area" onclick="document.getElementById('otaInput').click()">
            <p id="otaLabel" style="font-weight: bold; font-size: 16px;">Chọn file Firmware (.bin)</p>
            <input type="file" id="otaInput" accept=".bin">
        </div>
        <button class="btn" id="btnUploadOTA" onclick="uploadOTA()">Cập nhật Hệ thống</button>
        <div class="progress-container" id="otaProgressCont"><div class="progress-bar" id="otaProgressBar"></div></div>
        <div class="status" id="otaStatus"></div>
        <div class="warning">CẢNH BÁO: Không tắt nguồn ESP32 trong quá trình cập nhật!</div>
    </div>
</div>

<script>
    // Khởi tạo biến kích thước (Mặc định là 160x128, sẽ được cập nhật ngay khi load)
    let SYS_WIDTH = 160;
    let SYS_HEIGHT = 128;

    // Tự động gọi API lấy kích thước màn hình ngay khi mở Web
    window.onload = function() {
        fetch('/sysinfo')
            .then(response => response.json())
            .then(data => {
                SYS_WIDTH = data.w;
                SYS_HEIGHT = data.h;
                document.getElementById('wallLabel').innerHTML = `Hình nền (Tự động Crop <b>${SYS_WIDTH}x${SYS_HEIGHT} px</b> - File JPG)`;
            })
            .catch(err => console.error("Lỗi lấy cấu hình:", err));
    };

    function switchTab(tabId) {
        document.querySelectorAll('.tab-content').forEach(tab => tab.classList.remove('active'));
        document.querySelectorAll('.tab-btn').forEach(btn => btn.classList.remove('active'));
        document.getElementById(tabId).classList.add('active');
        event.currentTarget.classList.add('active');
    }

    function toggleSizeOptions() {
        const mode = document.querySelector('input[name="imgMode"]:checked').value;
        document.getElementById('customSizeInputs').style.display = (mode === 'icon') ? 'block' : 'none';
    }

    document.getElementById('fileInput').addEventListener('change', function(e) {
        const file = e.target.files[0];
        if (!file) return;
        document.getElementById('fileLabel').innerText = file.name;
        
        const optionsBlock = document.getElementById('imgOptionsBlock');
        const previewImg = document.getElementById('preview');

        if (file.type.startsWith('image/')) {
            optionsBlock.style.display = 'block';
            previewImg.src = URL.createObjectURL(file); 
            previewImg.style.display = 'block';
        } else {
            optionsBlock.style.display = 'none';
            previewImg.style.display = 'none';
        }
    });

    document.getElementById('otaInput').addEventListener('change', function(e) {
        if (e.target.files[0]) document.getElementById('otaLabel').innerText = e.target.files[0].name;
    });

    function uploadFile() {
        const fileInput = document.getElementById('fileInput');
        if (fileInput.files.length === 0) return alert("Vui lòng chọn file!");
        
        const file = fileInput.files[0];
        const stat = document.getElementById('fileStatus');
        
        if (file.type.startsWith('image/')) {
            stat.innerText = "Đang tối ưu ảnh...";
            stat.style.color = "#f39c12"; 
            
            const mode = document.querySelector('input[name="imgMode"]:checked').value;
            
            const img = new Image();
            img.onload = function() {
                const canvas = document.createElement('canvas');
                const ctx = canvas.getContext('2d');
                
                let targetW = SYS_WIDTH;
                let targetH = SYS_HEIGHT;
                let exportMime = 'image/jpeg';
                let exportName = file.name;
                
                if (mode === 'wallpaper') {
                    canvas.width = targetW;
                    canvas.height = targetH;
                    ctx.imageSmoothingEnabled = true;
                    ctx.imageSmoothingQuality = 'high';
                    
                    const ratio = Math.max(targetW / img.width, targetH / img.height);
                    const drawW = img.width * ratio;
                    const drawH = img.height * ratio;
                    ctx.drawImage(img, (targetW - drawW) / 2, (targetH - drawH) / 2, drawW, drawH);
                    
                    exportName = exportName.replace(/\.[^/.]+$/, "") + ".jpg";

                } else {
                    targetW = parseInt(document.getElementById('iconW').value) || img.width;
                    targetH = parseInt(document.getElementById('iconH').value) || img.height;
                    exportMime = 'image/png'; 
                    
                    canvas.width = targetW;
                    canvas.height = targetH;
                    ctx.clearRect(0, 0, targetW, targetH);
                    ctx.drawImage(img, 0, 0, targetW, targetH);

                    exportName = exportName.replace(/\.[^/.]+$/, "") + ".png";
                }

                canvas.toBlob(function(blob) {
                    URL.revokeObjectURL(img.src); 
                    sendToServer(blob, exportName, '/upload', 'fileProgressCont', 'fileProgressBar', 'fileStatus', 'btnUploadFile');
                }, exportMime, 0.90); 
            };
            img.src = URL.createObjectURL(file);
        } else {
            sendToServer(file, file.name, '/upload', 'fileProgressCont', 'fileProgressBar', 'fileStatus', 'btnUploadFile');
        }
    }

    function uploadOTA() {
        const fileInput = document.getElementById('otaInput');
        if (fileInput.files.length === 0) return alert("Vui lòng chọn file .bin!");
        sendToServer(fileInput.files[0], "update.bin", '/update', 'otaProgressCont', 'otaProgressBar', 'otaStatus', 'btnUploadOTA');
    }

    function sendToServer(fileData, filename, url, contId, barId, statId, btnId) {
        const btn = document.getElementById(btnId);
        btn.disabled = true;
        document.getElementById(contId).style.display = 'block';
        const bar = document.getElementById(barId);
        const stat = document.getElementById(statId);
        
        const formData = new FormData();
        formData.append("file", fileData, filename);

        const xhr = new XMLHttpRequest();
        xhr.open("POST", url, true);

        xhr.upload.addEventListener("progress", function(evt) {
            if (evt.lengthComputable) {
                let percentComplete = Math.round((evt.loaded / evt.total) * 100);
                bar.style.width = percentComplete + "%";
                stat.innerText = "Đang truyền: " + percentComplete + "%";
            }
        });

        xhr.onload = function() {
            btn.disabled = false;
            if (xhr.status == 200) {
                stat.innerText = xhr.responseText || "Thành công!";
                stat.style.color = "#2ecc71";
                if (url === '/update') stat.innerText = "Thành công! Thiết bị đang khởi động lại...";
            } else {
                stat.innerText = "Lỗi: " + xhr.statusText;
                stat.style.color = "#e74c3c";
            }
        };

        xhr.onerror = function() {
            btn.disabled = false;
            stat.innerText = "Mất kết nối!";
            stat.style.color = "#e74c3c";
        };

        xhr.send(formData);
    }
</script>
</body>
</html>
)rawliteral";

#endif