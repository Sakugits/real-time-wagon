/ --------------------------------------
// Include files
// --------------------------------------
#include <string.h>
#include <stdio.h>
#include <Wire.h>

// --------------------------------------
// Global Constants a
// --------------------------------------
#define SLAVE_ADDR 0x8
#define MESSAGE_SIZE 9

// --------------------------------------
// Global Variables
// --------------------------------------
double speed = 55.5;
bool request_received = false;
bool requested_answered = false;
char request[MESSAGE_SIZE+1];
char answer[MESSAGE_SIZE+1];

int out2  = 2; 
int out3  = 3;
int out4   = 4;
int out5  = 5;

// o = button not pushed 1 button pushed
int button = 0;

// number between [10000,90000]
long distance = 0; 

// 0 = distance selector 1 = approach mode 2 = stop mode
int mode = 0;

// 1 = on 0 = off
int lamp = 0;

// porcentage of light
int light = 0;

// 1 = up, -1 = down, 0 = flat
int slope = 0;

// 1 = on, 0 = off 
int gas = 1; 

// 1 = on, 0 = off
int brake = 0; 

// 1 = on, 0 = off
int mixer = 0; 

// --------------------------------------
// Function: comm_server
// --------------------------------------
int comm_server()
{
   static int count = 0;
   char car_aux;

   // If there were a received msg, send the processed answer or ERROR if none.
   // then reset for the next request.
   // NOTE: this requires that between two calls of com_server all possible 
   //       answers have been processed.
   if (request_received) {
      // if there is an answer send it, else error
      if (requested_answered) {
          Serial.print(answer);
      } else {
          Serial.print("MSG: ERR\n");
      }  
      // reset flags and buffers
      request_received = false;
      requested_answered = false;
      memset(request,'\0', MESSAGE_SIZE+1);
      memset(answer,'\0', MESSAGE_SIZE+1);
   }

   while (Serial.available()) {
      // read one character
      car_aux =Serial.read();
        
      //skip if it is not a valid character
      if  ( ( (car_aux < 'A') || (car_aux > 'Z') ) &&
           (car_aux != ':') && (car_aux != ' ') && (car_aux != '\n') ) {
         continue;
      }
      
      //Store the character
      request[count] = car_aux;
      
      // If the last character is an enter or
      // There are 9th characters set an enter and finish.
      if ( (request[count] == '\n') || (count == 8) ) {
         request[count] = '\n';
         count = 0;
         request_received = true;
         break;
      }


      // Increment the count
      count++;
   }
}
/ --------------------------------------
// Function: show_distance
// --------------------------------------
int show_distance()
{
   int value = 0;
   value = analogRead(A1);
   distance = map (value, 0, 1023, 10000, 90000);
   // while there is enough data for a request
   if ( (request_received) &&
        (0 == strcmp("DS:  REQ\n",request)) ) {

      char cstr[5];
	   ltoa(distance, cstr, 10);
     
     if(distance<=0){
       sprintf(answer,"DS:00000");
     } 
     else if(distance > 0 && distance < 10){
       sprintf(answer,"DS:0000%s",cstr);
     } 
     else if(distance >= 10 && distance < 100){
       sprintf(answer,"DS:000%s",cstr);
     } 
     else if(distance >= 100 && distance < 1000){
       sprintf(answer,"DS:00%s",cstr);
     } 
     else if(distance >= 1000 && distance < 10000){
       sprintf(answer,"DS:0%s",cstr);
     } 
     else if(distance >= 10000){
       sprintf(answer,"DS:%s",cstr);
     }
      request_received = false;
      answer_requested = true;
   }
   distance_aux = distance;
   while (distance_aux  >= 10){
    distance_aux  = distance_aux  / 10; 
  }

  digitalWrite(out2, bitRead(distance_aux , 0));  
  digitalWrite(out3, bitRead(distance_aux, 1));
  digitalWrite(out4, bitRead(distance_aux , 2));
  digitalWrite(out5, bitRead(distance_aux , 3));  

}
   return 0;
}
int movment_sensor(){

  int botton_val = 0;
  botton_val = digitalRead(6); 
  if(botton_value == 1 && botton == 0) {
    botton  = 1;
  }
  else if(botton== 1 && botton_value== 0){
     botton  = 0;
     if(mode == 0){
       mode = 1;
     } else if (mode == 2){
       mode = 0;
     }
  }
  if(speed <= 10 && distance <= 0){
      mode = 2;
   }
  if(mode != 2){
      double a = 0;
      if(acc==1){
         //V = Vo + A T; A = 0.5
         a = a + 0.5;
      }
      else if(brk==1){
         //V = Vo + A T; A= -0.5
         a = a - 0.5;
      }
      if(slope == -1){
         //V = Vo + A T; A= -0.25
         a = a + 0.25;
      }
      else if(slope == 1){
         //V = Vo + A T; A= 0.25
         a = a - 0.25;
      }

      distance = distance - (speed*t_distance + (1/2)*a*t_distance*t_distance);
   } 
   else {
      distance = 0;
   }

   return 0;
}

