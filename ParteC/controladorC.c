//-Uncomment to compile with arduino support
//#define ARDUINO

//-------------------------------------
//-  Include files
//-------------------------------------
#include <termios.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/errno.h>
#include <sys/stat.h>

#include <rtems.h>
#include <rtems/termiostypes.h>
#include <bsp.h>

#include "displayC.h"

//-------------------------------------
//-  Constants
//-------------------------------------
#define MSG_LEN 9
#define SLAVE_ADDR 0x8
#define NS_IN_SEC 1E9

//Parte C
#define MODO_NORMAL 0
#define MODO_FRENADO 1
#define MODO_PARADA 2

//-------------------------------------
//-  Global Variables
//-------------------------------------
float speed = 0.0;
struct timespec time_msg = {0,400000000};
int fd_serie = -1;
//Parte A
int gas = 0;
int brake = 0;
int mix = 0;
int mixer_crono = 0;
//Parte B
int light = 0;
int light_val = 0;
//Parte C
int modo_actual = 0; //Empieza en modo normal
int distancia_limite = 11000;
int distancia_actual = 99999; //Para que no se piense que esté en la parada nada más empezar 

//-------------------------------------
//-  Function: read_msg
//-------------------------------------
int read_msg(int fd, char *buffer, int max_size)
{
    char aux_buf[MSG_LEN+1];
    int count=0;
    char car_aux;

    //clear buffer and aux_buf
    memset(aux_buf, '\0', MSG_LEN+1);
    memset(buffer, '\0', MSG_LEN+1);

    while (1) {
        car_aux='\0';
        read(fd_serie, &car_aux, 1);
        // skip if it is not valid character
        if ( ( (car_aux < 'A') || (car_aux > 'Z') ) &&
             ( (car_aux < '0') || (car_aux > '9') ) &&
               (car_aux != ':')  && (car_aux != ' ') &&
               (car_aux != '\n') && (car_aux != '.') &&
               (car_aux != '%') ) {
            continue;
        }
        // store the character
        aux_buf[count] = car_aux;

        //increment count in a circular way
        count = count + 1;
        if (count == MSG_LEN) count = 0;

        // if character is new_line return answer
        if (car_aux == '\n') {
           int first_part_size = strlen(&(aux_buf[count]));
           memcpy(buffer,&(aux_buf[count]), first_part_size);
           memcpy(&(buffer[first_part_size]),aux_buf,count);
           return 0;
        }
    }
    strncpy(buffer,"MSG: ERR\n",MSG_LEN);
    return 0;
}

//-------------------------------------
//-  Function: task_speed
//-------------------------------------
int task_speed()
{
    char request[MSG_LEN+1];
    char answer[MSG_LEN+1];

    //--------------------------------
    //  request speed and display it
    //--------------------------------

    //clear request and answer
    memset(request, '\0', MSG_LEN+1);
    memset(answer, '\0', MSG_LEN+1);

    // request speed
    strcpy(request, "SPD: REQ\n");

#if defined(ARDUINO)
    // use UART serial module
    write(fd_serie, request, MSG_LEN);
    nanosleep(&time_msg, NULL);
    read_msg(fd_serie, answer, MSG_LEN);
#else
    //Use the simulator
    simulator(request, answer);
#endif

    // display speed
    if (1 == sscanf (answer, "SPD:%f\n", &speed)){
        displaySpeed(speed);
    }
    return 0;
}

//-------------------------------------
//-  Function: task_slope
//-------------------------------------
int task_slope()
{
    char request[MSG_LEN+1];
    char answer[MSG_LEN+1];

    //--------------------------------
    //  request slope and display it
    //--------------------------------

    //clear request and answer
    memset(request,'\0',MSG_LEN+1);
    memset(answer,'\0',MSG_LEN+1);

    // request slope
    strcpy(request, "SLP: REQ\n");

#if defined(ARDUINO)
    // use UART serial module
    write(fd_serie, request, MSG_LEN);
    nanosleep(&time_msg, NULL);
    read_msg(fd_serie, answer, MSG_LEN);
#else
    //Use the simulator
    simulator(request, answer);
#endif

    // display slope
    if (0 == strcmp(answer, "SLP:DOWN\n")) displaySlope(-1);
    if (0 == strcmp(answer, "SLP:FLAT\n")) displaySlope(0);
    if (0 == strcmp(answer, "SLP:  UP\n")) displaySlope(1);

    return 0;
}

