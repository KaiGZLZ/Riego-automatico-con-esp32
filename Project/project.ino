/*  Para esta primera entrega se presentarà un control basico de encendido y apagado por medio de un boton.
    Se implemento de esta manera para probar el envio y recepcion de datos entre los clientes y el dispositivo, para afianzar los conocimientos y posteriormente poder implementar
  el sistema final planteado en los objetivos */



//////////  LIBRERIAS  ///////////////////////////

#include <WiFi.h>     //  Libreria utilizada para el manejo de las funciones WiFi
#include <ESPmDNS.h>  
#include <WebSocketsServer.h>   //  Libreria utilizada para la creacion del servidor Web Socket

#include <ArduinoJson.h>  //  Libreria utilizada para la utilizacion del formato JSON para el envio y recepcion de los datos

#include <Ticker.h> //  Esta es una libreria que me va a permitir crear un temporizador
//  Es importante tener en cuenta que cuando se tiene un servidor WebSocket, el cliente estara en constante disponibilidad con el servidor y viceversa, eso implica que
//* yo puedo enviar en cualquier momento un mensaje y el receptor lo tendra. Cuando se tiene un sensor, se necesita enviar constantemente las peticiones que este 
//* requiera directo, mas sin embargo eso no se puede hacer todo el tiempo ya que de lo contrario habria un consumo excesivo de recursos. Es por esto entonce que la 
//* informacion del servidor al cliente, debe ser enviada periodicamente, o cuando el cliente lo solicite , y de esta manera entonces se puede obtener el estado de 
//* la variable que se requiere. Es por esta razon que se necesita un temporizador, para que envie la informacion periodicamente cada cierto tiempo

#include <ESPAsyncWebServer.h>  //  Libreria utilizada para la implementacion de un servidor asincrono en el esp32

AsyncWebServer server(80);        //  Servidor local asincrono en el puerto 80 para manejar la pagina web
WebSocketsServer websockets(81);  //  Servidor webSocket en el 81 para manejar las solicitudes de este tipo



//////////  DEFINICIONES DE LOS PINES  ///////////////////////////

#define VALVULA1 2 //  Se define el pin para la valvula 1
#define VALVULA2 4 //  Se define el pin para la valvula 2

#define SENSOR  36 //  Se define el pin para el conversor analogico digital



//////////  DEFINICIONES DE LAS FUNCIONES  ///////////////////////////

void enviarDataSensor(); //  Esta sera la funcion para enviar la informacion a la pagina web, pero para este caso, unicamente se declara el prototipo de la
                    //* funcion, para posteriormente al final del programa se definirla

void enviarActualizacionEstados();   //  Funcion para enviar al cliente todos los estados de las variables para que sean modificadas en la interfaz del usuario

void reloj();

void escribirHora();

//////////  DEFINICIONES DE LAS VARIABLES  ///////////////////////////

bool estadoValvula1, estadoValvula2;


int horas = 0, minutos = 0, segundos = 0, momentoDelDia = 0, diaSemana = 1;


int horaEncendido1 = 0, minutoEncendido1 = 0, horaApagado1 = 0, minutoApagado1 = 0;
bool momentoEncendido1 = 0, momentoApagado1 = 0, estadoAlarma1 = 0;
bool dias1[8];

int horaEncendido2 = 0, minutoEncendido2 = 0, horaApagado2 = 0, minutoApagado2 = 0;
bool momentoEncendido2 = 0, momentoApagado2 = 0, estadoAlarma2 = 0;
bool dias2[8];


String clave = "12345";           //  Clave de desbloqueo de usuario para realizar modificaciones
bool clientesConfirmados[100];    //  Lista de clientes que pueden realizar modificaciones en el sistema
                                  //* Hay que tener en cuenta que esto se va a verificar desde el Frontend, pero esto le da un poco mas de seguridad al sistema

void send_confirmacion(); //  Funcion para la confirmacion del desbloqueo de usuario 

String nombre1 = "HORARIO 1";
String nombre2 = "HORARIO 2"; 

  
Ticker timer;       //  Se define la variable temporizador, la cual servira para el envio de informacion por parte del servidor al cliente.


//////////  PAGINA WEB ///////////////////////////

char paginaWeb[] PROGMEM = R"=====(



<!DOCTYPE html>
<html>
    

<head>
    <meta http-equiv="Content-Type" content="text/html"; charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0"&amp;gt;/>

</head>

