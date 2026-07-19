#ifndef WEBUI_H
#define WEBUI_H

#include <Arduino.h>

const char WEB_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>

<head>

<meta charset="UTF-8">

<meta name="viewport"
      content="width=device-width,initial-scale=1">

<title>ESP32 PDF Server</title>

<style>

body
{
    margin:0;
    background:#f4f6f8;
    font-family:Arial,Helvetica,sans-serif;
}

.container
{
    width:90%;
    max-width:700px;

    margin:auto;

    margin-top:40px;

    background:white;

    padding:30px;

    border-radius:10px;

    box-shadow:0px 0px 15px rgba(0,0,0,.15);
}

h2
{
    text-align:center;
}

.status
{
    margin-top:20px;

    font-size:18px;
}

.progressBox
{
    width:100%;

    height:28px;

    background:#ddd;

    border-radius:8px;

    overflow:hidden;

    margin-top:20px;
}

.progressBar
{
    width:0%;

    height:100%;

    background:#2ecc71;

    transition:.2s;

    text-align:center;

    color:white;

    line-height:28px;
}

.info
{
    margin-top:20px;

    font-size:15px;
}

.info table
{
    width:100%;
}

.info td
{
    padding:6px;
}

button
{
    width:100%;

    padding:15px;

    margin-top:25px;

    border:none;

    border-radius:8px;

    background:#2196F3;

    color:white;

    font-size:18px;

    cursor:pointer;
}

button:hover
{
    background:#1976D2;
}

.footer
{
    margin-top:20px;

    text-align:center;

    color:gray;
}

</style>

</head>

<body>

<div class="container">

<h2>ECG Report Access Point</h2>

<button onclick="downloadReport()">

Generate PDF

</button>

</div>

<script>

function downloadReport()
{
    window.open("/download","_blank");
}

async function updateProgress()
{
    try
    {
        let response=
        await fetch("/progress");

        let data=
        await response.json();
        
    }
    catch(e)
    {
        console.log(e);
    }
}

updateProgress();

</script>

</body>

</html>

)rawliteral";

//setInterval(updateProgress,200);

/*   
        document.getElementById("state").innerHTML=
        data.state;

        document.getElementById("total").innerHTML=
        data.total;

        document.getElementById("received").innerHTML=
        data.received;

        document.getElementById("sent").innerHTML=
        data.sent;

        document.getElementById("speed").innerHTML=
        data.speed+" Bytes/s";

        document.getElementById("elapsed").innerHTML=
        data.elapsed+" s";

        document.getElementById("remaining").innerHTML=
        data.remaining+" s";

        let bar=
        document.getElementById("bar");

        bar.style.width=
        data.progress+"%";

        bar.innerHTML=
        data.progress+"%";
*/
#endif