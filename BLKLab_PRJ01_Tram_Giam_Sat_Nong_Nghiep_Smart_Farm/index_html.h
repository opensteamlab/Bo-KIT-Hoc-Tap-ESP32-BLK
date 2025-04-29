const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<meta charset="utf-8">

<head>
  <title>🌿HỆ THỐNG VƯỜN THÔNG MINH🌿</title>
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
    input[type=text], select ,#caytrong{
      background-color: white;
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
    input[type=number].tempThre , input[type=number].humiThre , 
    input[type=number].soilThre {
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
      gap: 10px;
      margin-bottom: 20px;
    }

    h1 {
      text-align: center;
      margin-bottom: 20px;
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
      <h1>🌿HỆ THỐNG GIÁM SÁT VƯỜN THÔNG MINH🌿</h1>
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

          <h2>Cấu hình ngưỡng</h2>

          <h4>Chọn loại cây trồng :</h4>
          <select name="caytrong" id="caytrong" onchange="checkUserSelected()">
            <option value="0">Tự cài đặt</option>
            <option value="1">Rau mầm</option>
            <option value="2">Bắp cải</option>
            <option value="3">Cà chua</option>
            <option value="4">Xà lách</option>
            <option value="5">Dưa chuột</option>
            <option value="6">Sen đá</option>
            <option value="7">Rau mùi</option>
            <!-- <option value="c++" disabled>C++</option>
            <option value="java" selected>Java</option> -->
          </select>
          <h4>Ngưỡng nhiệt độ môi trường (*C) </h4>
          <p style="font-size: 10px;">🌞Ngưỡng 1 &lt; Khoảng an toàn &lt; Ngưỡng 2</p>
     

          <div class="container-2">
            <input class="tempThre" type="number" id="tempThreshold1" name="tempThreshold1" min="10" max="100" step="1" placeholder="Ngưỡng 1">
            <input class="tempThre" type="number" id="tempThreshold2" name="tempThreshold2" min="10" max="100" step="1" placeholder="Ngưỡng 2">  
          </div>
          
 
          <h4>Ngưỡng độ ẩm không khí (%)</h4>
          <p style="font-size: 10px;">🌱Ngưỡng 1 &lt; Khoảng an toàn &lt; Ngưỡng 2</p>
 
          <div class="container-2">
            <input class="humiThre" type="number" id="humiThreshold1" name="humiThreshold1"  min="10" max="100" step="1" placeholder="Ngưỡng 1">
            <input class="humiThre" type="number" id="humiThreshold2" name="humiThreshold2"  min="10" max="100" step="1"  placeholder="Ngưỡng 2">  
    
          </div>
          


          <h4>Ngưỡng độ ẩm đất (%)</h4>
          <p style="font-size: 10px;">🍁Ngưỡng 1 &lt; Khoảng an toàn &lt; Ngưỡng 2</p>
   
          <div class="container-2">
            <input class="soilThre" type="number" id="soilMoistureThreshold1" name="soilMoistureThreshold1"  placeholder="Ngưỡng 1">
            <input class="soilThre" type="number" id="soilMoistureThreshold2" name="soilMoistureThreshold2"  placeholder="Ngưỡng 2">  
          </div>

      
          <button class="submit" id="btnSubmit"><h4 style="font-size: 35px;">Gửi</h4></button>
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
          typePlant : "",
          tempThreshold1 : "",
          tempThreshold2 : "",
          humiThreshold1 : "",
          humiThreshold2 : "",
          soilMoistureThreshold1 : "",
          soilMoistureThreshold2 : "",
    };
     const ssid      = document.getElementById("ssid");
     const pass      = document.getElementById("pass");
     const token     = document.getElementById("token");
     const typePlant = document.getElementById("caytrong");

    const tempThreshold1 =  document.getElementsByName('tempThreshold1')[0];
    const tempThreshold2 =  document.getElementsByName('tempThreshold2')[0];

    const humiThreshold1 = document.getElementsByName('humiThreshold1')[0];
    const humiThreshold2 = document.getElementsByName('humiThreshold2')[0];

    const soilMoistureThreshold1 = document.getElementsByName('soilMoistureThreshold1')[0];
    const soilMoistureThreshold2 = document.getElementsByName('soilMoistureThreshold2')[0];

    // Khởi tạo các giá trị ngưỡng của thực vật
    function PLANT(tempThreshold1, tempThreshold2, humiThreshold1, humiThreshold2, soilMoistureThreshold1, soilMoistureThreshold2) {
        this.tempThreshold1 = tempThreshold1;
        this.tempThreshold2 = tempThreshold2;
        this.humiThreshold1 = humiThreshold1;
        this.humiThreshold2 = humiThreshold2;
        this.soilMoistureThreshold1 = soilMoistureThreshold1;
        this.soilMoistureThreshold2 = soilMoistureThreshold2;
    }
    raumam   = new PLANT(20,25,50,70,60,80);
    bapcai   = new PLANT(15,21,60,80,60,80);
    cachua   = new PLANT(15,25,60,80,60,80);
    xalach   = new PLANT(15,20,60,80,60,80);
    duachuot = new PLANT(20,25,60,80,60,80);
    senda    = new PLANT(15,25,30,50,30,50);
    raumui   = new PLANT(15,20,50,70,60,80);
    
    // lấy data ban đầu
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
        token.value  = obj.token;
        typePlant.value = obj.typePlant;
        tempThreshold1.value = obj.tempThreshold1;
        tempThreshold2.value = obj.tempThreshold2;
        humiThreshold1.value = obj.humiThreshold1;
        humiThreshold2.value = obj.humiThreshold2;
        soilMoistureThreshold1.value = obj.soilMoistureThreshold1;
        soilMoistureThreshold2.value = obj.soilMoistureThreshold2;

        // Gán giá trị cây trồng ban đầu
        document.getElementById("caytrong").options.selectedIndex = Number(obj.typePlant);
        checkPlantAndPutValue(Number(obj.typePlant));
      }
    }

   
    
    // Hàm check kiểu cây trồng
    function checkUserSelected() {
      var index = typePlant.options.selectedIndex;
      checkPlantAndPutValue(index);
    }

    // hiển thị các giá trị ngưỡng tương ứng lên textbox khi người dùng chọn loại cây trồng
    function checkPlantAndPutValue(value) {
        if(value === 0) {
          document.getElementById("tempThreshold1").disabled = false;
          document.getElementById("tempThreshold2").disabled = false;
          document.getElementById("humiThreshold1").disabled = false;
          document.getElementById("humiThreshold2").disabled = false;
          document.getElementById("soilMoistureThreshold1").disabled = false;
          document.getElementById("soilMoistureThreshold2").disabled = false;
        } else {
          document.getElementById("tempThreshold1").disabled = true;
          document.getElementById("tempThreshold2").disabled = true;
          document.getElementById("humiThreshold1").disabled = true;
          document.getElementById("humiThreshold2").disabled = true;
          document.getElementById("soilMoistureThreshold1").disabled = true;
          document.getElementById("soilMoistureThreshold2").disabled = true;
          switch (value) {
            case 1:
              tempThreshold1.value = raumam.tempThreshold1;
              tempThreshold2.value = raumam.tempThreshold2;
              humiThreshold1.value = raumam.humiThreshold1;
              humiThreshold2.value = raumam.humiThreshold2;
              soilMoistureThreshold1.value = raumam.soilMoistureThreshold1;
              soilMoistureThreshold2.value = raumam.soilMoistureThreshold2;
              break;
            case 2:
              tempThreshold1.value = bapcai.tempThreshold1;
              tempThreshold2.value = bapcai.tempThreshold2;
              humiThreshold1.value = bapcai.humiThreshold1;
              humiThreshold2.value = bapcai.humiThreshold2;
              soilMoistureThreshold1.value = bapcai.soilMoistureThreshold1;
              soilMoistureThreshold2.value = bapcai.soilMoistureThreshold2;
              break;
            case 3:
              tempThreshold1.value = cachua.tempThreshold1;
              tempThreshold2.value = cachua.tempThreshold2;
              humiThreshold1.value = cachua.humiThreshold1;
              humiThreshold2.value = cachua.humiThreshold2;
              soilMoistureThreshold1.value = cachua.soilMoistureThreshold1;
              soilMoistureThreshold2.value = cachua.soilMoistureThreshold2;
              break;
            case 4:
              tempThreshold1.value = xalach.tempThreshold1;
              tempThreshold2.value = xalach.tempThreshold2;
              humiThreshold1.value = xalach.humiThreshold1;
              humiThreshold2.value = xalach.humiThreshold2;
              soilMoistureThreshold1.value = xalach.soilMoistureThreshold1;
              soilMoistureThreshold2.value = xalach.soilMoistureThreshold2;
              break;
            case 5:
              tempThreshold1.value = duachuot.tempThreshold1;
              tempThreshold2.value = duachuot.tempThreshold2;
              humiThreshold1.value = duachuot.humiThreshold1;
              humiThreshold2.value = duachuot.humiThreshold2;
              soilMoistureThreshold1.value = duachuot.soilMoistureThreshold1;
              soilMoistureThreshold2.value = duachuot.soilMoistureThreshold2;
              break;
            case 6:
              tempThreshold1.value = senda.tempThreshold1;
              tempThreshold2.value = senda.tempThreshold2;
              humiThreshold1.value = senda.humiThreshold1;
              humiThreshold2.value = senda.humiThreshold2;
              soilMoistureThreshold1.value = senda.soilMoistureThreshold1;
              soilMoistureThreshold2.value = senda.soilMoistureThreshold2;
              break;
            case 7:
              tempThreshold1.value = raumui.tempThreshold1;
              tempThreshold2.value = raumui.tempThreshold2;
              humiThreshold1.value = raumui.humiThreshold1;
              humiThreshold2.value = raumui.humiThreshold2;
              soilMoistureThreshold1.value = raumui.soilMoistureThreshold1;
              soilMoistureThreshold2.value = raumui.soilMoistureThreshold2;
              break;
          }
        }
    }

    // Gửi data về server khi click vào nút Submit
    var xhttp2 = new XMLHttpRequest();
    const btnSubmit = document.getElementById("btnSubmit"); 
    btnSubmit.addEventListener('click', () => { 
        data = {
          ssid   : ssid.value,
          pass   : pass.value,
          token  :  token.value,
          typePlant :  Number(typePlant.value),
          tempThreshold1 : Number(tempThreshold1.value),
          tempThreshold2 : Number(tempThreshold2.value),
          humiThreshold1 : Number(humiThreshold1.value),
          humiThreshold2 : Number(humiThreshold2.value),
          soilMoistureThreshold1 : Number(soilMoistureThreshold1.value),
          soilMoistureThreshold2 : Number(soilMoistureThreshold2.value)
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