<script>

    var conexion = new WebSocket('ws://'+location.hostname+':81/');


    conexion.onmessage = function(event){

        var full_data = event.data;        

        var data = JSON.parse(full_data);   

        console.log(full_data);


        if(data.MENSAJE == "CLAVE"){

            if(data.CONFIRMACION == false){

                alert("La clave ingresada es INCORRECTA");

                return;
            }
            else{

                alert("La clave ingresada es CORRECTA");
                return;
            }
        }

        if(data.AUTORIZACION == false){

            send_clave();
            
            return;
        }


        var momentoEncendido1, momentoApagado1;

        estadoAlarma1 = data.ALARMA_1;

        horaEncendido1 = parseInt(data.HORA_ENCENDIDO_1);
        minutoEncendido1 = parseInt(data.MINUTO_ENCENDIDO_1);
        horaApagado1 = parseInt(data.HORA_APAGADO_1);
        minutoApagado1 = parseInt(data.MINUTO_APAGADO_1);



        horaEncendido1 = parseInt(horaEncendido1);
        minutoEncendido1 = parseInt(minutoEncendido1);

        horaApagado1 = parseInt(horaApagado1);
        minutoApagado1 = parseInt(minutoApagado1);



        if (parseInt(data.MOMENTO_ENCENDIDO_1))  momentoEncendido1 = "PM";
        
        else   momentoEncendido1 = "AM";   
        

        if (parseInt(data.MOMENTO_APAGADO_1))  momentoApagado1 = "PM";
        
        else   momentoApagado1 = "AM"; 

        
        
        if (horaEncendido1 < 10)     horaEncendido1 = "0" + horaEncendido1;

        if (minutoEncendido1 < 10)     minutoEncendido1 = "0" + minutoEncendido1;

        if (horaApagado1 < 10)     horaApagado1 = "0" + horaApagado1;

        if (minutoApagado1 < 10)     minutoApagado1 = "0" + minutoApagado1;


        
        document.getElementById("encendido1_Texto").innerHTML = horaEncendido1+ ":" + minutoEncendido1 + "  " + momentoEncendido1;    
        
        document.getElementById("apagado1_Texto").innerHTML = horaApagado1+ ":" + minutoApagado1 + "  " + momentoApagado1;    
        
        document.getElementById("botonHorario1").checked = estadoAlarma1;

        document.getElementById("botonValvula1").checked = data.VALVULA_1;


        document.getElementById("boxTodos1").checked = data.TODOS_1;
        document.getElementById("boxLunes1").checked = data.LUNES_1;
        document.getElementById("boxMartes1").checked = data.MARTES_1;
        document.getElementById("boxMiercoles1").checked = data.MIERCOLES_1;
        document.getElementById("boxJueves1").checked = data.JUEVES_1;
        document.getElementById("boxViernes1").checked = data.VIERNES_1;
        document.getElementById("boxSabado1").checked = data.SABADO_1;
        document.getElementById("boxDomingo1").checked = data.DOMINGO_1;

        console.log(data.NOMBRE_1);
        document.getElementById("nombreFuncionAlarma1").innerHTML = data.NOMBRE_1;    
        

        var momentoEncendido2, momentoApagado2;

        estadoAlarma2 = data.ALARMA_2;

        horaEncendido2 = parseInt(data.HORA_ENCENDIDO_2);
        minutoEncendido2 = parseInt(data.MINUTO_ENCENDIDO_2);
        horaApagado2 = parseInt(data.HORA_APAGADO_2);
        minutoApagado2 = parseInt(data.MINUTO_APAGADO_2);



        horaEncendido2 = parseInt(horaEncendido2);
        minutoEncendido2 = parseInt(minutoEncendido2);

        horaApagado2 = parseInt(horaApagado2);
        minutoApagado2 = parseInt(minutoApagado2);



        if (parseInt(data.MOMENTO_ENCENDIDO_2))  momentoEncendido2 = "PM";
        
        else   momentoEncendido2 = "AM";   
        

        if (parseInt(data.MOMENTO_APAGADO_2))  momentoApagado2 = "PM";
        
        else   momentoApagado2 = "AM"; 

        
        
        if (horaEncendido2 < 10)     horaEncendido2 = "0" + horaEncendido2;

        if (minutoEncendido2 < 10)     minutoEncendido2 = "0" + minutoEncendido2;

        if (horaApagado2 < 10)     horaApagado2 = "0" + horaApagado2;

        if (minutoApagado2 < 10)     minutoApagado2 = "0" + minutoApagado2;


        
        document.getElementById("encendido2_Texto").innerHTML = horaEncendido2+ ":" + minutoEncendido2 + "  " + momentoEncendido2;    
        
        document.getElementById("apagado2_Texto").innerHTML = horaApagado2+ ":" + minutoApagado2 + "  " + momentoApagado2;    
        
        document.getElementById("botonHorario2").checked = estadoAlarma2;

        document.getElementById("botonValvula2").checked = data.VALVULA_2;


        document.getElementById("boxTodos2").checked = data.TODOS_2;
        document.getElementById("boxLunes2").checked = data.LUNES_2;
        document.getElementById("boxMartes2").checked = data.MARTES_2;
        document.getElementById("boxMiercoles2").checked = data.MIERCOLES_2;
        document.getElementById("boxJueves2").checked = data.JUEVES_2;
        document.getElementById("boxViernes2").checked = data.VIERNES_2;
        document.getElementById("boxSabado2").checked = data.SABADO_2;
        document.getElementById("boxDomingo2").checked = data.DOMINGO_2;

        console.log(data.NOMBRE_2);
        document.getElementById("nombreFuncionAlarma2").innerHTML = data.NOMBRE_2;    

    }

    function setEncendido(encendido_Hora,encendido_Minutos, encendido_AM, encendido_PM, encendido_Texto){
        
        console.log(encendido_Texto);

        var horaEncendido = parseInt(encendido_Hora.value);
        var minutoEncendido = parseInt(encendido_Minutos.value);
        var momentoEncendido


        if (!((horaEncendido > 0) && (horaEncendido < 13) && (minutoEncendido >= 0) && (minutoEncendido < 60))){    

            errorHora();
            return;
        }   
        
    
        if (encendido_AM.checked)

            momentoEncendido = 0;
        
        else

            momentoEncendido = 1;

        var full_data = '{"MENSAJE":"ENCENDIDO"';

        if(encendido_Hora.id == "entradaHoraEncendido1"){

                full_data = full_data + ',"ENCENDIDO":'+1;
        }
        else{

                full_data = full_data + ',"ENCENDIDO":'+2;
        }
        
        full_data = full_data + ',"HORA_ENCENDIDO":'+horaEncendido+',"MINUTO_ENCENDIDO":'+minutoEncendido+',"MOMENTO_ENCENDIDO":'+momentoEncendido+'}';
       
        console.log(full_data);

        conexion.send(full_data);

        enviarHora();
    }

    function setApagado(apagado_Hora,apagado_Minutos, apagado_AM, apagado_PM, apagado_Texto){
        
        console.log(apagado_Texto);

        var horaApagado = parseInt(apagado_Hora.value);
        var minutoapagado = parseInt(apagado_Minutos.value);
        var momentoapagado


        if (!((horaApagado > 0) && (horaApagado < 13) && (minutoapagado >= 0) && (minutoapagado < 60))){    

            errorHora();
            return;
        }   
        
    
        if (apagado_AM.checked)

            momentoapagado = 0;
        
        else

            momentoapagado = 1;

                 
        var full_data = '{"MENSAJE":"APAGADO"';


        if(apagado_Hora.id == "entradaHoraApagado1"){

            full_data = full_data + ',"APAGADO":'+1;
        }
        else{

            full_data = full_data + ',"APAGADO":'+2;
        } 

        full_data = full_data + ',"HORA_APAGADO":'+horaApagado+',"MINUTO_APAGADO":'+minutoapagado+',"MOMENTO_APAGADO":'+momentoapagado+'}';

        console.log(full_data);

        conexion.send(full_data);

        enviarHora();
    }

    function enviarHora(){
        
        var hoy = new Date();
        var dia = hoy.getDay();
        var horas = hoy.getHours();
        var minutos = hoy.getMinutes();
        var segundos = hoy.getSeconds();
      
        var full_data = '{"MENSAJE":"RELOJ"';

        full_data = full_data + ',"HORAS":'+horas+',"MINUTOS":'+minutos+',"SEGUNDOS":'+segundos+',"DIA":'+dia+'}';

        console.log(full_data);

        conexion.send(full_data);
    }

    function enviarNombre(nombreX){
        
        var full_data = '{"MENSAJE":"NOMBRE"';

        if(nombreX.id == "nombreFuncionAlarma1"){
 
           full_data = full_data + ',"HORARIO":'+1;
        }
        else if(nombreX.id == "nombreFuncionAlarma2"){

            full_data = full_data + ',"HORARIO":'+2;
        }
        
        full_data = full_data + ',"NOMBRE":'+'"'+nombreX.textContent+'"'+'}';

        console.log(full_data);

        conexion.send(full_data);
    }

    function accionBotonHorario(numero){

        var boton;

        if(numero == 1) boton = document.getElementById("botonHorario1");

        if(numero == 2) boton = document.getElementById("botonHorario2");


        var full_data = '{"MENSAJE":"BOTON"';

        full_data = full_data + ',"HORARIO":'+numero+',"ACCION":'+boton.checked+'}';

        console.log(full_data);

        conexion.send(full_data);

        enviarHora();
    }
    
    function enviarDias(numero){

        condicionDias(numero);

        if (numero == 1){
            
            todos   = document.getElementById("boxTodos1");

            lunes   = document.getElementById("boxLunes1");
            martes  = document.getElementById("boxMartes1");
            miercoles = document.getElementById("boxMiercoles1");
            jueves  = document.getElementById("boxJueves1");
            viernes = document.getElementById("boxViernes1");
            sabado  = document.getElementById("boxSabado1");
            domingo = document.getElementById("boxDomingo1");
        }
        
        if (numero == 2){
            
            todos   = document.getElementById("boxTodos2");

            lunes   = document.getElementById("boxLunes2");
            martes  = document.getElementById("boxMartes2");
            miercoles = document.getElementById("boxMiercoles2");
            jueves  = document.getElementById("boxJueves2");
            viernes = document.getElementById("boxViernes2");
            sabado  = document.getElementById("boxSabado2");
            domingo = document.getElementById("boxDomingo2");
        }
        

        var full_data = '{"MENSAJE":"DIAS1"';

        full_data = full_data + ',"HORARIO":'+numero+   ',"TODOS":'+todos.checked+
                                                        ',"LUNES":'+lunes.checked+
                                                        ',"MARTES":'+martes.checked+'}';
        console.log(full_data);
        conexion.send(full_data);

        var full_data = '{"MENSAJE":"DIAS2"';

        full_data = full_data + ',"HORARIO":'+numero+   ',"MIERCOLES":'+miercoles.checked+
                                                        ',"JUEVES":'+jueves.checked+
                                                        ',"VIERNES":'+viernes.checked+'}';
        console.log(full_data);
        conexion.send(full_data);

        var full_data = '{"MENSAJE":"DIAS3"';

        full_data = full_data + ',"HORARIO":'+numero+   ',"SABADO":'+sabado.checked+
                                                        ',"DOMINGO":'+domingo.checked+'}';
        console.log(full_data);
        conexion.send(full_data);

        enviarHora();
    }

    function accionBotonValvula(numero){

        var boton;

        if(numero == 1) boton = document.getElementById("botonValvula1");

        if(numero == 2) boton = document.getElementById("botonValvula2");


        var full_data = '{"MENSAJE":"VALVULA"';

        full_data = full_data + ',"HORARIO":'+numero+',"ACCION":'+boton.checked+'}';

        console.log(full_data);

        conexion.send(full_data);

        enviarHora();
    }

    function send_clave(){

        var clave = prompt("Ingrese la clave del sistema para realizar modificaciones");

        var full_data =  '{"MENSAJE":"CLAVE"'
                        +',"clave":"'+clave+'"}';
                    
        console.log(full_data);
                    
        conexion.send(full_data);
    }


