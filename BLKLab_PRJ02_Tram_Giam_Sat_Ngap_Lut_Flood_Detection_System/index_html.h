const char index_html[] PROGMEM = R"rawliteral(

<!DOCTYPE html>
<html>
<meta charset="utf-8">
  <head>
    <title>🍀 Hệ thống cảnh báo lũ lụt 🌌</title>
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
      font-size: 30px;
      height : 85px;
    }

    .submit {
      width: 100%;
      background-color:#04AA6D;
      color: black;
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
      justify-content: space-between;
      gap : 10px;
    }

    h1 {
      text-align: center;
      margin-bottom: 40px;
      font-size: 50px;
    }
    h2 {
      font-size: 40px;
    }
    h4 {
      font-size: 30px;
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
      <h1 >🍀 Hệ thống cảnh báo lũ lụt</h1>
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

          <h2>🎲Cấu hình cảm biến siêu âm</h2>
          <div class="container-2">
            <h4>Chiều cao lắp cảm biến (cm)</h4>
            <input  type="number" id="heightInstallSensor" name="heightInstallSensor" min="1" placeholder="cm">
          </div>
          <div class="container-2">
            <h4>Ngưỡng cảnh báo (cm)</h4>
            <input  type="number" id="thresholdWarning" name="thresholdWarning" min="1" placeholder="cm">
          </div>
          <div class="container-2">
            <button class="submit" id="btnDefauld"><h4 style="font-size: 30px;"> Chọn mặc định</h4></button>
            <button class="submit" id="btnSubmit"><h4 style="font-size: 30px;">Gửi</h4></button>
          </div>
        </div>
      </div>
  </div>

  <script>
    var data = {
          ssid   : "",
          pass   : "",
          token : "",
          timeOpenDoor : "",
          numberEnterWrong : "",

    };
    const ssid   = document.getElementById("ssid");
    const pass   = document.getElementById("pass");
    const token = document.getElementById("token");

    const btnDefauld = document.getElementById("btnDefauld");
    const heightInstallSensor =  document.getElementsByName('heightInstallSensor')[0];
    const thresholdWarning =  document.getElementsByName('thresholdWarning')[0];


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
          heightInstallSensor.value = obj.heightInstallSensor;
          thresholdWarning.value = obj.thresholdWarning;
      }
    }

    // Add data mặc định vào textbox khi click vào nút btnDefauld
    btnDefauld.addEventListener("click", function(event) {
        heightInstallSensor.value = 200;
        thresholdWarning.value = 20;
        
    });

    var xhttp2 = new XMLHttpRequest();
    const btnSubmit = document.getElementById("btnSubmit"); 
    btnSubmit.addEventListener('click', () => { 
        data = {
            ssid   : ssid.value,
            pass   : pass.value,
            token :  token.value,
            heightInstallSensor : Number(heightInstallSensor.value),
            thresholdWarning : Number(thresholdWarning.value),
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