//-------------------------------------
//-  Function: task_gas
//-------------------------------------
int task_gas ()
{
    char request[MSG_LEN+1];
    char answer[MSG_LEN+1];

    memset(request,'\0',MSG_LEN+1);
    memset(answer,'\0',MSG_LEN+1);

    //Gas function

    if (speed >  55)
    {
        strcpy(request, "GAS: CLR\n");
        gas = 0;
    }
    else 
    {
        strcpy(request, "GAS: SET\n");
        gas = 1;
    }

    #if defined(ARDUINO)
        // use UART serial module
        write(fd_serie, request, MSG_LEN);
        nanosleep(&time_msg, NULL);
        read_msg(fd_serie, answer, MSG_LEN);
    #else
        //Use the simulator
        simulator(request, answer);
    #endif

        if (0 == strcmp(answer, "GAS:  OK\n"))
        displayGas(gas);

    return 0;
}

//-------------------------------------
//-  Function: task_brake
//-------------------------------------
int task_brake ()
{
    char request[MSG_LEN+1];
    char answer[MSG_LEN+1];

    memset(request,'\0',MSG_LEN+1);
    memset(answer,'\0',MSG_LEN+1);

    //Brake Function

    if (speed > 55)
    {
        strcpy(request, "BRK: SET\n");
    	brake = 1;
    }
    else
    {
        strcpy(request, "BRK: CLR\n");
    	brake = 0;
    }

    #if defined(ARDUINO)
        // use UART serial module
        write(fd_serie, request, MSG_LEN);
        nanosleep(&time_msg, NULL);
        read_msg(fd_serie, answer, MSG_LEN);
    #else
        //Use the simulator
        simulator(request, answer);
    #endif

        if (0 == strcmp(answer, "BRK:  OK\n")) 
        displayBrake(brake);
    
    return 0;
}


//-------------------------------------
//-  Function: task_mixer
//-------------------------------------

int task_mixer ()
{
    char request[MSG_LEN+1];
    char answer[MSG_LEN+1];

    memset(request,'\0',MSG_LEN+1);
    memset(answer,'\0',MSG_LEN+1);

    //Mixer Function

    if (mixer_crono >= 40) //Cuando llegamos a los 40 s se hacen cosas, valor válido al estar en 30 y 60 secs
    {
        if (mix == 0) // Hora de trabajar
        {
            strcpy(request, "MIX: SET\n");
            mix = 1;
        }

        else // Hora de descansar
        {
            strcpy(request, "MIX: CLR\n");
            mix = 0;
        }

        mixer_crono = 0; //En otros 40 secs volvemos aquí
    }


    #if defined(ARDUINO)
        // use UART serial module
        write(fd_serie, request, MSG_LEN);
        nanosleep(&time_msg, NULL);
        read_msg(fd_serie, answer, MSG_LEN);
    #else
        //Use the simulator
        simulator(request, answer);
    #endif

    if (0 == strcmp(answer, "MIX:  OK\n"))
    displayMix(mix);

    return 0;
}

//-------------------------------------
//-  Function: light_sensor
//-------------------------------------
int task_ligth_sensor()
{
    char request[MSG_LEN+1];
    char answer[MSG_LEN+1];

    memset(request,'\0',MSG_LEN+1);
    memset(answer,'\0',MSG_LEN+1);

    // Vemos el nivel de luminosidad del entorno
    strcpy(request, "LIT: REQ\n");

    #if defined(ARDUINO)
        // use UART serial module
        write(fd_serie, request, MSG_LEN);
        nanosleep(&time_msg, NULL);
        read_msg(fd_serie, answer, MSG_LEN);
    #else
        //Use the simulator
        simulator(request, answer);
    #endif
    if (1 == sscanf (answer, "LIT: %i\n", &light_val)){
        
        if(light_val <= 50) //Se encienden los focos
        {
            light=1;
        }
        else
        {
            light=0;
        }
        
        displayLightSensor(light);
    }
    
    return 0;
}

//-------------------------------------
//-  Function: task_lamp
//-------------------------------------

int task_lamp()
{
    char request[MSG_LEN+1];
    char answer[MSG_LEN+1];

    memset(request,'\0',MSG_LEN+1);
    memset(answer,'\0',MSG_LEN+1);

    //Lamp function

    if (light == 1)
    {
    	strcpy(request, "LAM: SET\n");
    }
    else
    {
    	strcpy(request, "LAM: CLR\n");
    }

    #if defined(ARDUINO)
        // use UART serial module
        write(fd_serie, request, MSG_LEN);
        nanosleep(&time_msg, NULL);
        read_msg(fd_serie, answer, MSG_LEN);
    #else
        //Use the simulator
        simulator(request, answer);
    
    #endif

        if (0 == strcmp(answer, "LAM:  OK\n"))
        displayLamps(light);
    
    return 0;
}