</script>

<style>

  html {
      font-size: 10px;
  }
  
  body {
      background: Black;
      color: lime;
      font-family: 'Orbitron', sans-serif;
      animation: all .7s; 
  }
  
  /*  SECCION DEL RELOJ   */
  
  .contenedor-reloj {
      z-index: 1; /* Sit on top */
  
      max-width: 40rem;
      padding-top: 1rem;
      margin-top: 3rem;
      margin-bottom: 2rem;
      background-color: rgb(49, 49, 49);
      position: -webkit-sticky;
      position: sticky;
      top: 0px;
      left: 0px;
      right: 0px;
      border-radius: 20px;
  }
  
  .dia-semana-de-reloj {
      font-size: 3rem;
  }
  
  .hora-de-reloj {
      width: 100%;
      padding: 10px;
      font-size: 4rem;
  }
  
  
  /*  SECCION DE LA PAGINA    */
  
  .contenedor-principal {
      border: 2px solid rgb(116, 116, 116);
      border-radius: 1rem;
      width: 65%;
  }
  
  .nombre-horario {
      display: flex;
      flex-wrap: wrap;
      align-items: center;
      justify-content: center;
      width: 50%;
      font-size: 3rem;
  }
  
  .boton-valvula {
      margin-top: 1rem;
      margin-bottom: 3rem;
      position: relative;
      width: 60%;
      top: 10px;
      font-size: 2em;
  }
  
  .boton-dias{
      margin-bottom: 4rem;
      cursor: pointer;
      border-radius: 1rem;
      padding: 0.5rem 1rem;
  }
  .boton-dias:hover {
      opacity: 0.8;
      color: white;
      transition: 0.4s;
  }
  
  .dias-de-activacion {
      display: none; /* Hidden by default */
      position: fixed; /* Stay in place */
      z-index: 1; /* Sit on top */
      padding-top: 10rem; /* Location of the box */
      left: 0;
      top: 0;
      width: 100%; /* Full width */
      height: 100%; /* Full height */
      overflow: auto; /* Enable scroll if needed */
      background-color: rgb(0,0,0); /* Fallback color */
      background-color: rgba(0,0,0,0.4); /* Black w/ opacity */
  }
  
  .contenido-dias-de-activacion {
      font-size: 2.4rem;
      background-color: black;
      margin: auto;
      padding: 2rem;
      border: 0.2rem solid #888;
      width: 80%;
  }
  
  .tablaDias {
      margin-left: 5rem;
      font-family: arial, sans-serif;
      border-collapse: collapse;
      width: 50%;
      height: 10%;
      border-color: aliceblue;
  }
  
  .cajasDias {
  
      width: 30px;  /*  Ancho de la caja*/
      height: 30px; /*  Alto de la caja*/
  }
  
  .close {
      color: #aaaaaa;
      float: right;
      font-size: 28px;
      font-weight: bold;
  }
  
  .close:hover,
  .close:focus {
      color: white;
      text-decoration: none;
      cursor: pointer;
  }
  
  /*  COMIENZO DE LA ENTRADA DE DATOS */
  
  .texto-horas-minutos{
      font-size: 2rem;
      margin-bottom: 1.5rem;
  }
  
  .entrada-horas-minutos {
      width : 10rem;
      padding: 0.5rem 2rem;
      border-radius: 1.5rem;
      font-size: 1em;
      font-weight: bold;
      text-align: center;
  }
  
  .radio-momento{
      display: inline-block;
      margin: 0px 10px;
      font-size: large;
      cursor: pointer;
  }
  
  .boton-ok{
      margin: 10px;
      padding: 5px;
      background: grey;
      border: solid 1px grey;
      font-weight: bold;
      font-size: 20px;
      border-radius: 50%;
      cursor: pointer;
  }
  .boton-ok:hover {
      opacity: 0.8;
      color: white;
      transition: 0.4s;
  }
  
  .contenedor-hora-fijada {
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      padding-left: 8rem;
  }
  
  .hora-fijada {
      font-size: 2.5rem;
      line-height: 3.5rem;
      margin-top: 0rem;
      margin-bottom: 4.5rem;
  }
  
  table {
      margin-left: 0;
      font-family: arial, sans-serif;
      border-collapse: collapse;
      /* width: 60%; */
      border: 10px;
      border-color: aliceblue;
      align-content:inherit;
  }
  
  
  .switch {
      position: relative;
      display: inline-block;
      width: 60px;  /*  Ancho de la caja*/
      height: 34px; /*  Alto de la caja*/
  }
  
  button{
      margin: 10px;
      padding: 5px;
      background: grey;
      border: solid 1px grey;
      font-weight: bold;
      font-size: 15px;
  }
  
  
  .switch input {
      opacity: 0;
      width: 40px;
      height: 21px;
  }
  
  .slider {
      position: absolute;
      cursor: pointer;
      top: 0;
      left:0;
      right: 0;
      bottom: 0;
      background-color: rgba(219, 51, 21, 0.822);
      -webkit-transition: .4s;
      transition: .4s;
  }
  
  .slider:before {
      position: absolute;
      content:"";
      height: 26px; /*  Alto del slider */
      width: 30px;  /*  Ancho del slider*/
      left: 4px;    /*  Cuan corrido estara el slider del BORDE IZQUIERDO la caja HACIA LA DERECHA*/
      bottom: 4px;  /*  Cuan corrido estara el slider del BORDE DERECHO la caja HACIA LA IZQUIERDA*/
      background-color: white;
      -webkit-transition: .4s;
      transition: .4s;
  }
  
  input:checked + .slider {
      background-color: rgb(15, 182, 15);
  }
  
  input:focus + .slider {
      box-shadow: 0 0 1px #2196F3;
  }
  
  input:checked + .slider:before {
      -webkit-transform: translateX(26px);
      -ms-transform: translateX(26px);
      transform: translateX(26px);
      left: 0px;   /*  Modificacion del movimiento a la izquierda, luego de que se ha movido*/
  }
  
  .slider.round {
      border-radius: 34px;
  }
  
  .slider.round:before {
      border-radius: 50%;
  }
  
  
  
  /* Changing sizes.... */
  
  @media (max-width:500px) {
  
      .contenedor-principal {
          width: 95%;
      }
      
      .contenedor-reloj {
          margin-top: 0;
      }
  
      .nombre-horario {
          width: 85%;
          font-size: 2.5rem;
      }
  
      .boton-valvula {
          margin-top: 0.5rem;
          margin-bottom: 2rem;
          font-size: 1.7em;
      }
  
      .boton-dias{
          font-size: 1.3rem;
      }
  
      .tablaDias {
          margin-left: 0;
      }
  
      /*  COMIENZO DE LA ENTRADA DE DATOS */
  
      .texto-horas-minutos{
          font-size: 1.5rem;
      }
      
      .entrada-horas-minutos {
          width : 3rem;
          padding: 0.5rem 1.5rem;
      }
      
      .radio-momento{
          font-size: 1.3rem;
      }
      .contenedor-hora-fijada {
          padding-left: 3rem;
          width: 12rem;
      }
  
      .hora-fijada {
          font-size: 2rem;
          line-height: 2.5rem;
      }
  }
  
  </style>
  