// --------------------------------------
// Function: read_light
// --------------------------------------
int read_ligth()
{
   int value = 0;
   value = analogRead(A0);
   ligth = map (value, 0, 1023, 0, 99);
   // while there is enough data for a request
   if ( (request_received) &&
        (0 == strcmp("LIT: REQ\n",request)) ) {
     char ptr[5];
	  itoa(ligth, cstr, 10);
     
     if(ligth>=10){
       sprintf(answer,"LIT: %s\n",ptr);
     } else {
       sprintf(answer,"LIT: 0%s\n",ptr);
     }
    request_received = false;
    requested_answered = true;
   }
   return 0;
}
// --------------------------------------
// Function: lamp_req
// --------------------------------------
int lamp_req()
{
   if ( (request_received) && (!requested_answered)){
        if (0 == strcmp("LAM: SET",request)) ) {
           lamp = 1;
           digitalWrite(7, HIGH);
           sprintf(answer,"LAM:  OK"); 
        }

        if( (request_received) &&
        (0 == strcmp("LAM: CLR",request)) ){
           lamp = 0;
           digitalWrite(7, LOW);
           sprintf(answer,"LAM:  OK");
        }
        request_received = false;
        requested_answered = true;  
   }
   return 0;
}
// --------------------------------------
// Function: speed_req
// --------------------------------------
int speed_req()
{
   // If there is a request not answered, check if this is the one
   if ( (request_received) && (!requested_answered) &&
        (0 == strcmp("SPD: REQ\n",request)) ) {

      // send the answer for speed request
      char num_str[5];
      dtostrf(speed,4,1,num_str);
      sprintf(answer,"SPD:%s\n",num_str);

      // set request as answered
      requested_answered = true;
   }
   return 0;
}
// --------------------------------------
// Function: gas_req
// -------------------------------------- 
int gas_req(){
   if ( (request_received) && (!requested_answered) &&
        (0 == strcmp("GAS: SET\n",request)) ) {
         gas = 1;
         digitalWrite(13, HIGH);
         requested_answered = true;
         sprintf(answer,"GAS:  OK\n");
      }
      if ( (request_received) && (!requested_answered) &&
        (0 == strcmp("GAS: CLR\n",request)) ) {
         gas = 0;
         digitalWrite(13, LOW);
         requested_answered = true;
         sprintf(answer,"GAS:  OK\n", request);
      }
      return 0;
}
// --------------------------------------
// Function: brake_req
// --------------------------------------
int brake_req(){
   if ( (request_received) && (!requested_answered) &&
        (0 == strcmp("BRK: SET\n",request)) ) {
         brake = 1;
         digitalWrite(12, HIGH);
         requested_answered = true;
         sprintf(answer,"BRK:  OK\n");
      }
      
   if ( (request_received) && (!requested_answered) &&
        (0 == strcmp("BRK: CLR\n",request)) ) {
         brake = 0;
         digitalWrite(12, LOW);
         requested_answered = true;
         sprintf(answer,"BRK:  OK\n");
      }
   return 0; 
}
// --------------------------------------
// Function: mix_req
// --------------------------------------   
int mix_req(){
   if ( (request_received) && (!requested_answered) &&
       (0 == strcmp("MIX: SET\n",request)) )
   {
         mixer = 1;
         digitalWrite(11, HIGH);
         requested_answered = true;
         sprintf(answer, "MIX:  OK\n");
   }
    if ( (request_received) && (!requested_answered) &&
      (0 == strcmp("MIX: CLR\n",request)) )
      {
         mixer = 0;
         digitalWrite(11, LOW);
         requested_answered = true;
         sprintf(answer, "MIX:  OK\n");
      }
  return 0;
}
// --------------------------------------
// Function: show_speed
// --------------------------------------
int show_speed(){
  double acceleration = 0; 
   if(gas == 1 ){
      acceleration = acceleration + 0.5;
      // Serial.print(acceleration);
      }
   if(brake == 1){
      acceleration = acceleration - 0.5;
      }
   if(slope == 1){
      acceleration = acceleration - 0.25;
      }
   if (slope == -1){
      acceleration =  acceleration + 0.25; 
      }
   speed = speed +  (acceleration * 0.1);
   
   if(speed < 40 ){
      digitalWrite(10, 0);
   } else if (speed > 70){
      digitalWrite(10, 1);
   } else {
      int ligth_speed = map (speed, 40, 70, 0, 255);
      analogWrite(10, ligth_speed);
   }
   return 0;
 
}
// --------------------------------------
// Function: read_slope
// --------------------------------------
int read_slope(){
   if ((request_received) && (!requested_answered)&&
      (0 == strcmp("SLP: REQ\n",request)) ){
        slope = 0;
        if (digitalRead(9) == HIGH) {
            slope = 1;
            requested_answered = true;
            sprintf(answer,"SLP:  UP\n");
        }
        if (digitalRead(8) == HIGH) {
            slope = -1;
            requested_answered = true;
            sprintf(answer,"SLP:DOWN\n");
        }
        else{
            requested_answered = true;
            sprintf(answer,"SLP:FLAT\n");
         }
      }
   return 0;   
}

