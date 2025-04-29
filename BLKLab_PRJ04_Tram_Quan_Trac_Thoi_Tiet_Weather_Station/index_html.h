const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<meta charset="utf-8">
  <head>
    <title>🌿 TRẠM THEO DÕI THỜI TIẾT 🌿</title>
    <style>
      body {
        background-color: rgb(241, 241, 241);
        padding: 10px;
      }
      .container {     
        display: flex;
        justify-content: center; 
        align-items: center; 
        flex-direction: column;
        padding-top: 10px;
        font-family: Monospace;
        
      }
      .button {
        background-color: #04AA6D; /* Green */
        border: none;
        color: white;
        padding: 15px 32px;
        text-align: center;
        text-decoration: none;
        display: inline-block;
        font-size: x-large;
        margin: 10px 2px;
        cursor: pointer;
        width: 30%;
        border-radius: 5px;
      }
      input[type=text], select {
        width: 100%;
        padding: 12px 20px;
        margin: 8px 0;
        display: inline-block;
        border: 1px solid #ccc;
        border-radius: 4px;
        box-sizing: border-box;
        font-size: xx-large;
        height : 85px;
      }
      input[type=number] {
        width: 30%;
        padding: 12px 20px;
        margin: 8px 2px;
        display: inline-block;
        border: 1px solid #ccc;
        border-radius: 4px;
        box-sizing: border-box;
        font-size: xx-large;
        height : 85px;
      }

      .submit {
        width: 100%;
        background-color:#04AA6D;
        color: white;
        padding: 14px 20px;
        margin: 8px 0;
        border: none;
        border-radius: 4px;
        cursor: pointer;
      }

      .submit:hover, .button:hover {
        background-color: #989b98;
      }
      .container-2 {
        display: flex;
        margin-bottom: 20px;
        justify-content: start;
        gap: 10px;
      }

      h1 {
        text-align: center;
        margin-bottom: 40px;
      }
      @media (min-width: 300px) and (max-width: 900px) {
        .container {     
          margin-right: 0%;
          margin-left: 0%;
        }
      }
    
    </style>
  </head>

 <body>
  <div class="container">
      <h1>🌿 TRẠM THEO DÕI THỜI TIẾT 🌿</h1>
      <div>
        <hr>
        <h2>Cấu hình WIFI</h2>
        <div>
          <h4>Tên WIFI </h4>
          <input type="text" id="ssid" name="ssid" placeholder="Your ssid..">
      
          <h4>Mật khẩu</h4>
          <input type="text" id="pass" name="pass" placeholder="Your password ..">

          <h4>Mã Token Blynk</h4>
          <input type="text" id="token" name="token" placeholder="Your Token Blynk ..">
          
          <hr>

          <h2>🔥Cấu hình cảm biến nhiệt độ, độ ẩm💧</h2>
          <h4>Ngưỡng nhiệt độ môi trường (*C)</h4>
          <p style="font-size: 10px;">🌱Ngưỡng 1 &lt; Khoảng an toàn &lt; Ngưỡng 2</p>
     

          <div class="container-2">
            <input  type="number" id="tempThreshold1" name="tempThreshold1" min="10" max="100" step="1" placeholder="Ngưỡng 1">
            <input  type="number" id="tempThreshold2" name="tempThreshold2" min="10" max="100" step="1" placeholder="Ngưỡng 2">  

          </div>
          
          <h4>Ngưỡng độ ẩm không khí (%)</h4>
          <p style="font-size: 10px;">🌱Ngưỡng 1 &lt; Khoảng an toàn &lt; Ngưỡng 2</p>
 
          <div class="container-2">
            <input  type="number" id="humiThreshold1" name="humiThreshold1" min="10" max="100" step="1"  placeholder="Ngưỡng 1">
            <input  type="number" id="humiThreshold2" name="humiThreshold2" min="10" max="100" step="1"  placeholder="Ngưỡng 2">  

          </div>
          
          <hr>
          <h2>🌧 Cấu hình cảm biến mưa</h2>
          <h4>Ngưỡng cảm biến mưa (mm/h)</h4>
          <p style="font-size: 10px;">🌱Khoảng an toàn &lt; Ngưỡng 1(Mưa nhỏ) &lt; Ngưỡng 2(Mưa to)</p>
   
          <div class="container-2">
            <input  type="number" id="rainThreshold1" name="rainThreshold1" min="0"  placeholder="Ngưỡng 1">
            <input  type="number" id="rainThreshold2" name="rainThreshold2" min="0"  placeholder="Ngưỡng 2">  

          </div>

          <h4>⏰Thời gian lấy mẫu (phút)</h4>
          <div class="container-2">
            <input  type="number" id="rainTimeSample" name="rainTimeSample" min="0"  placeholder="Phút">
          </div>
          <h4>🔵Diện tích phễu hứng (mm2)</h4>
          <p style="font-size: 10px;">Diện tích S = 2*PI*R^2</p>
          <div class="container-2">
            <input  type="number" id="rainSFunnel" name="rainSFunnel" min="0"  placeholder="mm2">
          </div>
          <h4>🧪Lượng nước để lật phễu đo (ml)</h4>
          <div class="container-2">
            <input  type="number" id="rainAmountOfWater" name="rainAmountOfWater" min="0"  placeholder="ml">
          </div>

          <hr>
          <h2>💨Cấu hình cảm biến gió</h2>
          <h4>Ngưỡng cảm biến gió (m/s)</h4>
          <p style="font-size: 10px;">🌱Khoảng an toàn &lt; Ngưỡng 1(Gió nhỏ) &lt; Ngưỡng 2(Gió to)</p>
   
          <div class="container-2">
            <input  type="number" id="windThreshold1" name="windThreshold1" min="0"  placeholder="Ngưỡng 1">
            <input  type="number" id="windThreshold2" name="windThreshold2" min="0"  placeholder="Ngưỡng 2">  
          </div>
          <h4>⏰Thời gian lấy mẫu (giây)</h4>
          <div class="container-2">
            <input  type="number" id="windTimeSample" name="windTimeSample" min="0"  placeholder="Giây">
          </div>
          <h4>🟡Đường kính bộ cánh hứng gió (mm)</h4>
          <div class="container-2">
            <input  type="number" id="windDAnemometer" name="windDAnemometer" min="0"  placeholder="mm">
          </div>

           <div class="container-2">
            <button class="submit" id="btnDefauld"><h4 style="font-size: 35px;"> Chọn mặc định</h4></button>
            <button class="submit" id="btnSubmit"><h4 style="font-size: 35px;">Gửi</h4></button>
          </div>
        </div>
      </div>
      <div>
         
      </div>
    
  </div>

  <script>
    var data = {
          ssid   : "",
          pass   : "",
          token : "",
          tempThreshold1 : "",
          tempThreshold2 : "",
          humiThreshold1 : "",
          humiThreshold2 : "",
          rainThreshold1 : "",
          rainThreshold2 : "",
          windThreshold1 : "",
          windThreshold2 : "",
          rainTimeSample : "",
          windTimeSample : "",
    };
    const ssid   = document.getElementById("ssid");
    const pass   = document.getElementById("pass");
    const token = document.getElementById("token");

    const btnDefauld = document.getElementById("btnDefauld");
    const tempThreshold1 =  document.getElementsByName('tempThreshold1')[0];
    const tempThreshold2 =  document.getElementsByName('tempThreshold2')[0];

    const humiThreshold1 = document.getElementsByName('humiThreshold1')[0];
    const humiThreshold2 = document.getElementsByName('humiThreshold2')[0];

    const rainThreshold1 = document.getElementsByName('rainThreshold1')[0];
    const rainThreshold2 = document.getElementsByName('rainThreshold2')[0];

    const windThreshold1 = document.getElementsByName('windThreshold1')[0];
    const windThreshold2 = document.getElementsByName('windThreshold2')[0];

    const rainTimeSample = document.getElementsByName('rainTimeSample')[0];
    const windTimeSample = document.getElementsByName('windTimeSample')[0];

    const rainSFunnel       = document.getElementsByName('rainSFunnel')[0];
    const rainAmountOfWater = document.getElementsByName('rainAmountOfWater')[0];
    const windDAnemometer   = document.getElementsByName('windDAnemometer')[0];
    


      var xhttp = new XMLHttpRequest();
      xhttp.open("GET","/data_before", true),
      xhttp.send();
      xhttp.onreadystatechange = function() {
        if(xhttp.readyState == 4 && xhttp.status == 200) {
          //alert(this.responseText);
          const obj    = JSON.parse(this.responseText); // chuyển JSON sang JS Object
          //alert(obj.ssid);
          ssid.value   = obj.ssid;
          pass.value   = obj.pass;
          token.value = obj.token;
          tempThreshold1.value = obj.tempThreshold1;
          tempThreshold2.value = obj.tempThreshold2;
          humiThreshold1.value = obj.humiThreshold1;
          humiThreshold2.value = obj.humiThreshold2;
          rainThreshold1.value = obj.rainThreshold1;
          rainThreshold2.value = obj.rainThreshold2;
          windThreshold1.value = obj.windThreshold1;
          windThreshold2.value = obj.windThreshold2;
          rainTimeSample.value = obj.rainTimeSample;
          windTimeSample.value = obj.windTimeSample;
          rainSFunnel.value       = obj.rainSFunnel;
          rainAmountOfWater.value = obj.rainAmountOfWater;
          windDAnemometer.value   = obj.windDAnemometer;
        }
      }

      // Add data mặc định vào textbox khi click vào nút btnDefauld
      btnDefauld.addEventListener("click", function(event) {
          tempThreshold1.value = 20;
          tempThreshold2.value = 32;
          humiThreshold1.value = 40;
          humiThreshold2.value = 75;
          rainThreshold1.value = 10;
          rainThreshold2.value = 30;
          windThreshold1.value = 1;
          windThreshold2.value = 5;
          rainTimeSample.value = 5;
          windTimeSample.value = 5;
          rainSFunnel.value       = 9498;
          rainAmountOfWater.value = 6;
          windDAnemometer.value   = 208;
      });

      var xhttp2 = new XMLHttpRequest();
      const btnSubmit = document.getElementById("btnSubmit"); 
      btnSubmit.addEventListener('click', () => { 
          data = {
            ssid   : ssid.value,
            pass   : pass.value,
            token :  token.value, 
            tempThreshold1 : Number(tempThreshold1.value),
            tempThreshold2 : Number(tempThreshold2.value),
            humiThreshold1 : Number(humiThreshold1.value),
            humiThreshold2 : Number(humiThreshold2.value),
            rainThreshold1 : Number(rainThreshold1.value),
            rainThreshold2 : Number(rainThreshold2.value),
            windThreshold1 : Number(windThreshold1.value),
            windThreshold2 : Number(windThreshold2.value),
            rainTimeSample : Number(rainTimeSample.value),
            windTimeSample : Number(windTimeSample.value),
            rainSFunnel      : Number(rainSFunnel.value),
            rainAmountOfWater: Number(rainAmountOfWater.value),
            windDAnemometer  : Number(windDAnemometer.value)
          }
          
          xhttp2.open("POST","/post_data", true),
          xhttp2.send(JSON.stringify(data)); // chuyển JSObject sang JSON để gửi về server
          xhttp2.onreadystatechange = function() {
            if(xhttp2.readyState == 4 && xhttp2.status == 200) {
              alert("Cài đặt thành công");
            } 
          }
      });

    </script>
    
  </body>

</html>



)rawliteral";