<body>
  <center>        <!-- Se declara que lo siguiente a ser escrito sera centrado-->


  <!--  SECCION DEL CONTENEDOR DEL RELOJ -->

    <div class="contenedor-reloj">
      <div class="dia-semana-de-reloj" id="dia">Hora:</div>
      <div class="hora-de-reloj" id="horaReloj"></div>
    </div>

  <!--  SECCION DEL HORARIO 1 Y SUS BOTONES -->
    <div class="contenedor-principal">
      <div class="nombre-horario">
        <p id="nombreFuncionAlarma1" contenteditable="true" oninput="enviarNombre(nombreFuncionAlarma1)">Horario 1</p>
      </div>
      
      <div>
        <label class="switch">
          <input id="botonHorario1" onclick="accionBotonHorario(1)"  type="checkbox">
          <span class="slider round"></span>
        </label> <br/>
      </div>

      <div class="boton-valvula" >
        <label for="botonValvula1">
          <p style="display: inline-block;">Valvula 1 &nbsp</p>
          <label class="switch">
            <input id="botonValvula1" onclick="accionBotonValvula(1)"  type="checkbox">
            <span class="slider round"></span>
          </label>
        </label>
      </div>

    <!--  SECCION DEL CONTROL DE LOS DIAS DE LA SEMANA --> 

      <button id="botonDias1" class="boton-dias" onclick="funcionBotonDias(diasDeActivacion1, 1)">Dias de la semana</button>

      <div id="diasDeActivacion1" class="dias-de-activacion">
        <div class="contenido-dias-de-activacion">
          <span onclick="funcionSpan(diasDeActivacion1, 1)" class="close">&times;</span>
          <p><strong>HORARIO</strong></p><br/>
            <div>
              <table class="tablaDias">
                <tr>
                  <th>
                    <input id="boxTodos1" class="cajasDias" type="checkbox" />
                  </th>
                  <th>Todos</th>
                </tr>
                <tr>
                  <th>&nbsp</th>
                  <th>&nbsp</th>
                </tr>
                <tr>
                  <th>
                    <input id="boxLunes1" class="cajasDias" type="checkbox" />
                  </th>
                  <th>Lunes</th>
                </tr>
                <tr>
                  <th>
                    <input id="boxMartes1" class="cajasDias" type="checkbox" />
                  </th>
                  <th>Martes</th>
                </tr>
                <tr>
                  <th>
                    <input id="boxMiercoles1" class="cajasDias" type="checkbox" />
                  </th>
                  <th>Miercoles</th>
                </tr>
                <tr>
                  <th>
                    <input id="boxJueves1" class="cajasDias" type="checkbox" />
                  </th>
                  <th>Jueves</th>
                </tr>
                <tr>
                  <th>
                    <input id="boxViernes1" class="cajasDias" type="checkbox" />
                  </th>
                  <th>Viernes</th>
                </tr>
                <tr>
                  <th>
                    <input id="boxSabado1" class="cajasDias" type="checkbox" />
                  </th>
                  <th>Sabado</th>
                </tr>
                <tr>
                  <th>
                    <input id="boxDomingo1" class="cajasDias" type="checkbox" />
                  </th>
                  <th>Domingo</th>
                </tr>
              </table>
            </div>
          </div>
        </div>

        <!--  SECCION DEL SECCION DE LA ENTRADA DE DATOS -->

        <div>
          <table id="tabla">
            <tr>
              <th>
                <div class="texto-horas-minutos">
                  <label for="entradaHoraEncendido1"> Hora:&nbsp&nbsp&nbsp&nbsp&nbsp     <input type="text" id="entradaHoraEncendido1" class="entrada-horas-minutos">
                  </label>
                </div>
                <div class="texto-horas-minutos">
                  <label for="entradaMinutoEncendido1"> Minuto:&nbsp&nbsp <input type="text" id="entradaMinutoEncendido1" class="entrada-horas-minutos">
                  </label>
                </div>
    
                <div>
                  <div class="radio-momento">
                    <label for="entradaMomentoAmEncendido1" >
                      <input type="radio" id="entradaMomentoAmEncendido1" name="entradaMomentoEncendido1" value="AM">AM
                    </label>
                  </div>
                  <div class="radio-momento">
                    <label for="entradaMomentoPmEncendido1" >
                      <input type="radio" id="entradaMomentoPmEncendido1" name="entradaMomentoEncendido1" value="PM">PM
                    </label>
                  </div>
                </div>
                <button class="boton-ok" onclick="setEncendido(entradaHoraEncendido1, entradaMinutoEncendido1, entradaMomentoAmEncendido1, entradaMomentoPmEncendido1,encendido1_Texto)">OK!</button><br/><br/><br/>
    
              </th>
              <th class="contenedor-hora-fijada">
                <p class="hora-fijada">Hora de Encendido </p>
                <p id="encendido1_Texto" class="hora-fijada"></p>
              </th>
            </tr>
            <tr>
              <th>
                <div class="texto-horas-minutos">
                  <label for="entradaHoraApagado1"> Hora:&nbsp&nbsp&nbsp&nbsp&nbsp     <input type="text" id="entradaHoraApagado1" class="entrada-horas-minutos">
                  </label>
                </div>
                <div class="texto-horas-minutos">
                  <label for="entradaMinutoApagado1"> Minuto:&nbsp&nbsp <input type="text" id="entradaMinutoApagado1" class="entrada-horas-minutos">
                  </label>
                </div>
                <div>
                  <div class="radio-momento">
                    <label for="entradaMomentoAmApagado1" >
                      <input type="radio" id="entradaMomentoAmApagado1" name="apagado1_Momento" value="AM">AM
                    </label>
                  </div>
                  <div class="radio-momento">
                    <label for="entradaMomentoPmApagado1" >
                      <input type="radio" id="entradaMomentoPmApagado1" name="apagado1_Momento" value="PM">PM
                    </label>
                  </div>
                </div>
                <button class="boton-ok" onclick="setApagado(entradaHoraApagado1, entradaMinutoApagado1, entradaMomentoAmApagado1, entradaMomentoPmApagado1,apagado1_Texto)">OK!</button><br/><br/><br/>
              </th>
              <th class="contenedor-hora-fijada">
                <p class="hora-fijada">Hora de Apagado </p>
                <p id="apagado1_Texto" class="hora-fijada"></p>
              </th>
            </tr>
          </table>
        </div>
      </div>



        <p>   <br><br><br><br>  </p>


      <div class="contenedor-principal">
        <div class="nombre-horario">
            <p id="nombreFuncionAlarma2" class="nombre" contenteditable="true" oninput="enviarNombre(nombreFuncionAlarma2)">Horario 2 &nbsp</p>
        </div>

        <div>
          <label class="switch">
            <input id="botonHorario2" onclick="accionBotonHorario(2)"  type="checkbox">
            <span class="slider round"></span>
          </label> <br/>
        </div>

        <div class="boton-valvula" >
          <label for="botonValvula2">
            <p style="display: inline-block;">Valvula 2 &nbsp</p>
            <label class="switch">
              <input id="botonValvula2" onclick="accionBotonValvula(2)"  type="checkbox">
              <span class="slider round"></span>
            </label>
          </label>
        </div>
        
  <!--  SECCION DEL CONTROL DE LOS DIAS DE LA SEMANA --> 

        <button id="botonDias2" class="boton-dias" onclick="funcionBotonDias(diasDeActivacion2, 2)">Dias de la semana</button>

        <div id="diasDeActivacion2" class="dias-de-activacion">
            <div class="contenido-dias-de-activacion">
                <span onclick="funcionSpan(diasDeActivacion2, 2)" class="close">&times;</span>
                <p><strong>HORARIO</strong></p><br/>
                <div display="align-right">
                    <table class="tablaDias">
                        <tr><th><input id="boxTodos2" class="cajasDias" type="checkbox" /></th><th>Todos</th></tr>
                        <tr><th>&nbsp</th><th>&nbsp</th></tr>
                        <tr><th><input id="boxLunes2" class="cajasDias" type="checkbox" /></th><th>Lunes</th></tr>
                        <tr><th><input id="boxMartes2" class="cajasDias" type="checkbox" /></th><th>Martes</th></tr>
                        <tr><th><input id="boxMiercoles2" class="cajasDias" type="checkbox" /></th><th>Miercoles</th></tr>
                        <tr><th><input id="boxJueves2" class="cajasDias" type="checkbox" /></th><th>Jueves</th></tr>
                        <tr><th><input id="boxViernes2" class="cajasDias" type="checkbox" /></th><th>Viernes</th></tr>
                        <tr><th><input id="boxSabado2" class="cajasDias" type="checkbox" /></th><th>Sabado</th></tr>
                        <tr><th><input id="boxDomingo2" class="cajasDias" type="checkbox" /></th><th>Domingo</th></tr>
                    </table>
                </div>
            </div>
        </div>

      <!--  SECCION DEL SECCION DE LA ENTRADA DE DATOS -->       
        
        <div>
          <table id="tabla" style="align-self: center;">
            <tr>
              <th>
                <div class="texto-horas-minutos">
                  <label for="entradaHoraEncendido2"> Hora:&nbsp&nbsp&nbsp&nbsp&nbsp     <input type="text" id="entradaHoraEncendido2" class="entrada-horas-minutos">
                  </label>
                </div>
                <div class="texto-horas-minutos">
                  <label for="entradaMinutoEncendido2"> Minuto:&nbsp&nbsp <input type="text" id="entradaMinutoEncendido2" class="entrada-horas-minutos">
                  </label>
                </div>

                <div>
                  <div class="radio-momento">
                    <label for="entradaMomentoAmEncendido2" >
                      <input type="radio" id="entradaMomentoAmEncendido2" name="entradaMomentoEncendido2" value="AM">AM
                    </label>
                  </div>
                  <div class="radio-momento">
                    <label for="entradaMomentoPmEncendido2" >
                      <input type="radio" id="entradaMomentoPmEncendido2" name="entradaMomentoEncendido2" value="PM">PM
                    </label>
                  </div>
                </div>

                <button class="boton-ok" onclick="setEncendido(entradaHoraEncendido2, entradaMinutoEncendido2, entradaMomentoAmEncendido2, entradaMomentoPmEncendido2,encendido2_Texto)">OK!</button><br/><br/><br/>

              </th>
              <th class="contenedor-hora-fijada">
                <p class="hora-fijada">Hora de Encendido </p>
                <p id="encendido2_Texto" class="hora-fijada"></p>
              </th>
            </tr>
            <tr>
              <th>
                <div class="texto-horas-minutos">
                  <label for="entradaHoraApagado2"> Hora:&nbsp&nbsp&nbsp&nbsp&nbsp     <input type="text" id="entradaHoraApagado2" class="entrada-horas-minutos">
                  </label>
                </div>
                <div class="texto-horas-minutos">
                  <label for="entradaMinutoApagado2"> Minuto:&nbsp&nbsp <input type="text" id="entradaMinutoApagado2" class="entrada-horas-minutos">
                  </label>
                </div>
                <div>
                  <div class="radio-momento">
                    <label for="entradaMomentoAmApagado2" >
                      <input type="radio" id="entradaMomentoAmApagado2" name="entradaMomentoApagado2" value="AM">AM
                    </label>
                  </div>
                  <div class="radio-momento">
                    <label for="entradaMomentoPmApagado2" >
                      <input type="radio" id="entradaMomentoPmApagado2" name="entradaMomentoApagado2" value="PM">PM
                    </label>
                  </div>
                </div>
                <button class="boton-ok" onclick="setApagado(entradaHoraApagado2, entradaMinutoApagado2, entradaMomentoAmApagado2, entradaMomentoPmApagado2,apagado2_Texto)">OK!</button><br/><br/><br/>
              </th>
              <th class="contenedor-hora-fijada">
                <p class="hora-fijada">Hora de Apagado </p>
                <p id="apagado2_Texto" class="hora-fijada"></p>
              </th>
            </tr>
          </table>
        </div>
      </div>
    </center>
