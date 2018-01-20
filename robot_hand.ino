//www.elegoo.com
//2016.12.08
//#include <stdio.h>
#include <string.h>
#include "IRremote.h"
#include <Servo.h>
//for the basic program
#define PASTLOOKING 10

//for the hand
int motorPin1 = 1;
int motorPin2 = 2;
bool closed = true;

//for the remote
int receiver = 11; // Signal Pin of IR receiver to Arduino Digital Pin 11
IRrecv irrecv(receiver);     // create instance of 'irrecv'
decode_results results;      // create instance of 'decode_results'

//for the servo motor
Servo myservo;
int servoPos = 1;


int compare1dArrays(int arrayA[], int arrayB[], int length){
  for (int i = 0; i < length; i++){
    if (arrayA[i] != arrayB[i]){ return 0; }
  }
  return 1;
}

void copyChangeCode(int newArray[9], int oldArray[9]){
  for (int i = 0; i < 9; i++){
    newArray[i] = oldArray[i];
  }
}
/*
void printValues(int values[3][3][3]){
  for (int i = 0; i < 3; i++){
    for (int j = 0; j < 3; j++){
      for (int k = 0; k < 3; k++){
        printf("%d\n", values[i][j][k]);
      }
      printf(">\n");
    }
    printf(">>\n");
  }
}
*/
void copyValues(int newArray[3][3][3], int oldArray[3][3][3]){
  for (int i = 0; i < 3; i++){
    for (int j = 0; j < 3; j++){
      for (int k = 0; k < 3; k++){
        newArray[i][j][k] = oldArray[i][j][k];
      }
    }
  }
}

void applyChange(int values[3][3][3], int changeCode[9]){

  for (int a = 0; a < 9; a++){
    changeCode[a] --;
  }

  for (int i = 0; i < 3; i++){
    for (int k = 0; k < 3; k++){
      values[i][k][ (0+k) % 3 ] += changeCode[ (3*i) + 0];
      values[i][k][ (1+k) % 3 ] += changeCode[ (3*i) + 1];
      values[i][k][ (2+k) % 3 ] += changeCode[ (3*i) + 2];
    }
  }

  for (int a = 0; a < 9; a++){//we need this so that we don't mess up changeCode in the upper stack frame
    changeCode[a] ++;
  }

}

void outputMove(int index){
  if (index == 0) {
    Rock();
  } else if (index == 1) {
    Paper();
  } else {
    Scissors();
  }
}

int maxIndex(int array[],int length){
  int best[2] = {0,array[0]};
  for (int i = 1; i < length; i++){
    if (array[i] > best[1]) {
      best[0] = i;
      best[1] = array[i];
    }
  }
  //printf("the top value was %d\n", best[1]);
  return best[0];
}

int makeMove(int values[3][3][3], int pastMoves[3]){
  int decidingValues[3] = {0,0,0};
  for (int i = 0; i < 3; i++){
    int pieceMoved = pastMoves[i];
    for (int j = 0; j < 3; j++){
      decidingValues[j] += values[i][pieceMoved][j];
    }
  }
  return maxIndex(decidingValues,3);
}

void updateArray(int array[], int newElement, int length) {
  for (int i = 0; i < length - 1; i ++) {
    array[i] = array[i+1];
  }
  array[length-1] = newElement;
}
/*
void print(int array[], int length){
  for (int i = 0; i < length; i ++) {
    printf("%d\n", array[i]);
  }
  printf("\n");
}
*/
int evaluateValues(int values[3][3][3], int data[PASTLOOKING]){

  int score = 0;
  int wouldBeMove;
  for (int i = 0; i < (PASTLOOKING-3); i++){
    int tempMoves[3] = {data[i], data[i+1], data[i+2]};
    wouldBeMove = makeMove(values, tempMoves);
    if ( wouldBeMove == (data[i+3] + 1) % 3 ){//if the computer move would have beaten the user move
      score++;
    } else {//this method does not like ties. Maybe not the best. TODO test having ties be worth nothing
      score --;
    }
  }
  return score;

}

int incrementBaseThree(int number[], int length){//the return value is 1 if successfull, and 0 if not. this happens when you try and increment an all two's array

  //NOTE: number[0] is the least significant digit
  int i = 0;
  int carry = 1;
  while (carry) {
    number[i]++;
    if (number[i]==3){
      number[i] = 0;
    } else {
      carry = 0;
    }
    i++;
    if (i == length) {
      if (carry) return 0;
    }
  }
  return 1;
}

