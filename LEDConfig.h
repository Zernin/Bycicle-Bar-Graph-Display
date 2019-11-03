#include <stdint.h>
#include <stdbool.h>
/*
 * LEDConfig.h
 *
 *  Created on: Oct 25, 2019
 *      Author: Jonathan
 */

#ifndef LEDCONFIG_H_
#define LEDCONFIG_H_


#define HT16K33_BLINK_OFF       0
#define HT16K33_BLINK_DISPLAYON 0x01
#define HT16K33_BLINK_CMD       0x80
#define HT16K33_CMD_BRIGHTNESS 0xE0
#define HT16K33_BLINK_2HZ  1
#define HT16K33_BLINK_1HZ  2
#define HT16K33_BLINK_HALFHZ  3

uint16_t matrix[17] = {0x00};
uint8_t addresses[] = {0x70,0x72,0x74,0x71,0x73,0x76};
bool lit[] = {false, false,false,false,false,false};
unsigned char i;

unsigned int current = 0;


void static writeData(uint8_t data){
    UCB0TXBUF = data;
}


void static writeRAM(){
    writeData(0x00);
}

void static blinkRate(uint8_t blink){
    if(blink > 3 ){blink = 0;}
    writeData(HT16K33_BLINK_CMD | HT16K33_BLINK_DISPLAYON | (blink << 1));
}



void setBrightness(uint8_t brightness) {
  if (brightness > 15) brightness = 15;
  writeData(HT16K33_CMD_BRIGHTNESS | brightness);
}

void turnOnOscillator(){
    writeData((0x20|1));
}

void static fullMatrix(){
    for(i = 1; i < 17; i++){
        matrix[i] = 0xFF;
    }
}

void static clearMatrix(){
    for(i = 1; i < 17; i++){
        matrix[i] = 0;
    }
}

void static lightLevel(int light){
    light = light % 9;
    if(light == 0){clearMatrix();}
    else{
        clearMatrix();
        for(i = 1; i < (light*2)+1; i++){
            matrix[i] = 0xFF;
        }
    }
}

void static lightAllBefore1(int index){
        lightLevel(8);
        for(i = 0; i < index; i++){
            if(!lit[i]){
                lit[i] = true;
                UCB0I2CSA = addresses[i];                         // Slave Address
                while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
                UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
            }
        }

    }


void static lightDisplay(int light){
    if(light > 54) {light = 54;}
    int lights = light   % 9;
    int display = light / 9;
    lightAllBefore1(display);
    lightLevel(lights);
    UCB0I2CSA = addresses[display];                         // Slave Address
//    while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
//    UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
}


void clearDisplay(){
    TA0CTL |= MC__STOP;
    for(i=0;i<6;i++){
        UCB0I2CSA = addresses[i];
        while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
        UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
    }

}



#endif /* LEDCONFIG_H_ */
