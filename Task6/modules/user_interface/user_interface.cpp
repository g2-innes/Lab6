//=====[Libraries]=============================================================

#include "mbed.h"
#include "arm_book_lib.h"

#include "user_interface.h"

#include "code.h"
#include "siren.h"
#include "smart_home_system.h"
#include "fire_alarm.h"
#include "date_and_time.h"
#include "temperature_sensor.h"
#include "gas_sensor.h"
#include "matrix_keypad.h"
#include "display.h"

//=====[Declaration of private defines]========================================

#define DISPLAY_REFRESH_TIME_MS 1000

//=====[Declaration of private data types]=====================================

//=====[Declaration and initialization of public global objects]===============

DigitalOut incorrectCodeLed(LED3);
DigitalOut systemBlockedLed(LED2);

//=====[Declaration of external public global variables]=======================

//=====[Declaration and initialization of public global variables]=============

char codeSequenceFromUserInterface[CODE_NUMBER_OF_KEYS];

//=====[Declaration and initialization of private global variables]============

static bool incorrectCodeState = OFF;
static bool systemBlockedState = OFF;

static bool codeComplete = false;
static int numberOfCodeChars = 0;
static bool systemPrompt = false;

//=====[Declarations (prototypes) of private functions]========================

static void userInterfaceMatrixKeypadUpdate();
static void incorrectCodeIndicatorUpdate();
static void systemBlockedIndicatorUpdate();

static void userInterfaceDisplayInit();
static void userInterfaceDisplayUpdate();

static void displayGas();
static void displayTemp();
static void startPrompt();

//=====[Implementations of public functions]===================================

void userInterfaceInit()
{
    incorrectCodeLed = OFF;
    systemBlockedLed = OFF;
    matrixKeypadInit( SYSTEM_TIME_INCREMENT_MS );
    systemPrompt = false;
    startPrompt();
}

void userInterfaceUpdate()
{
    userInterfaceMatrixKeypadUpdate();
    incorrectCodeIndicatorUpdate();
    systemBlockedIndicatorUpdate();
    userInterfaceDisplayUpdate();
}

bool incorrectCodeStateRead()
{
    return incorrectCodeState;
}

void incorrectCodeStateWrite( bool state )
{
    incorrectCodeState = state;
}

bool systemBlockedStateRead()
{
    return systemBlockedState;
}

void systemBlockedStateWrite( bool state )
{
    systemBlockedState = state;
}

bool userInterfaceCodeCompleteRead()
{
    return codeComplete;
}

void userInterfaceCodeCompleteWrite( bool state )
{
    codeComplete = state;
}

//=====[Implementations of private functions]==================================

static void userInterfaceMatrixKeypadUpdate()
{
    static int numberOfHashKeyReleased = 0;
    char keyReleased = matrixKeypadUpdate();

    if( keyReleased != '\0' ) {
        
        if (!systemPrompt){
            if (keyReleased == '*'){
                systemPrompt = true;
                userInterfaceDisplayInit();
            } 
            else{
                startPrompt();
                return;
        }}



        if (keyReleased == '2'){
            displayGas();
        }
        else if (keyReleased == '3'){
            displayTemp();
        }

        if( sirenStateRead() && !systemBlockedStateRead() ) {
            if( !incorrectCodeStateRead() ) {
                codeSequenceFromUserInterface[numberOfCodeChars] = keyReleased;
                numberOfCodeChars++;
                if ( numberOfCodeChars >= CODE_NUMBER_OF_KEYS ) {
                    codeComplete = true;
                    numberOfCodeChars = 0;
                }
            } else {
                if( keyReleased == '#' ) {
                    numberOfHashKeyReleased++;
                    if( numberOfHashKeyReleased >= 2 ) {
                        numberOfHashKeyReleased = 0;
                        numberOfCodeChars = 0;
                        codeComplete = false;
                        incorrectCodeState = OFF;
                    }
                }
            }
        }
    }
}

static void userInterfaceDisplayInit()
{
    displayInit( DISPLAY_CONNECTION_I2C_PCF8574_IO_EXPANDER );
     
    displayCharPositionWrite ( 0,0 );
    displayStringWrite( "Temperature:" );

    displayCharPositionWrite ( 0,1 );
    displayStringWrite( "Gas:" );
    
    displayCharPositionWrite ( 0,2 );
    displayStringWrite( "Alarm:" );
}

static void userInterfaceDisplayUpdate()
{
    static int accumulatedDisplayTime = 0;
    char temperatureString[3] = "";
    
    if (!systemPrompt){
        return;
    }

    if( accumulatedDisplayTime >=
        DISPLAY_REFRESH_TIME_MS ) {

        accumulatedDisplayTime = 0;

        sprintf(temperatureString, "%.0f", temperatureSensorReadCelsius());
        displayCharPositionWrite ( 12,0 );
        displayStringWrite( temperatureString );
        displayCharPositionWrite ( 14,0 );
        displayStringWrite( "'C" );

        displayCharPositionWrite ( 4,1 );

        if ( gasDetectedRead() ) {
            displayStringWrite( "Detected    " );
        } else {
            displayStringWrite( "Not Detected" );
        }
        
        displayCharPositionWrite ( 6,2 );
        if(gasDetectedRead()){
            sirenStateWrite(ON);
        }

        displayCharPositionWrite(0, 3);
        displayStringWrite("                   ");

        displayCharPositionWrite(0,3);
        if (gasDetectedRead()){
            displayStringWrite("Gas too high!");
        }

        displayCharPositionWrite(0,3);
        if (overTemperatureDetectedRead()){
            displayStringWrite("Temp too high!");
        }

        displayCharPositionWrite ( 6,2 );
        
        if ( sirenStateRead() ) {
            displayStringWrite( "ON " );
        } else {
            displayStringWrite( "OFF" );

            displayCharPositionWrite(0, 3);
            displayStringWrite("                ");
        }
        
    } else {
        accumulatedDisplayTime =
            accumulatedDisplayTime + SYSTEM_TIME_INCREMENT_MS;        
    } 
}

static void incorrectCodeIndicatorUpdate()
{
    incorrectCodeLed = incorrectCodeStateRead();
}

static void systemBlockedIndicatorUpdate()
{
    systemBlockedLed = systemBlockedState;
}

static void displayGas(){
    displayCharPositionWrite(0, 3);
    if (gasDetectedRead()){
        displayStringWrite("Gas Alarm Is Active");
    }
    else{
        displayStringWrite("Gas Alarm Is OFF");

    }
}

static void displayTemp(){
    displayCharPositionWrite(0, 3);
    if (overTemperatureDetectedRead()){
        displayStringWrite("Temp Alarm Is Active");
    }
    else{
        displayStringWrite("Temp Alarm Is OFF");

    }
}

static void startPrompt(){
    displayInit( DISPLAY_CONNECTION_I2C_PCF8574_IO_EXPANDER );
    displayCharPositionWrite(0, 0);
    displayStringWrite("                     ");
    displayStringWrite("Press * to start");

}