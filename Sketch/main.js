let h = 720;
let w = 1366;
let h_box = 75;
let w_box = 150;
let d_box = 200;
let lAxis = 100;
let font;

let serial;
let latestData = "waiting for data";
let roll, pitch, yaw;
let str; 

function setup() {
    
    createCanvas(windowWidth, windowHeight, WEBGL);

    textFont(font);
    textSize(22);

    serial = new p5.SerialPort();

    serial.list();
    serial.open('COM6');

    serial.on('connected', serverConnected);

    serial.on('list', gotList);

    serial.on('data', gotData);

    serial.on('error', gotError);

    serial.on('open', gotOpen);

    serial.on('close', gotClose);
}   

function draw(){
    background(122);
    text(latestData, -100, 200);
    push();
    //Compute rotation
    rotateZ(-radians(pitch));
    rotateX(radians(roll));
    rotateY(radians(yaw));
    //Draw
    
    strokeWeight(6);
    stroke(230,14,14);
    line(0,0,0,0,0,-lAxis-d_box/2); //X-axis
    stroke(21,230,14);
    line(-lAxis-w_box/2,0, 0 ,0 ,0,0); // Y-axis
    //line(w_box,h_box, w_box/2,h_box/2);
    stroke(7,21,114);
    line(0, -lAxis-h_box/2, 0, 0, 0, 0); //Z-axis
    strokeWeight(1);
    stroke(0);
    fill(25,122,255);
    box(w_box, h_box, d_box);
    pop();
}

function serverConnected() {
 print("Connected to Server");
}

function gotList(thelist) {
 print("List of Serial Ports:");

 for (let i = 0; i < thelist.length; i++) {
  print(i + " " + thelist[i]);
 }
}

function gotOpen() {
 print("Serial Port is Open");
}

function gotClose(){
 print("Serial Port is Closed");
 latestData = "Serial Port is Closed";
}

function gotError(theerror) {
 print(theerror);
}

function gotData() {
 let currentString = serial.readLine();
  trim(currentString);
 if (!currentString) return;
 //console.log(currentString);
 latestData = currentString;
    try {
        
    } catch (error) {
        
    }
 str = split(latestData, ',');
    roll = parseFloat(str[0]);
    pitch = parseFloat(str[1]);
    yaw = parseFloat(str[2]);
}

function preload(){
    font = loadFont('assets/BAHNSCHRIFT 1.TTF');
}