int task_check_current_distance()
{
    char request[MSG_LEN+1];
    char answer[MSG_LEN+1];

    memset(request,'\0',MSG_LEN+1);
    memset(answer,'\0',MSG_LEN+1);

    //Pedimos la distancia
    strcpy(request, "DS:  REQ\n");

    #if defined(ARDUINO)
        // use UART serial module
        write(fd_serie, request, MSG_LEN);
        nanosleep(&time_msg, NULL);
        read_msg(fd_serie, answer, MSG_LEN);
    #else
        //Use the simulator
        simulator(request, answer);
    
    #endif
        
        if (1 == sscanf (answer, "DS:%i\n", &distancia_actual))
        {
            if(distancia_actual <= 0)
            {
                modo_actual = MODO_PARADA;
            }

            else if(distancia_actual < distancia_limite)
            {
                modo_actual = MODO_FRENADO;
            }

            displayDistance(distancia_actual);
	    }
    
    return 0;
}

// Modo Frenado Tasks

int task_gas_modo_frenado ()
{
    char request[MSG_LEN+1];
    char answer[MSG_LEN+1];

    memset(request,'\0',MSG_LEN+1);
    memset(answer,'\0',MSG_LEN+1);

    //Gas function

    if (speed >  2.5)
    {
        strcpy(request, "GAS: CLR\n");
        gas = 0;
    }
    else 
    {
        strcpy(request, "GAS: SET\n");
        gas = 1;
    }

    #if defined(ARDUINO)
        // use UART serial module
        write(fd_serie, request, MSG_LEN);
        nanosleep(&time_msg, NULL);
        read_msg(fd_serie, answer, MSG_LEN);
    #else
        //Use the simulator
        simulator(request, answer);
    #endif

        if (0 == strcmp(answer, "GAS:  OK\n"))
        displayGas(gas);

    return 0;
}

int task_brake_modo_frenado ()
{
    char request[MSG_LEN+1];
    char answer[MSG_LEN+1];

    memset(request,'\0',MSG_LEN+1);
    memset(answer,'\0',MSG_LEN+1);

    //Brake Function

    if (speed > 2.5)
    {
        strcpy(request, "BRK: SET\n");
    	brake = 1;
    }
    else
    {
        strcpy(request, "BRK: CLR\n");
    	brake = 0;
    }

    #if defined(ARDUINO)
        // use UART serial module
        write(fd_serie, request, MSG_LEN);
        nanosleep(&time_msg, NULL);
        read_msg(fd_serie, answer, MSG_LEN);
    #else
        //Use the simulator
        simulator(request, answer);
    #endif

        if (0 == strcmp(answer, "BRK:  OK\n")) 
        displayBrake(brake);
    
    return 0;
}

int task_lamp_not_normal()
{
    char request[MSG_LEN+1];
    char answer[MSG_LEN+1];

    memset(request,'\0',MSG_LEN+1);
    memset(answer,'\0',MSG_LEN+1);

    strcpy(request, "LAM: SET\n"); //Focos encendidos todo el tiempo

    #if defined(ARDUINO)
        // use UART serial module
        write(fd_serie, request, MSG_LEN);
        nanosleep(&time_msg, NULL);
        read_msg(fd_serie, answer, MSG_LEN);
    #else
        //Use the simulator
        simulator(request, answer);
    
    #endif

        if (0 == strcmp(answer, "LAM:  OK\n"))
        displayLamps(light);
    
    return 0;
}

//Modo Parada tasks

int task_start_moving_again ()
{
    char request[MSG_LEN+1];
    char answer[MSG_LEN+1];

    memset(request,'\0',MSG_LEN+1);
    memset(answer,'\0',MSG_LEN+1);

    strcpy(request, "STP: REQ\n"); //Ver distancia

    #if defined(ARDUINO)
        // use UART serial module
        write(fd_serie, request, MSG_LEN);
        nanosleep(&time_msg, NULL);
        read_msg(fd_serie, answer, MSG_LEN);
    #else
        //Use the simulator
        simulator(request, answer);
    
    #endif

        if (0 == strcmp(answer, "STP:  GO\n")){
            modo_actual = MODO_NORMAL;
            displayStop(0);
        }

        else if (0 == strcmp(answer, "STP:STOP\n")){
            displayStop(1);
        }
        
    return 0;
        
}

//--------------- MODOS DE EJECUCIÓN --------------//

void modo_normal ()
{
    long time_passed = 0;
    int secondary_cycle_counter = 0;
    struct timespec start, end;

    while (modo_actual == MODO_NORMAL)
    {
        clock_gettime(CLOCK_MONOTONIC, &start);

        switch (secondary_cycle_counter) //CP = 10, CS = 5
        {
        case 0:
            task_ligth_sensor();
            task_lamp ();
            task_mixer ();
            task_speed ();
            task_slope ();
            break;
        
        case 1:
            task_ligth_sensor();
            task_lamp();
            task_gas();
            task_brake();
            task_check_current_distance();
            break;
        }

        secondary_cycle_counter = (secondary_cycle_counter + 1) % 2;
        clock_gettime(CLOCK_MONOTONIC,&end);
        time_passed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec)/NS_IN_SEC;
        sleep(5 - time_passed);
        mixer_crono += 5;

    }
}