</body>

<script>

    function reloj() {

        var hoy = new Date();
        var dia = hoy.getDay();
        var hora = hoy.getHours();
        var minuto = hoy.getMinutes();
        var segundo = hoy.getSeconds();
        var estadoHora;

        if (hora == 0){ hora = 12;  estadoHora = 1}

        if (hora > 12){ hora = hora - 12;   estadoHora = 1}

        else estadoHora = 0;

        if (minuto < 10) {
            minuto = "0" + minuto
        };
        if (segundo < 10) {
            segundo = "0" + segundo
        };
        

        var horaReloj = document.getElementById("horaReloj");

        if (estadoHora == 0)    horaReloj.textContent = (hora + ":" + minuto + ":" + segundo + " AM");

        else                    horaReloj.textContent = (hora + ":" + minuto + ":" + segundo + " PM");


        var dias = ["Domingo", "Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "Sabado"];

        document.getElementById("dia").textContent = (dias[dia]);
    };

    setInterval(reloj, 1000);

    function errorHora(){

        alert("Por favor ingrese correctamente la hora");
    }



    function funcionBotonDias(diasDeActivacion, numero){

        diasDeActivacion.style.display = "block";
        
        enviarDias(numero);
    }

    function funcionSpan(diasDeActivacion, numero){

        diasDeActivacion.style.display = "none";

        enviarDias(numero);
    }

    function condicionDias(numero){

        
        if (numero == 1){
            
            todos   = document.getElementById("boxTodos1");

            lunes   = document.getElementById("boxLunes1");
            martes  = document.getElementById("boxMartes1");
            miercoles = document.getElementById("boxMiercoles1");
            jueves  = document.getElementById("boxJueves1");
            viernes = document.getElementById("boxViernes1");
            sabado  = document.getElementById("boxSabado1");
            domingo = document.getElementById("boxDomingo1");
        }

        else if (numero == 2){

            todos   = document.getElementById("boxTodos2");

            lunes   = document.getElementById("boxLunes2");
            martes  = document.getElementById("boxMartes2");
            miercoles = document.getElementById("boxMiercoles2");
            jueves  = document.getElementById("boxJueves2");
            viernes = document.getElementById("boxViernes2");
            sabado  = document.getElementById("boxSabado2");
            domingo = document.getElementById("boxDomingo2");
        }
        
        
        if ((lunes.checked == 0)&&(martes.checked == 0)&&(miercoles.checked == 0)&&(jueves.checked == 0)&&(viernes.checked == 0)&&(sabado.checked == 0)&&(domingo.checked == 0)){
            
            todos.checked = 1;
        }
        else {
            todos.checked = 0;
        }
    }
    
    
    var diasDeActivacion1 = document.getElementById("diasDeActivacion1");
    var diasDeActivacion2 = document.getElementById("diasDeActivacion2");

    window.onclick = function(event) {

        if (event.target == diasDeActivacion1)     {
            
            diasDeActivacion1.style.display = "none";
            enviarDias(1);
        }    
        if (event.target == diasDeActivacion2)     {
            
            diasDeActivacion2.style.display = "none";
            enviarDias(2);
        }   
    }    


</script>
</html>



)=====";