// --------------------------------------
// Function: setup
// --------------------------------------
void setup()
{
   // Setup Serial Monitor
   Serial.begin(9600);

   pinMode(2, OUTPUT);
   pinMode(3, OUTPUT);
   pinMode(4, OUTPUT);
   pinMode(5, OUTPUT);
   pinMode(6, INPUT);
   pinMode(7, OUTPUT)
   pinMode(8, INPUT);
   pinMode(9, INPUT);
   pinMode(10, OUTPUT);
   pinMode(11, OUTPUT);
   pinMode(12, OUTPUT);
   pinMode(13, OUTPUT);
}


// --------------------------------------
// Function: loop
// --------------------------------------
void loop()
{
   double start = millis();
   switch (mode){
   case 0:
      comm_server();
      speed_req();
      gas_req();
      brake_req();
      mix_req();
      show_speed();
      read_slope();
      read_ligth();
      lamp_req();
      show_distance();
      movment_sensor();

   case 1:
      comm_server();
      speed_req();
      gas_req();
      brake_req();
      mix_req();
      show_speed();
      read_slope();
      read_ligth();
      lamp_req();
      show_distance();
      movment_sensor();

   case 2:
      comm_server();
      speed_req();
      gas_req();
      brake_req();
      mix_req();
      show_speed();
      read_slope();
      read_ligth();
      lamp_req();
      show_distance();
      movment_sensor();
   }


   double end = millis();
   delay(100-(end-start));
   double diff = end - start;
  // Serial.print(diff);
   //Serial.print("end - start:\n");

//  

}