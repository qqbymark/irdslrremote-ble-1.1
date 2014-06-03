/*===========================
IrDslrRemote Arduino BLE 1.1

Irdslrremote 2.6 download

https://itunes.apple.com/tw/app/ir-dslr-remote/id590362141?l=zh&mt=8
 4.X Servo
 5.Y Servo
 7.LED
 8.Metering
 9.Shutter

 By Sfan 
=============================*/
#include <Servo.h>
int shutter = 9; //Shutter
int metering = 8; //Metering
int lifeled = 7; //LED
int sec=0;
int mode = 0;   //Mode 1 Start delay=YES 2 Start delay = NO
long startdelay = 0; //start delay time
long delaytime = 0; //delay time
long bulb = 0; //BULB
int count=0; //Count
int st=0; //Time lapse  1=YES 0=NO
long time=0; // 0.001s
int Triggertime = 300; //Trigger time 0.3s

Servo Xservo;
Servo Yservo;
float xangle=90; //X default
float yangle=90; //Y default
int sub=0;
int ptzxstart=-1,ptzxend=-1,ptzystart=-1,ptzyend=-1,ptzon=0;
float xstep=0,ystep=0;

void setup()
{ 
  Serial.begin(9600);
 
  pinMode(shutter,OUTPUT);
  pinMode(metering,OUTPUT);
  pinMode(lifeled,OUTPUT);
  
  digitalWrite(shutter,LOW);
  digitalWrite(metering,LOW);
  digitalWrite(lifeled,LOW);
  
  Xservo.attach(4,1000,2000);
  Xservo.write(xangle);

  Yservo.attach(5,1000,2000);
  Yservo.write(yangle);
  
  Serial1.begin(9600);
  delay(50);  
  Serial.print("Setup") ;
}

void loop()  
{  
  //BLE catch
  if(Serial1.available()>0) {
    char inputbuffer[Serial1.available()];
    Serial1.readBytes(inputbuffer,Serial1.available());
    delay(50);
    if(inputbuffer[0]=='0'){ //Reset
      reset();
      Serial1.println("0");
      delay(500);
    }
    else if(inputbuffer[0]=='1'){ //Time lapse setup
      catchdata();
      time=0;
      if(startdelay>0){
        mode=1;
      }
      else{
        mode=2;
      }
    }
    else if(inputbuffer[0]=='2'){
      catchptz();
    }
    else if(inputbuffer[0]=='4'){ //Shutter Touch
      Serial1.println("4");
      digitalWrite(lifeled,HIGH);
      digitalWrite(shutter,HIGH);
    }
    else if(inputbuffer[0]=='5'){ //Shutter Release
      Serial1.println("5");
      digitalWrite(shutter,LOW);
      digitalWrite(lifeled,LOW);
    }
    else if(inputbuffer[0]=='6'){ //Metering Touch
      Serial1.println("6");
      digitalWrite(metering,HIGH);
    }
    else if(inputbuffer[0]=='7'){ //Metering Release
      Serial1.println("7");
      digitalWrite(metering,LOW);
    }
    else if(inputbuffer[0]=='p'){ //Get XY data
      catchxyz();
    }
    else if(inputbuffer[0]=='b'){ //Get BULB
      catchbulb();
    }
    else if(inputbuffer[0]=='a'){ //Send X Y info to phone
      sendbackxy();
    }
    else if(inputbuffer[0]=='s'){ //Send Timelapse info to phone
      sendbacktimelapse();
    }      
  }
  //Time lapse work
  if(st==1){
    if(ptzxstart!=-1){
      xstep = (ptzxend-ptzxstart)/((float)count-1);
      Xservo.write(ptzxstart);
      xangle = ptzxstart;
      ptzxstart=-1;
      Serial.println(xstep);
    }
    if(ptzystart!=-1){
      ystep = (ptzyend-ptzystart)/((float)count-1);
      Yservo.write(ptzystart);
      yangle = ptzystart;
      ptzystart=-1;
      Serial.println(ystep);
    }
    delay(1);
    time++;
    timeovercheck();
    runlight(); 
    if(mode==1){
      if(time/1000==startdelay){
        //Serial.println("Mode1");
        stateHigh();
        count--;
        mode=2;
        time=0;
      }
    }
    else if(mode==2){
      if(count!=0){
        if((time+(delaytime*1000)/2)/1000==delaytime){
          if(ptzon==1){
            xangle = xangle+xstep;
            yangle = yangle+ystep;
            Xservo.write(xangle);
            Yservo.write(yangle);
            ptzon=0;
            //Serial.println(Xservo.read());
          }
        }
        if(time/1000==delaytime){
          stateHigh();
          count--;
          time=0;
          ptzon=1;
        }
      }
      else{
        reset();
        delay(300);
        Serial1.println("2");
      }
    }
  }
  else if(st==2){
    runlight();
    delay(1);
    time++;
    timeovercheck();
    if(time/1000==bulb){
      //digitalWrite(A,LOW);
      //digitalWrite(B,LOW);
      Serial1.print("9");
      reset();
     }
  }
}