////////// FIN DE LA PAGINA WEB ///////////////////////////



void notFound(AsyncWebServerRequest *request) //  En caso de que se ingrese alguna otra direccion en el buscador web, que no sea la del dispositivo
{
    request->send(404, "text/plain", "Ingrese en la direccion 192.168.4.1 para ingresar al sistema");
}


//  Se define la funcion para el caso de que ocurra un evento de tipo WebSocket

void webSocketEvent(uint8_t numCliente, WStype_t type, uint8_t * payload, size_t length) {

  switch (type) 
  {
    case WStype_DISCONNECTED: //  En caso de que uno de los clientes se desconecte
        
        Serial.printf("Cliente [%u] desconectado\n", numCliente);

        clientesConfirmados[numCliente] = false;  //  En caso de que el usuario pudiese modificar el estado de las variables

        break;

    
    case WStype_CONNECTED: {  //  En caso de que se conecte un nuevo cliente
        
        IPAddress ip = websockets.remoteIP(numCliente); //  Se le asigna una direccion IP al cliente
        Serial.printf("Cliente [%u] IP: %d.%d.%d.%d url: %s\n", numCliente, ip[0], ip[1], ip[2], ip[3], payload); //  Se observa toda la informacion del cliente en pantalla

        // Se obtendrá la hora del dispositivo que se conecte

        enviarActualizacionEstados(); //  Le envio los estados de las variables al cliente que recien se conectó, para que se le actualice todo
        
      }
      break;

    case WStype_TEXT:   //  En caso de que se envie otro tipo de mensaje por parte de alguno de los clientes

      Serial.printf("Cliente [%u]   Mensaje: %s\n", numCliente, payload); //  Se muestra la informacion del cliente y del mensaje

      String mensaje = String((char*)( payload));   //  Se guarda la informacion del mensaje en una variable para posteriormente ser procesada 
      
      DynamicJsonDocument variableJSON(200);  //  Se crea una variable de tipo JSON para contener toda la informacion que llegue desde la pagina web
    
      DeserializationError error = deserializeJson(variableJSON, mensaje);  //  En caso de que haya algun error en el mensaje se guarda el mensaje de error para luego ser mostrado

      //  Si hubo un error se muestra
      if (error) {  
          Serial.print("Error en la deserializacion");
          Serial.println(error.c_str());
          return;
      }

      //  Si no hubo ningun error

      
      if(variableJSON["MENSAJE"] == "CLAVE"){    //  En caso de que el mensaje corresponda con una clave de desbloqueo para poder modificar las variables del sistema
                
          if(variableJSON["clave"] == clave){    //  En caso de que la clave SI coincida

              Serial.println("La clave es CORRECTA");
      
              clientesConfirmados[numCliente] = true;   //  Ahora el cliente puede realizar modificaciones al sistema
      
              send_confirmacionClave(numCliente, true);      //  Envio la confirmacion de que ahora puede realizar modificaciones para que se  
                                                                  //* cambie la variable ubicada en el Frontend que realiza este permiso 
           }
           else{                         //  En caso de que la clave NO coincida  

              Serial.println("La clave es INCORRECTA");
              
              send_confirmacionClave(numCliente, false);
           }

           enviarActualizacionEstados(); 
           
           return;
      }

      if(clientesConfirmados[numCliente] == false){  //  En caso de que el cliente NO este autorizado a modificar variables en el sistema

          if (variableJSON["MENSAJE"] == "RELOJ") return;
          
          if (variableJSON["MENSAJE"] == "DIAS2") return;
          if (variableJSON["MENSAJE"] == "DIAS3") return;
          
          usuarioNoAutorizado(numCliente);
          return;
      }
      
      if(variableJSON["MENSAJE"] == "ENCENDIDO"){

          if(variableJSON["ENCENDIDO"] == 1){

              estadoAlarma1 = 1;
              
              horaEncendido1 = variableJSON["HORA_ENCENDIDO"];
              minutoEncendido1 = variableJSON["MINUTO_ENCENDIDO"];
              momentoEncendido1 = variableJSON["MOMENTO_ENCENDIDO"];
          }
          if(variableJSON["ENCENDIDO"] == 2){

              estadoAlarma2 = 1;
              
              horaEncendido2 = variableJSON["HORA_ENCENDIDO"];
              minutoEncendido2 = variableJSON["MINUTO_ENCENDIDO"];
              momentoEncendido2 = variableJSON["MOMENTO_ENCENDIDO"];
          }
      }
      
      else if (variableJSON["MENSAJE"] == "APAGADO"){

          if(variableJSON["APAGADO"] == 1){

              estadoAlarma1 = 1;
              
              horaApagado1 = variableJSON["HORA_APAGADO"];
              minutoApagado1 = variableJSON["MINUTO_APAGADO"];
              momentoApagado1 = variableJSON["MOMENTO_APAGADO"];
          }
          if(variableJSON["APAGADO"] == 2){

              estadoAlarma2 = 1;
              
              horaApagado2 = variableJSON["HORA_APAGADO"];
              minutoApagado2 = variableJSON["MINUTO_APAGADO"];
              momentoApagado2 = variableJSON["MOMENTO_APAGADO"];
          }
      }
      
      else if (variableJSON["MENSAJE"] == "RELOJ"){

          horas = variableJSON["HORAS"];

          if(horas > 12){

              horas = horas - 12;
              momentoDelDia = 1;
          }
          else if(horas == 0){

              horas = 12;
              momentoDelDia = 1;
          }
          else{

              momentoDelDia = 0;
          }
          
          minutos = variableJSON["MINUTOS"];
          segundos = variableJSON["SEGUNDOS"];
          diaSemana = variableJSON["DIA"];
      }
      
      else if (variableJSON["MENSAJE"] == "BOTON"){

          if(variableJSON["HORARIO"] == 1){
 
              estadoAlarma1 = variableJSON["ACCION"]; if(estadoAlarma1 == 0){  estadoValvula1 = 0; digitalWrite(VALVULA1, LOW); enviarActualizacionEstados();}
          }
          if(variableJSON["HORARIO"] == 2){
 
              estadoAlarma2 = variableJSON["ACCION"]; if(estadoAlarma2 == 0){  estadoValvula2 = 0; digitalWrite(VALVULA2, LOW); enviarActualizacionEstados();}
          }
      }
      
      else if (variableJSON["MENSAJE"] == "VALVULA"){

          if(variableJSON["HORARIO"] == 1){   estadoValvula1 = variableJSON["ACCION"]; digitalWrite(VALVULA1, variableJSON["ACCION"]); enviarActualizacionEstados();}

          if(variableJSON["HORARIO"] == 2){   estadoValvula2 = variableJSON["ACCION"]; digitalWrite(VALVULA2, variableJSON["ACCION"]); enviarActualizacionEstados();}

          if(estadoAlarma1 == 0){  
      
              estadoValvula1 = 0;  
              digitalWrite(VALVULA1, LOW); 
              enviarActualizacionEstados();
          } 
          
          if(estadoAlarma2 == 0){  
      
              estadoValvula2 = 0;  
              digitalWrite(VALVULA2, LOW); 
              enviarActualizacionEstados();
          } 
      }

      else if (variableJSON["MENSAJE"] == "DIAS1"){

          if(variableJSON["HORARIO"] == 1){   
            
              dias1[0] = variableJSON["TODOS"]; 
              dias1[1] = variableJSON["LUNES"]; 
              dias1[2] = variableJSON["MARTES"]; 
          }

          if(variableJSON["HORARIO"] == 2){   
            
              dias2[0] = variableJSON["TODOS"]; 
              dias2[1] = variableJSON["LUNES"]; 
              dias2[2] = variableJSON["MARTES"]; 
          }
      }
      
      else if (variableJSON["MENSAJE"] == "DIAS2"){

          if(variableJSON["HORARIO"] == 1){  
              
              dias1[3] = variableJSON["MIERCOLES"]; 
              dias1[4] = variableJSON["JUEVES"]; 
              dias1[5] = variableJSON["VIERNES"];
          }

          if(variableJSON["HORARIO"] == 2){  
              
              dias2[3] = variableJSON["MIERCOLES"]; 
              dias2[4] = variableJSON["JUEVES"]; 
              dias2[5] = variableJSON["VIERNES"];
          }
      }
      
      else if (variableJSON["MENSAJE"] == "DIAS3"){

          if(variableJSON["HORARIO"] == 1){  
            
              dias1[6] = variableJSON["SABADO"]; 
              dias1[7] = variableJSON["DOMINGO"]; 
              
              enviarActualizacionEstados();
          }

          if(variableJSON["HORARIO"] == 2){  
            
              dias2[6] = variableJSON["SABADO"]; 
              dias2[7] = variableJSON["DOMINGO"]; 
              
              enviarActualizacionEstados();
          }
      }
      
      else if (variableJSON["MENSAJE"] == "NOMBRE"){

          if(variableJSON["HORARIO"] == 1)    nombre1 = (const char*)variableJSON["NOMBRE"]; 
          
          if(variableJSON["HORARIO"] == 2)    nombre2 = (const char*)variableJSON["NOMBRE"];

          return;
      }


      Serial.print("Hora de encendido 1   ");
      Serial.print(horaEncendido1);
      Serial.print(":");
      Serial.print(minutoEncendido1);
      Serial.print(" -- ");
      if(momentoEncendido1 == 1)  Serial.println("PM");    
      else  Serial.println("AM");
      
      Serial.print("Hora de apagado 1  ");
      Serial.print(horaApagado1);
      Serial.print(":");
      Serial.print(minutoApagado1);
      Serial.print(" -- ");
      
      if(momentoApagado1 == 1)  Serial.println("PM");    
      else  Serial.println("AM");
      


      Serial.print("Hora de encendido 2   ");
      Serial.print(horaEncendido2);
      Serial.print(":");
      Serial.print(minutoEncendido2);
      Serial.print(" -- ");
      if(momentoEncendido2 == 1)  Serial.println("PM");    
      else  Serial.println("AM");

      Serial.print("Hora de apagado 2  ");
      Serial.print(horaApagado2);
      Serial.print(":");
      Serial.print(minutoApagado2);
      Serial.print(" -- ");
      
      if(momentoApagado2 == 1)  Serial.println("PM");    
      else  Serial.println("AM");
      
      
      
      Serial.print("Hora Actual   ");
      Serial.print(horas);
      Serial.print(":");
      Serial.print(minutos);
      Serial.print(":");
      Serial.print(segundos);
      Serial.print(" -- ");
      if(momentoDelDia == 1)  Serial.println("PM");    
      else  Serial.println("AM");

      enviarActualizacionEstados();
  }
}

