// --------------------------------------
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
   comm_server();
   speed_req();
   gas_req();
   brake_req();
   mix_req();
   show_speed();
   read_slope();

   double end = millis();
   delay(200-(end-start));
   double diff = end - start;
   //Serial.println(diff);

}