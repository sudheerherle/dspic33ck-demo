/*******************************************************************************
Copyright 2016 Microchip Technology Inc. (www.microchip.com)

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
#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

/* This module uses 3 dimmable LEDs (red, green, blue).  It uses them to
 * implement one color changing LED.
 */

#define LED_ON  1
#define LED_OFF 0

#define INPUT  1
#define OUTPUT 0

struct LED {
    uint16_t intensity;
    uint16_t modulation_count;
    unsigned int next_state;
    void (*pin_control)(unsigned int state);
};

static void LED3_RED_PinControl(unsigned int state);
static void LED3_GREEN_PinControl(unsigned int state);
static void LED3_BLUE_PinControl(unsigned int state);

static struct LED red = {0, 0, LED_OFF, &LED3_RED_PinControl};
static struct LED green = {0, 0, LED_OFF, &LED3_GREEN_PinControl};
static struct LED blue = {0, 0, LED_OFF, &LED3_BLUE_PinControl};

#define DEFAULT_INTENSITY 0x2000

void LED3_RED_SetIntensity(uint16_t new_intensity)
{  
    red.intensity = new_intensity >> 8;
}

void LED3_RED_On(void)
{
    LED3_RED_SetIntensity(DEFAULT_INTENSITY);
}

void LED3_RED_Off(void)
{
    LED3_RED_SetIntensity(0);
}

void LED3_RED_Toggle(void)
{
	if(red.intensity == 0)
    {
        LED3_RED_SetIntensity(DEFAULT_INTENSITY);
    }
    else
    {
        LED3_RED_SetIntensity(0);
    }
}

static void LED3_RED_PinControl(unsigned int state)
{
    _TRISE15 = 0;
    _LATE15 = state;
}

void LED3_GREEN_SetIntensity(uint16_t new_intensity)
{  
    green.intensity = new_intensity >> 8;
}

void LED3_GREEN_On(void)
{
    LED3_GREEN_SetIntensity(DEFAULT_INTENSITY);
}

void LED3_GREEN_Off(void)
{
    LED3_GREEN_SetIntensity(0);
}

void LED3_GREEN_Toggle(void)
{
	if(green.intensity == 0)
    {
        LED3_GREEN_SetIntensity(DEFAULT_INTENSITY);
    }
    else
    {
        LED3_GREEN_SetIntensity(0);
    }
}

static void LED3_GREEN_PinControl(unsigned int state)
{
    _TRISE14 = 0;
    _LATE14 = state;
}

void LED3_BLUE_SetIntensity(uint16_t new_intensity)
{      
    blue.intensity = new_intensity >> 8;
}

void LED3_BLUE_On(void)
{
    LED3_BLUE_SetIntensity(DEFAULT_INTENSITY);
}

void LED3_BLUE_Off(void)
{
    LED3_BLUE_SetIntensity(0);
}

void LED3_BLUE_Toggle(void)
{
	if(blue.intensity == 0)
    {
        LED3_BLUE_SetIntensity(DEFAULT_INTENSITY);
    }
    else
    {
        LED3_BLUE_SetIntensity(0);
    }
}

static void LED3_BLUE_PinControl(unsigned int state)
{
    _TRISE13 = 0;
    _LATE13 = state;
}

void LED3_RGB_On(void)
{
    LED3_RED_On();
    LED3_BLUE_On();
    LED3_GREEN_On();  
}

void LED3_RGB_Off(void)
{
    LED3_RED_Off();
    LED3_BLUE_Off();
    LED3_GREEN_Off();  
}

void LED3_RGB_Toggle(void)
{
    LED3_RED_Toggle();
    LED3_BLUE_Toggle();
    LED3_GREEN_Toggle();  
}

void LED3_RGB_SetColor(uint16_t red, uint16_t green, uint16_t blue)
{
    LED3_RED_SetIntensity(red);
    LED3_GREEN_SetIntensity(green);
    LED3_BLUE_SetIntensity(blue);
}


//https://www.embeddedrelated.com/showarticle/107.php
#define FULL_SCALE_MODULATION_COUNT 285
#define MIN_MODULATION_COUNT 18

static void ModulateLEDNextStateSet(struct LED *led)
{
    uint16_t count_change = led->intensity;
    
    if(count_change < MIN_MODULATION_COUNT)
    {       
        if(count_change > (MIN_MODULATION_COUNT>>1))
        {
            count_change = MIN_MODULATION_COUNT;
        }
        else
        {
            count_change = 0;
        }
    }
    
    led->modulation_count += count_change;
    if(led->modulation_count < FULL_SCALE_MODULATION_COUNT)
    {
        led->next_state = LED_OFF;
    }
    else
    {
        led->next_state = LED_ON;
        led->modulation_count -= FULL_SCALE_MODULATION_COUNT;
    }
}

static void SetLEDState(struct LED *led)
{
    led->pin_control(led->next_state);
}

void LED3_RGB_TickHandler(void)
{
    SetLEDState(&red);
    SetLEDState(&blue);
    SetLEDState(&green);
    
    ModulateLEDNextStateSet(&red);
    ModulateLEDNextStateSet(&blue);
    ModulateLEDNextStateSet(&green);
}