//  Ahora se establecen las entradas y salidas del sistema
void setup(void)
{
    Serial.begin(115200);
    pinMode(VALVULA1,OUTPUT);  //  Se declara el puerto 2 como salida
    pinMode(VALVULA2,OUTPUT);  //  Se declara el puerto 3 como salida

    
    //  Se inicia el arreglo de clientes confirmados con todos en false

    for (int i = 0; i < 100; i++)
    {
      clientesConfirmados[i] = false;
    }

    //  Se coloca por defecto a encender todos los dias
    
    dias1[0] = 1;
    dias2[0] = 1;
    
    
    WiFi.softAP("ESP32", ""); //  Se crea un servidor WiFi con el nombre de ESP32 y sin contraseña
    Serial.println("softap");
    Serial.println("");
    Serial.println(WiFi.softAPIP());


  if (MDNS.begin("ESP")) { //esp.local/
    Serial.println("MDNS responder started");
  }

  server.on("/", [](AsyncWebServerRequest * request)
  {
      request->send_P(200, "text/html", paginaWeb);
  });

  server.on("/led1/on", HTTP_GET, [](AsyncWebServerRequest * request)
  { 
      digitalWrite(VALVULA1,HIGH);
      request->send_P(200, "text/html", paginaWeb);
  });

  server.onNotFound(notFound);

  server.begin();     //  Se inicia el servidor Asincrono
  websockets.begin(); //  Se inicia el servidor WebSocket 
  websockets.onEvent(webSocketEvent);

  //  A continuacion se fija un temporizador el cual se programa en segundos, y que tendra junto a ella la funcion para enviar informacion del sensor al 
  //* cliente. Funcion la cual se explicara mas abajo 
  //  Notese que el temporizador tiene como tiempo de refresco 0.3 segundos, lo cual implica que en ese periodo es que se va a estar enviando la data del sensor
  timer.attach(1,reloj);

}

void loop(void)
{
    websockets.loop();
}

