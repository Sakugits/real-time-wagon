// --------------------------------------
// Include files
// --------------------------------------
#include <string.h>
#include <stdio.h>
#include <Wire.h>

// --------------------------------------
// Global Constants
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

int sc_s = 0.1;

//ciclo secundario en milisegundos 
unsigned long sc_m = 100;
// tiempo de ejecucion en milisegundos del ciclo secundario
unsigned long sc_tiempo_ejecucion_m; 

unsigned long tiempo;
unsigned long tiempo_total;
unsigned long lag; 

int ciclo = 1 ;
int resto_cilco = 1;
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
// Function: ligth_req
// --------------------------------------
int ligth_req()
{
   // while there is enough data for a request
   if ( (request_received) &&
        (0 == strcmp("LIT: REQ\n",request)) ) {
     char ptr[5];
	  itoa(ligth, cstr, 10);
     
     if(ligth>=10){
       sprintf(answer,"LIT: %s%%\n",ptr);
     } else {
       sprintf(answer,"LIT: 0%s%%\n",ptr);
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
           sprintf(answer,"LAM:  OK"); 
        }

        if( (request_received) &&
        (0 == strcmp("LAM: CLR",request)) ){
           lamp = 0;
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
      request_received = false;
   }
   return 0;
}  
int slope_req(){
   if ((request_received) && (!requested_answered)){
      if (0 == strcmp("SLP: REQ\n",request)){
         if (slope == 1){
            sprintf(answer, "SLP:  UP\n");
         }
         if (slope == -1){
            sprintf(answer, "SLP:DOWN\n");
         }  
         if (slope == 0){
            sprintf(answer, "SLP:FLAT\n");
         }
      request_received = false;
      requested_answered = true;
      }
    
   }
   return 0;   
}
int gas_req(){
   if ((request_received) && (!requested_answered)){
      if (0 == strcmp("GAS: SET\n",request)){
         gas = 1;
         sprintf(answer, "GAS:  OK\n");
      }
      if(0 == strcmp("GAS: CLR\n", request)){
         gas = 0;
         sprintf(answer, "GAS:  OK\n", request);
      }
      request_received = false;
      requested_answered = true;
   }
      return 0;
}

int brake_req(){
   if ((request_received) && (!requested_answered)){
      if (0 == strcmp("BRK: SET\n",request)){
         brake = 1;
         sprintf(answer, "BRK:  OK\n");
      }
      if (0 == strcmp("BRK: CLR\n",request)){
         brake = 0;
         sprintf(answer, "BRK:  OK\n");
      }
      request_received = false;
      requested_answered = true;
   }
   return 0;  
}
int mix_req(){
   if ((request_received) && (!requested_answered)){
      if(0 == strcmp("MIX: SET\n",request)){
         mixer = 1;
         sprintf(answer, "MIX:  OK\n");
    
      }
      if(0 == strcmp("MIX: CLR\n",request)){
         mixer = 1;
         sprintf(answer, "MIX:  OK\n");
      }
      request_received = false;
      requested_answered = true;
   }
      return 0;
}
int arduino_distance(){
   int value = 0;
   value = analogRead(1);
   distance = map (value, 0, 1023, 10000, 90000);
   return 0;
}
int arduino_val_dis(){

  int value = 0;
  value = digitalRead(6); 
  if(value == 1 && pushed == 0) {
    pushed  = 1;
  }
  else if(pushed == 1 && value == 0){
     pushed = 0;
     if(mode == 0){
       mode = 1;
     } else if (mode == 2){
       mode = 0;
     }
  }

   return 0;
}

int ardduino_lamp()
{
 if (lamp == 1){
    digitalWrite(7, HIGH);
    return 0;
 }
 if (lamp == 0){
    digitalWrite(7, LOW);
    return 0;
 }
}
int arduino_ligth()
{
  int value = 0;
  value = analogRead(0);
  ligth = map (value, 0, 1023, 0, 99);
}

int arduino_gas() {
   if (gas == 0) {
      digitalWrite(13, LOW);
      return 0;
   }
   if (gas == 1){
      digitalWrite(13, HIGH);
      return 0;
   }
   return 0;
}

int arduino_slope() {
   slope = 0;
   if (digitalRead(9) == HIGH) {
      slope = 1;
      return 0;
   }
   if (digitalRead(8) == HIGH) {
      slope = -1;
      return 0;
   }
   return 0;  
}

int arduino_brk(){
   if(brake == 1){
      digitalWrite(12, HIGH);
   }
   if(brake == 0){
      digitalWrite(12, LOW);
   }
   return 0; 
}
int arduino_mixer() {
  if (mixer == 1) {
   digitalWrite(11, HIGH);
   }
  if (mixer == 0) {
   digitalWrite(11, LOW);
  }
  return 0;
}

int arduino_speed(){
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
   analogWrite(10, map(speed, 40, 70, 0, 255));
   return 0;
}


int all_requests(){
   speed_req();
   slope_req();
   gas_req();
   brake_req();
   ligth_req()
   lamp_req()
   mix_req();
   return 0;
}

int all_tasks(){
   arduino_gas();
   arduino_brk();
   arduino_mixer();
   arduino_slope();
   arduino_ligth();
   ardduino_lamp();
   return 0;
}


// --------------------------------------
// Function: setup
// --------------------------------------
void setup()
{
   // Setup Serial Monitor
   Serial.begin(9600);

   Wire.begin(SLAVE_ADDR);
  
  // Function to run when data received from master
   Wire.onReceive(comm_server);

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
   arduino_speed();
   all_tasks();
   all_requests();

   double end = millis();
   delay(100-(end-start));
   double diff = end - start;
  // Serial.print(diff);
   //Serial.print("end - start:\n");

//  

}