void hillClimb(int values[3][3][3],int pastData[PASTLOOKING]){
  int bestVal = evaluateValues(values,pastData);
  int bestCode[9] = {0,0,0,0,0,0,0,0,0};

  int valueChangeCode[9] = {0,0,0,0,0,0,0,0,0};
  int valuesCopy[3][3][3];
  //copyValues(valuesCopy,values);
  int i = 0;
  while (incrementBaseThree(valueChangeCode,9)){
    copyValues(valuesCopy,values);
    applyChange(valuesCopy,valueChangeCode);
    int evaluation = evaluateValues(valuesCopy,pastData);
    if (evaluation > bestVal) {
      bestVal = evaluation;
      copyChangeCode(bestCode,valueChangeCode);
    }
    i++;
  }
  applyChange(values,bestCode);
  //printf("the best values I found had a score of: %d\n", bestVal);
}
void setup() 
{
  Serial.begin(9600);
  Serial.println("IR Receiver Button Decode"); 
  irrecv.enableIRIn(); // Start the receiver
  myservo.attach(9);

  int goForAnother = 1;

  /*  evaluateRecentMoves is:
   *  [move][pieceMoved][valueForPiece]
   */

  //myservo.write(1);
   
  int evaluateRecentMoves[3][3][3] = {
    {
      {0,0,0},
      {0,0,0},
      {0,0,0}
    },
    {
      {0,0,0},
      {0,0,0},
      {0,0,0}
    },
    {
      {0,0,0},
      {0,0,0},
      {0,0,0}
    }
  };
  int pastMoves[3] = {0,1,2};
  int extendedPastMoves[PASTLOOKING];
  char userMove[1];
  int userMoveInt;

  int userMovesTaken = 0;

  while (1) {
    int move = makeMove(evaluateRecentMoves,pastMoves);
    outputMove(move);
    Serial.print("making a "); Serial.println(move);
    /*
    printf("what was your move? (r/p/s)\n");
    scanf("%s",userMove);
    */
    goBack:
    while (! irrecv.decode(&results) ) // have we received an IR signal?
    { delay(100); }
    userMoveInt = translateIR()-1;
    if (userMoveInt == -1) {
      Serial.println("here");
      delay(100);
      irrecv.resume();
      goto goBack;
    }
    Serial.print("recieving a "); Serial.println(userMoveInt);
    irrecv.resume(); // receive the next value
    //printf("\n");
/*
    //now we need to convert the input to an integer
    if (strcmp(userMove,"r")==0) {
      userMoveInt = 0;
    } else if (strcmp(userMove,"p")==0) {
      userMoveInt = 1;
    } else if (strcmp(userMove,"s")==0) {
      userMoveInt = 2;
    } else {return 1;}//this closes the program because it has gotten an invalid input error
*/
    updateArray(pastMoves,userMoveInt,3);
    updateArray(extendedPastMoves,userMoveInt,PASTLOOKING);
    if (userMovesTaken > PASTLOOKING){
      hillClimb(evaluateRecentMoves,extendedPastMoves);
    }

    userMovesTaken++;
  }
}

void loop() 
{

}

void Rock() {
  if (!closed){
    Close();
  }
  servoPos = 1;
  myservo.write(servoPos);
  delay(50);
}

void Paper() {
  if (!closed) {
    Close();
  }
  servoPos = 90;
  myservo.write(servoPos);
  delay(50);
}

void Scissors() {
  if (closed) {
    Open();
  }
  servoPos = 1;
  myservo.write(servoPos);
  delay(50);
}

void Close() {
  digitalWrite(motorPin1, HIGH); //rotates motor
  digitalWrite(motorPin2, LOW);    // set the Pin motorPin2 LOW
  delay(2500); //waits
  digitalWrite(motorPin2, HIGH);    // set the Pin motorPin1 LOW
}

void Open() {
  digitalWrite(motorPin2, HIGH); //rotates motor
  digitalWrite(motorPin1, LOW);    // set the Pin motorPin1 LOW
  delay(2500); //waits
  //digitalWrite(motorPin2, LOW);    // set the Pin motorPin2 LOW
}

int translateIR() // takes action based on IR code received
// describing Remote IR codes 
{

  switch(results.value)

  {
  case 0xFF30CF: return 1;    break; //1
  case 0xFF18E7: return 2;    break; //2
  case 0xFF7A85: return 3;    break; //3

  default: 
    return 0;

  }

  delay(500); // Do not get immediate repeat


} //END translateIR