void reloj()
{
    if(segundos < 59){   segundos = segundos + 1;   comprobarHoras();   return;}

    else{
        segundos = 0;

        if(minutos < 59){    minutos = minutos + 1;   comprobarHoras(); return;}

        else{
            minutos = 0;

            if (horas < 12){   horas = horas + 1;    comprobarHoras();  return;}
            
            else{
                horas = 1;
                if (momentoDelDia == 1){
                  
                  if (diaSemana == 7) diaSemana = 1;
                  else  diaSemana = diaSemana + 1;
                }
                momentoDelDia = !momentoDelDia;
                comprobarHoras();
                //escribirHora();
            }
        }
    }
}

void comprobarHoras(){

//  PARA VALVULA 1
  
    if((horas == horaEncendido1)&&(minutos == minutoEncendido1)&&(momentoEncendido1 == momentoDelDia)&&(estadoAlarma1 == 1)&&((dias1[diaSemana]==1)||(dias1[0]==1))){    
      
        estadoValvula1 = 1;  
        digitalWrite(VALVULA1, HIGH);
        enviarActualizacionEstados();
    }

    if((horas == horaApagado1)&&(minutos == minutoApagado1)&&(momentoApagado1 == momentoDelDia)&&(estadoAlarma1 == 1)){    
      
        estadoValvula1 = 0;  
        digitalWrite(VALVULA1, LOW);
        enviarActualizacionEstados();
    }

    Serial.print("Estado de valvula 1  ");
    Serial.println(estadoValvula1);

//  PARA VALVULA 2
  
    if((horas == horaEncendido2)&&(minutos == minutoEncendido2)&&(momentoEncendido2 == momentoDelDia)&&(estadoAlarma2 == 1)&&((dias2[diaSemana]==1)||(dias2[0]==1))){    
      
        estadoValvula2 = 1;  
        digitalWrite(VALVULA2, HIGH);
        enviarActualizacionEstados();
    }

    if((horas == horaApagado2)&&(minutos == minutoApagado2)&&(momentoApagado2 == momentoDelDia)&&(estadoAlarma2 == 1)){    
      
        estadoValvula2 = 0;  
        digitalWrite(VALVULA2, LOW);
        enviarActualizacionEstados();
    }

    Serial.print("Estado de valvula 2  ");
    Serial.println(estadoValvula2);
    
    escribirHora();
}

void escribirHora(){

    Serial.print("Hora Actual   ");
    Serial.print(horas);
    Serial.print(":");
    Serial.print(minutos);
    Serial.print(":");
    Serial.print(segundos);  
    Serial.print("  ");

    if(momentoDelDia == 1)  Serial.println("PM");
    
    else  Serial.println("AM");
}

void enviarActualizacionEstados(){

  String informacionJSON = "{\"MENSAJE\":\"INFORMACION\"";  
         
         informacionJSON += ",\"ALARMA_1\":"; //-------> ALARMA 1
         informacionJSON += estadoAlarma1;
          
         informacionJSON += ",\"VALVULA_1\":";
         informacionJSON += estadoValvula1;
         
         informacionJSON += ",\"HORA_ENCENDIDO_1\":";
         informacionJSON += horaEncendido1;
         informacionJSON += ",\"MINUTO_ENCENDIDO_1\":";
         informacionJSON += minutoEncendido1;
         informacionJSON += ",\"MOMENTO_ENCENDIDO_1\":";
         informacionJSON += momentoEncendido1;
         
         informacionJSON += ",\"HORA_APAGADO_1\":";
         informacionJSON += horaApagado1;
         informacionJSON += ",\"MINUTO_APAGADO_1\":";
         informacionJSON += minutoApagado1;
         informacionJSON += ",\"MOMENTO_APAGADO_1\":";
         informacionJSON += momentoApagado1;

         informacionJSON += ",\"TODOS_1\":";
         informacionJSON += dias1[0];
         informacionJSON += ",\"LUNES_1\":";
         informacionJSON += dias1[1];
         informacionJSON += ",\"MARTES_1\":";
         informacionJSON += dias1[2];
         informacionJSON += ",\"MIERCOLES_1\":";
         informacionJSON += dias1[3];
         informacionJSON += ",\"JUEVES_1\":";
         informacionJSON += dias1[4];
         informacionJSON += ",\"VIERNES_1\":";
         informacionJSON += dias1[5];
         informacionJSON += ",\"SABADO_1\":";
         informacionJSON += dias1[6];
         informacionJSON += ",\"DOMINGO_1\":";
         informacionJSON += dias1[7];

         informacionJSON += ",\"NOMBRE_1\":\"";
         informacionJSON += nombre1;
         informacionJSON += "\"";

         informacionJSON += ",\"ALARMA_2\":";  //-------> ALARMA 2
         informacionJSON += estadoAlarma2;

         informacionJSON += ",\"VALVULA_2\":";
         informacionJSON += estadoValvula2;

         informacionJSON += ",\"HORA_ENCENDIDO_2\":";
         informacionJSON += horaEncendido2;
         informacionJSON += ",\"MINUTO_ENCENDIDO_2\":";
         informacionJSON += minutoEncendido2;
         informacionJSON += ",\"MOMENTO_ENCENDIDO_2\":";
         informacionJSON += momentoEncendido2;
         
         informacionJSON += ",\"HORA_APAGADO_2\":";
         informacionJSON += horaApagado2;
         informacionJSON += ",\"MINUTO_APAGADO_2\":";
         informacionJSON += minutoApagado2;
         informacionJSON += ",\"MOMENTO_APAGADO_2\":";
         informacionJSON += momentoApagado2;

         informacionJSON += ",\"TODOS_2\":";
         informacionJSON += dias2[0];
         informacionJSON += ",\"LUNES_2\":";
         informacionJSON += dias2[1];
         informacionJSON += ",\"MARTES_2\":";
         informacionJSON += dias2[2];
         informacionJSON += ",\"MIERCOLES_2\":";
         informacionJSON += dias2[3];
         informacionJSON += ",\"JUEVES_2\":";
         informacionJSON += dias2[4];
         informacionJSON += ",\"VIERNES_2\":";
         informacionJSON += dias2[5];
         informacionJSON += ",\"SABADO_2\":";
         informacionJSON += dias2[6];
         informacionJSON += ",\"DOMINGO_2\":";
         informacionJSON += dias2[7];

         informacionJSON += ",\"NOMBRE_2\":\"";
         informacionJSON += nombre2;
         informacionJSON += "\"";

         informacionJSON += ",\"AUTORIZACION\":";
         informacionJSON += true;
         
         informacionJSON += "}";
         
    Serial.println(informacionJSON);
    
    websockets.broadcastTXT(informacionJSON);
}

void usuarioNoAutorizado(uint8_t cliente){

  String informacionJSON = "{\"AUTORIZACION\":";
         informacionJSON += false;
         
         informacionJSON += "}";
         
    Serial.println(informacionJSON);
    
    websockets.sendTXT(cliente, informacionJSON);
}

//  Funcion para confirmar si la clave ingresada por alguno de los usuarios, para permitirle la modificacion de las variables del sistema, es correcta 
void send_confirmacionClave(uint8_t cliente, bool confirmacion){

  String informacionJSON = "{\"MENSAJE\":\"CLAVE\"";
         
         informacionJSON += ",\"CONFIRMACION\":";
         informacionJSON += confirmacion;

         informacionJSON += ",\"AUTORIZACION\":";
         informacionJSON += confirmacion;
         
         informacionJSON += "}";
         
         Serial.println(informacionJSON);

         websockets.sendTXT(cliente, informacionJSON);  //  Le envio la autenticacion unicamente al cliente que la solicito 
}
