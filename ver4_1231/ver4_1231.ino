
#include <ESP32Servo.h>

#include <IcsHardSerialClass.h>
#include "BluetoothSerial.h"
BluetoothSerial SerialBT;
#include<stdio.h>


const byte EN_PIN = 2;
const long BAUDRATE =1250000; //115200;
const int TIMEOUT = 10;    //通信できてないか確認用にわざと遅めに設定

IcsHardSerialClass krs(&Serial,EN_PIN,BAUDRATE,TIMEOUT);  //インスタンス＋ENピン(2番ピン)およびUARTの指定

byte val=0,pre_val=0;

//#define
int s=5;

#define n 0
#define f 1
#define b 2
#define u 3
#define d 4
#define r 5
#define l 6
#define rr 7
#define lr 8


//servo for camera
// create for servo objects
Servo servo1;
int servo1Pin = 12;

// Published values for SG90 servos; adjust if needed
int minUs = 500;
int maxUs = 2400;
int pos = 0;      // position in degrees

typedef struct{
  int flag_hold=0;
  int current_angle=0;
  int ref_angle=0;
  float w=1;
  int last_state=0;
  int ki=0;
}serv;

serv servo_vector[6];



void setup() {

  //servo for camera
  servo1.setPeriodHertz(50);      // Standard 50hz servo
  servo1.attach(servo1Pin, minUs, maxUs);

  s+=1;
  SerialBT.begin("ESP32");
  //Serial.begin(9600);
  digitalWrite(2,OUTPUT);

//ICS用
  pinMode(4,OUTPUT);
  digitalWrite(4,HIGH);
  krs.begin();  //サーボモータの通信初期設定

//servo_vectorを初期化

  init_servo_vector();

//resetしてから５秒内に電源ONで目標角度、現在角度を取得
  delay(5000);
  for(int i=0;i<6;i++){
    servo_vector[i].current_angle=krs.getPos(i);
    servo_vector[i].ref_angle=servo_vector[i].current_angle;
  }
}

float pre_time=0;
void loop() {

//  for (pos = 0; pos <= 180; pos += 1) { // sweep from 0 degrees to 180 degrees
//    // in steps of 1 degree
//    servo1.write(pos);
//    delay(2);             // waits 20ms for the servo to reach the position
//  }
//  for (pos = 180; pos >= 0; pos -= 1) { // sweep from 180 degrees to 0 degrees
//    servo1.write(pos);
//    delay(2);
//  }


  int r_pos=0;
  //hold
  for (int i=0;i<6;i++){
    servo_vector[i].current_angle=krs.getPos(i);
    //flag_holdが１だとholdする、hold_check()で連続して同じコマンドが送られているときは０、コマンドが切れたときは１にする
    if(servo_vector[i].flag_hold==1){
      //目標値と現在値の差
      int dif=servo_vector[i].ref_angle-servo_vector[i].current_angle;
      if(2<abs(dif) && abs(dif)<500){
        krs.setPos(i,pi(dif,i));
      }else if(abs(dif)>6000){//3500->11500 or 11500->3500
        //forward後の場合　４が左、５が右？
        if(servo_vector[i].last_state==f){
          switch (i){
            case 4 :
            krs.setPos(i,11500);
            break;
            case 5:
            krs.setPos(i,3500);
            break;
          }

        }else if(servo_vector[i].last_state==b){// backの後の場合　４が左、５が右？
          switch (i){
            case 4 :
            krs.setPos(i,3500);
            break;
            case 5:
            krs.setPos(i,11500);
            break;
          }
        }else if(servo_vector[i].last_state==u){
          krs.setPos(i,11500);
        }else if(servo_vector[i].last_state==d){
          krs.setPos(i,3500);
        }
      }
        // servo_vector[i].current_angle=krs.getPos(i);
        // krs.setPos(i,servo_vector[i].current_angle);
    }
  }
  pre_time=micros();

  if(SerialBT.available()>0){
    val=SerialBT.read();
    //SerialBT.println("init");
  }

  if(val=='f'){
    forward();
    hold_check(f);

  }else if(val=='b'){
    back();
    SerialBT.println("back");
    hold_check(b);

  }else if(val=='u'){
    pull_up();
    SerialBT.println("pull_up");
    hold_check(u);

  }else if(val=='d'){
    pull_down();
    SerialBT.println("pull_down");
    hold_check(d);

  }else if(val=='r'){
    r_rotate();
    SerialBT.println("r_rotate");
    hold_check(r);

  }else if(val=='l'){
    l_rotate();
    SerialBT.println("l_rotate");
    hold_check(l);

  }else if(val=='a'){
    r_rotate_reverse();
    SerialBT.println("r_rotate_reverse");
    hold_check(rr);

  }else if(val=='c'){
    l_rotate_reverse();
    SerialBT.println("l_rotate_reverse");
    hold_check(lr);

  }else if(val=='y'){
    l_rotate();
    r_rotate();
    SerialBT.println("both_rotate");
    hold_check(l);
    hold_check(r);

  }else if(val=='h'){
    l_rotate_reverse();
    r_rotate_reverse();
    SerialBT.println("both_rotate_reverse");
    hold_check(lr);
    hold_check(rr);

  }
  else if(val=='i'){
    reset_servo_vector();
    delay(1000);
    init_servo_vector();
    
  }
  else if(val=='t'){
  //r_pos=set_pos(4,7400);

//  SerialBT.println(r_pos);
   //r_pos=krs.setPos(4,8000);
   //r_pos=krs.setFree(4);
   //r_pos=krs.getPos(4);

   r_pos=krs.setPos(4,7500);////////


   SerialBT.println(r_pos);
   }
  else{
//    krs.setPos(4,7500);/////////
//    krs.setPos(1,7500);
//    krs.setPos(3,7500);
//    krs.setFree(2);
//    //krs.setFree(4);/////////
//    krs.setFree(5);
//    //hold_all();


    hold_check(n);
    SerialBT.println("quit");
  }



}