//reset all
void reset(){
  st=0;
  time=0;
  startdelay=0;
  delaytime=0;
  sec=0;
  mode=0;
  count=0;
  ptzxstart = -1;
  ptzxend = -1;
  ptzystart = -1;
  ptzyend = -1;
  ptzon = 0;
  bulb=0;
  digitalWrite(lifeled,LOW);
  digitalWrite(shutter,LOW);
  digitalWrite(metering,LOW);
}

//check time over
void timeovercheck(){
  if(time>=99999999){
      time=0;
  }
}
//LED
void runlight(){
  sec++;
  if(sec==900){
    digitalWrite(lifeled,HIGH);
  }
  else if(sec==1100){
    digitalWrite(lifeled,LOW);
    sec=0;
  }
}
//Time lapse setup
void catchdata(){
  char inputbuffer[Serial1.available()];
  digitalWrite(lifeled,HIGH);
  Serial1.readBytes(inputbuffer,Serial1.available());
  delay(500);
  digitalWrite(lifeled,LOW);
  String str ="";
  if(inputbuffer[0]=='1'){ 
    for(int i=1;i<=5;i++){
      str+=inputbuffer[i];
    }
    startdelay = str.toInt();
    str="";
    for(int i=6;i<=10;i++){
      str+=inputbuffer[i];
    }
    delaytime = str.toInt();
    str="";
    for(int i=11;i<=14;i++){
      str+=inputbuffer[i];
    }
    count = str.toInt();
    str="";
    Serial1.print("1");
  }      
}
//shutter on
void stateHigh(){
  sendbacktimelapse();
  digitalWrite(metering,HIGH);
  digitalWrite(shutter,HIGH);
  delay(Triggertime);
  digitalWrite(shutter,LOW);
  digitalWrite(metering,LOW);
  sendbackxy();
}
void catchptz(){
  char inputbuffer[Serial1.available()];
  Serial1.readBytes(inputbuffer,Serial1.available());
  delay(500);
  String str = "";
  if(inputbuffer[0]=='1'){
    for(int i =1;i<=3;i++){
      str+=inputbuffer[i];
    }
    ptzxstart = str.toInt();
    str="";
    for(int i=4;i<=6;i++){
      str+=inputbuffer[i];
    }
    ptzxend = str.toInt();
    str="";
    for(int i=7;i<=9;i++){
      str+=inputbuffer[i];
    }
    ptzystart = str.toInt();
    str="";
    for(int i=10;i<=12;i++){
      str+=inputbuffer[i];
    }
    ptzyend = str.toInt();
    str="";
    ptzon=1;
    st=1;
    mode=1;
  }
}
void catchxyz(){
  char inputbuffer[Serial1.available()];
  digitalWrite(lifeled,HIGH);
  Serial1.readBytes(inputbuffer,Serial1.available());
  delay(100);
  digitalWrite(lifeled,LOW);
  String str ="";
  for(int i=0;i<3;i++){
    str+=inputbuffer[i];
  }
  xangle = str.toInt();
  Xservo.write(xangle);
  Serial.println(xangle);
  str="";
  for(int i=3;i<6;i++){
    str+=inputbuffer[i];
  }
  yangle = str.toInt();
  Yservo.write(yangle);
  Serial.println(yangle);
}
void sendbackxy(){
  Serial1.print("x");
  Serial1.print(xangle);
  
  Serial1.print("y");
  Serial1.print(yangle);
}
void sendbacktimelapse(){
  Serial1.print("s");
  Serial1.print(startdelay);
  
  Serial1.print("d");
  Serial1.print(delaytime);
  
  Serial1.print("c");
  Serial1.print(count);
}
void catchbulb(){
  char inputbuffer[Serial1.available()];
  digitalWrite(lifeled,HIGH);
  Serial1.readBytes(inputbuffer,Serial1.available());
  delay(100);
  digitalWrite(lifeled,LOW);
  String str ="";
  for(int i=0;i<5;i++){
    str+=inputbuffer[i];
  }
  bulb = str.toInt();
  Serial.print(bulb);
  if(bulb>0){
    Serial1.print("8");
    digitalWrite(metering,HIGH);
    digitalWrite(shutter,HIGH);
    st=2;
  }
  
}