void modo_frenado () 
{
    long time_passed = 0;
    int secondary_cycle_counter = 0;
    struct timespec start, end;

    while (modo_actual == MODO_FRENADO)
    {
        clock_gettime(CLOCK_MONOTONIC, &start);

        switch (secondary_cycle_counter) //CP = 20, CS = 5
        {
        case 0:
            task_mixer();
            task_speed();
            task_slope();
            task_gas_modo_frenado();
            task_brake_modo_frenado();
            break;
        
        case 1:
            task_speed();
            task_gas_modo_frenado();
            task_brake_modo_frenado();
            task_check_current_distance();
            task_lamp_not_normal();
            break;
        
        case 2:
            task_speed();
            task_gas_modo_frenado();
            task_brake_modo_frenado();
            task_mixer();
            task_slope();
            break;
        
        case 3:
            task_speed();
            task_gas_modo_frenado();
            task_brake_modo_frenado();
            task_check_current_distance();
            break;
        }

        secondary_cycle_counter = (secondary_cycle_counter + 1) % 4;
        clock_gettime(CLOCK_MONOTONIC,&end);
        time_passed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec)/NS_IN_SEC;
        sleep(5 - time_passed);
        mixer_crono += 5;

    }

}

void modo_parada()
{
    long time_passed = 0;
    struct timespec start, end;

    while (modo_actual == MODO_PARADA) //CP = CS = 5
    {
        clock_gettime(CLOCK_MONOTONIC, &start);

        task_start_moving_again();
        task_mixer();
        task_lamp_not_normal();

        clock_gettime(CLOCK_MONOTONIC,&end); //Fin tiempo del bucle
        time_passed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec)/NS_IN_SEC; // Segundos + nanosegundos
        sleep (5 - time_passed); //Duermo el bucle hasta terminar el periodo, para evitar acarreo de errores
        mixer_crono += 5;
    }
    
}


//-------------------------------------
//-  Function: controller
//-------------------------------------
void *controller(void *arg)
{
    // Endless loop
    while(1) {

        switch (modo_actual)
        {
        case MODO_NORMAL:
            modo_normal();
            break;
        
        case MODO_FRENADO:
            modo_frenado();
            break;
        
        case MODO_PARADA:
            modo_parada();
            break;
    
        }
        
    }
}

//-------------------------------------
//-  Function: Init
//-------------------------------------
rtems_task Init (rtems_task_argument ignored)
{
    pthread_t thread_ctrl;
    sigset_t alarm_sig;
    int i;

    /* Block all real time signals so they can be used for the timers.
       Note: this has to be done in main() before any threads are created
       so they all inherit the same mask. Doing it later is subject to
       race conditions */
    sigemptyset (&alarm_sig);
    for (i = SIGRTMIN; i <= SIGRTMAX; i++) {
        sigaddset (&alarm_sig, i);
    }
    sigprocmask (SIG_BLOCK, &alarm_sig, NULL);

    // init display
    displayInit(SIGRTMAX);

#if defined(ARDUINO)
    /* Open serial port */
    char serial_dev[]="/dev/com1";
    fd_serie = open (serial_dev, O_RDWR);
    if (fd_serie < 0) {
        printf("open: error opening serial %s\n", serial_dev);
        exit(-1);
    }

    struct termios portSettings;
    speed_t speed=B9600;

    tcgetattr(fd_serie, &portSettings);
    cfsetispeed(&portSettings, speed);
    cfsetospeed(&portSettings, speed);
    cfmakeraw(&portSettings);
    tcsetattr(fd_serie, TCSANOW, &portSettings);
#endif

    /* Create first thread */
    pthread_create(&thread_ctrl, NULL, controller, NULL);
    pthread_join (thread_ctrl, NULL);
    exit(0);
}

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_MAXIMUM_TASKS 1
#define CONFIGURE_MAXIMUM_SEMAPHORES 10
#define CONFIGURE_MAXIMUM_FILE_DESCRIPTORS 30
#define CONFIGURE_MAXIMUM_DIRVER 10
#define CONFIGURE_MAXIMUM_POSIX_THREADS 2
#define CONFIGURE_MAXIMUM_POSIX_TIMERS 1

#define CONFIGURE_INIT
#include <rtems/confdefs.h>
