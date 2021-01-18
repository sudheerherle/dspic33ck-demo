/*******************************************************************************
Copyright 2019 Microchip Technology Inc. (www.microchip.com)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*******************************************************************************/

/******************************************************************************
 Getting Started
 ------------------------------------------------------------------------------
 There is an attached readme.txt file attached to this project (and located in
 the root directory of the project) that describes how to run this demo. 
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "system.h"

#include "led1.h"
#include "led2.h"

#include "button_s1.h"
#include "button_s2.h"
#include "button_s3.h"

#include "led3_rgb.h"

#include "timer_1ms.h"

#include "adc.h"

#define WHITE_INTENSITY 0x8000

static void PrintHeader(void);
static void PrintData(void);

static uint16_t potentiometer;
static volatile bool button_s1_is_pressed;
static volatile bool button_s2_is_pressed;
static bool button_s3_is_pressed;

static uint16_t red = 0x4000;
static uint16_t green = 0x8000;
static uint16_t blue = 0xB000;
    
static char button_strings[2][12] = {
    { "Not Pressed" },
    { "Pressed    " }
};

static char color_strings[3][6] = {
    { "Red  " },
    { "Green" },
    { "Blue " }
};

//------------------------------------------------------------------------------
//Application related definitions
//------------------------------------------------------------------------------
#define BUTTON_DEBOUCE_TIME_MS      20

typedef enum
{
    BUTTON_COLOR_RED = 0,
    BUTTON_COLOR_GREEN = 1,
    BUTTON_COLOR_BLUE = 2
} BUTTON_COLOR;

//------------------------------------------------------------------------------
//Private prototypes
//------------------------------------------------------------------------------
static void ButtonDebounce(void);

//------------------------------------------------------------------------------
//Global variables
//------------------------------------------------------------------------------
static volatile BUTTON_COLOR buttonColor;

//------------------------------------------------------------------------------
//Main Function
//------------------------------------------------------------------------------
int main(void)
{    
    SYSTEM_Initialize();
    
    buttonColor = BUTTON_COLOR_RED;
   
    //Enable and configure the ADC so it can sample the potentiometer.
    ADC_SetConfiguration(ADC_CONFIGURATION_DEFAULT);
    ADC_ChannelEnable(ADC_CHANNEL_POTENTIOMETER);
    
    //Turn on a timer, so to generate periodic interrupts.
    TIMER_SetConfiguration(TIMER_CONFIGURATION_1MS);
    
    //Used for the LED modulation
    TIMER_RequestTick(LED3_RGB_TickHandler, 1);
    
    //Register the ButtonDebounce() callback function, so it gets called periodically
    //when the timer interrupts occur (in this case at 1:1 rate, so ButtonDebounce()
    //executes once per 1ms).
    TIMER_RequestTick(&ButtonDebounce, 1);
    
    PrintHeader();
    
    //Main loop
    while(1)
    {
        //Fetch an ADC sample from the potentiometer
        potentiometer = ADC_Read(ADC_CHANNEL_POTENTIOMETER);

        //Use the potentiometer ADC value to set the brightness of the currently
        //selected color channel on the RGB LED.  The "currently selected channel"
        //is manually selected by the user at runtime by pressing the pushbuttons.
        switch(buttonColor)
        {
            case BUTTON_COLOR_RED:
                red = potentiometer;
                break;

            case BUTTON_COLOR_GREEN:
                green = potentiometer;
                break;

            case BUTTON_COLOR_BLUE:
                blue = potentiometer;
                break;

            default:
                break;
        }

        button_s3_is_pressed = BUTTON_S3_IsPressed();
        
        if( button_s3_is_pressed == true)
        {
            //Set RGB LED color to white if S3 is pressed
            LED3_RGB_SetColor(WHITE_INTENSITY, WHITE_INTENSITY, WHITE_INTENSITY);
        }
        else
        {
            //Update the PWM values controlling the intensity of the RGB LED channels.
            LED3_RGB_SetColor(red, green, blue);
        }
        
        PrintData();

    }
}


//Helper function that advances the currently selected RGB color channel that
//is to be adjusted next.  This function is called in response to user pushbutton
//press events.
static void ChangeColor(void)
{         
    switch(buttonColor)
    {
        case BUTTON_COLOR_RED:
            buttonColor = BUTTON_COLOR_GREEN;
            break;

        case BUTTON_COLOR_GREEN:
            buttonColor = BUTTON_COLOR_BLUE;
            break;

        case BUTTON_COLOR_BLUE:
            buttonColor = BUTTON_COLOR_RED;
            break;

        default:
            buttonColor = BUTTON_COLOR_RED;
            break;
    }
}


//This callback function gets called periodically (1/1ms) by the timer interrupt event
//handler.  This function is used to periodically sample the pushbuttons and implement
//a debounce algorithm to reject spurious chatter that can occur during press events.
static void ButtonDebounce(void)
{
    static uint16_t debounceCounterS1 = 0;
    static uint16_t debounceCounterS2 = 0;
    
    //Sample the button S1 to see if it is currently pressed or not.
    if(BUTTON_S1_IsPressed())
    {
        //The button is currently pressed.  Turn on the general purpose LED.
        LED1_On();
        
        //Check if the debounce blanking interval has been satisfied.  If so,
        //advance the RGB color channel user control selector.
        if(debounceCounterS1 == 0)
        {
            ChangeColor();  
            button_s1_is_pressed = true;
        }
        
        //Reset the debounce countdown timer, so a new color change operation
        //won't occur until the button is released and remains continuously released 
        //for at least BUTTON_DEBOUCE_TIME_MS.
        debounceCounterS1 = BUTTON_DEBOUCE_TIME_MS;
    }
    else
    {
        //The button is not currently pressed.  Turn off the LED.
        LED1_Off();
        
        //Allow the debounce interval timer to count down, until it reaches 0.
        //Once it reaches 0, the button is effectively "re-armed".
        if(debounceCounterS1 != 0)
        {
            debounceCounterS1--;
        }
        else
        {
            button_s1_is_pressed = false;
        }
    }
       
    //Sample the button S2 to see if it is currently pressed or not.
    if(BUTTON_S2_IsPressed())
    {
        //The button is currently pressed.  Turn on the general purpose LED.
        LED2_On();
        
        //Check if the debounce blanking interval has been satisfied.  If so,
        //advance the RGB color channel user control selector.
        if(debounceCounterS2 == 0)
        {
            ChangeColor();   
            button_s2_is_pressed = true;
        }
        
        //Reset the debounce countdown timer, so a new color change operation
        //won't occur until the button is released and remains continuously released 
        //for at least BUTTON_DEBOUCE_TIME_MS.
        debounceCounterS2 = BUTTON_DEBOUCE_TIME_MS;
    }
    else
    {
        //The button is not currently pressed.  Turn off the LED.
        LED2_Off();
        
        //Allow the debounce interval timer to count down, until it reaches 0.
        //Once it reaches 0, the button is effectively "re-armed".
        if(debounceCounterS2 != 0)
        {
            debounceCounterS2--;
        }
        else
        {
            button_s2_is_pressed = false;
        }
    }    
}

/*******************************************************************************
 Required baud rate settings:
 - Baud Rate: 38400
 - Data: 8 bit
 - Parity: none
 - Stop: 1 stop bit
 - Flow Control: none
 ******************************************************************************/
static void PrintHeader(void)
{
    printf("\033[2J");      //Clear screen
    printf("\033[0;0f");    //return cursor to 0,0
    printf("\033[?25l");    //disable cursor
        
    printf("------------------------------------------------------------------\r\n");
    printf("dsPIC33CK Curiosity Development Board Demo                        \r\n");
    printf("------------------------------------------------------------------\r\n");
    printf("S1 - controls LED1\r\n");
    printf("S2 - controls LED2\r\n");
    printf("S3 - makes RGB LED turn white\r\n");
    printf("\r\n");
}

static void PrintData(void)
{
    printf("\033[8;0f");    //move cursor to row 8, column 0 (after header)

    printf("Potentiometer: %i/4095    \r\n", potentiometer>>4);
    printf("Button S1: %s\r\n", button_strings[button_s1_is_pressed]);
    printf("Button S2: %s\r\n", button_strings[button_s2_is_pressed]);
    printf("Button S3: %s\r\n", button_strings[button_s3_is_pressed]);
    printf("Actively controlled color: %s         \r\n", color_strings[buttonColor]);
    printf("  Red:   %i        \r\n", red>>4);
    printf("  Green: %i        \r\n", green>>4);
    printf("  Blue:  %i        \r\n", blue>>4);
}