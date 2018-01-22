//www.elegoo.com
//2016.12.08
//#include <stdio.h>
#include <string.h>
#include "IRremote.h"
#include <Servo.h>

//for the hand
int motorPin1 = 3; //red
int motorPin2 = 2; //black
bool closed = true; //the hand has to start closed

//for the LED
int RED = 7;
int GREEN = 6;
int BLUE = 5;

//for the remote
int receiver = 11; // Signal Pin of IR receiver to Arduino Digital Pin 11
IRrecv irrecv(receiver);     // create instance of 'irrecv'
decode_results results;      // create instance of 'decode_results'

//for the servo motor
Servo myservo;
int servoPos = 1;


void outputMove(int userMove) {
  if (userMove == 0) {
    Rock();
  } else if (userMove == 1) {
    Paper();
  } else {
    Scissors();
  }
}

void setup()
{
  //for the hand
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  digitalWrite(motorPin1, HIGH);
  digitalWrite(motorPin2, HIGH);
  Open();

  //for the LED
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);

  //general
  Serial.begin(9600);

  //for the IR reciever
  irrecv.enableIRIn(); // Start the receiver
  myservo.attach(9);

}
void loop()
{
goBack:
  while (! irrecv.decode(&results) ) // have we received an IR signal?
  {
    delay(100);
  }
  int move = translateIR() - 1;
  if (move != 0) {
    color(0, 255, 0);
    Serial.println("error in IR");
    delay(500);
    color(0, 0, 0);
    irrecv.resume();
    goto goBack;
  }
  outputMove(rand() % 3);
  Serial.print("making a "); Serial.println(move);
  //Serial.print("recieving a "); Serial.println(userMoveInt);
  irrecv.resume(); // receive the next value
}

void color(int r, int g, int b) {
  digitalWrite(RED, r);
  digitalWrite(GREEN, g);
  digitalWrite(BLUE, b);
}

void Rock() {
  color(255, 0, 0);
  myservo.write(45);
  if (!closed) {
    Close();
  }
  delay(500);
  color(0, 0, 0);
}

void Paper() {
  color(255, 0, 255);
  myservo.write(135);
  if (!closed) {
    Close();
  }
  delay(500);
  color(0, 0, 0);
}

void Scissors() {
  color(0, 0, 255);
  myservo.write(45);
  if (closed) {
    Open();
  }
  delay(500);
  color(0, 0, 0);
}

void Close() {
  closed = true;
  Serial.println("calling Close()");
  digitalWrite(motorPin1, HIGH); //rotates motor
  digitalWrite(motorPin2, LOW);    // set the Pin motorPin2 LOW
  delay(2500); //waits
  digitalWrite(motorPin2, HIGH);    // set the Pin motorPin1 LOW
}

void Open() {
  closed = false;
  Serial.println("calling Open()");
  digitalWrite(motorPin2, HIGH); //rotates motor
  digitalWrite(motorPin1, LOW);    // set the Pin motorPin1 LOW
  delay(2700); //waits
  digitalWrite(motorPin1, HIGH);    // set the Pin motorPin2 LOW
}

int translateIR() // takes action based on IR code received
// describing Remote IR codes
{

  switch (results.value)

  {
    case 0xFF30CF: return 1;    break; //1
    case 0xFF18E7: return 2;    break; //2
    case 0xFF7A85: return 3;    break; //3

    default:
      return 0;

  }

  delay(500); // Do not get immediate repeat


} //END translateIR
