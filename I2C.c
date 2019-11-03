#include <msp430.h>
#include <stdint.h>
#include <LEDConfig.h>
#include <stdbool.h>


unsigned char TXByteCtr;
unsigned int index;
int Step;
int light = 0;
bool initiated = false;
unsigned int initiatecount;

unsigned int current;
int counter;



int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
  P3SEL |= 0x03;                            // Assign I2C pins to USCI_B0
  UCB0CTL1 |= UCSWRST;                      // Enable SW reset
  UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
  UCB0CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
  UCB0BR0 = 3;                             // fSCL = SMCLK/3 = ~400kHz
  UCB0BR1 = 0;
  UCB0I2CSA = addresses[current];                         // Slave Address is 070h
  UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
  UCB0IE |= UCTXIE;                         // Enable TX interrupt


  TA0CCTL0 |= CCIE;                             // TACCR0 interrupt enabled
  TA0CCR0 = (65250-1)/20;
  TA0CTL |= TASSEL__SMCLK | MC__UP | ID__8  ;     // SMCLK, continuous mode


  __bis_SR_register(GIE);                // Enter LPM0, enable interrupts


  while (1)
  {
      if(initiated){
          TXByteCtr = 17;


          if(light == 0){
              lightLevel(0);
              clearDisplay();
              TA0CTL |= MC__UP;
          }
          else{
              UCB0I2CSA = addresses[current++];
              lightDisplay(light);
          }

          if(current>1){current = 0;}
      }
      else{
          TXByteCtr = 1;
          while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
          UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
      }

      while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
      UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
      __no_operation();                       // Remain in LPM0 until all data
                                              // is TX'd
  }
}

//------------------------------------------------------------------------------
// The USCIAB0TX_ISR is structured such that it can be used to transmit any
// number of bytes by pre-loading TXByteCtr with the byte count. Also, TXData
// points to the next byte to transmit.
//------------------------------------------------------------------------------
#pragma vector = USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void)
{
  switch(UCB0IV)
  {
  case 12:                                  // Vector 12: TXIFG

    if (TXByteCtr)                          // Send commands one byte at a time
    {
        switch(Step){

                  case 0:
                     turnOnOscillator();
                     Step++;
                     break;

                  case 1:
                    blinkRate(HT16K33_BLINK_OFF);
//                    blinkRate(HT16K33_BLINK_1HZ);

                     Step++;
                     break;

                  case 2:
                     setBrightness(1);
                     Step++;

                     initiatecount++;
                     if(initiatecount < 6){
                        UCB0I2CSA = addresses[initiatecount];
                        Step = 0;
                     }else{
                         initiated = true;
                     }
                     break;


                  case 3:
                     UCB0TXBUF = matrix[index];
                     index++;
                     if(index == 17){
                         index = 0;
                     }
                     break;


                  default:
                      break;
                  }
      TXByteCtr--;
    }
    else{
      if(Step == 3){
          TXByteCtr = 17;
      }
      UCB0CTL1 |= UCTXSTP;                  // I2C stop condition
      UCB0IFG &= ~UCTXIFG;                  // Clear USCI_B0 TX int flag

    }
  default: break;
  }
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void TIMER_A0_VECTOR(void){
    light = (light+1)%54;
}


