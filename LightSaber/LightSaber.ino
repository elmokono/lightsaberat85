/* Audio Sample Player v2

   David Johnson-Davies - www.technoblogy.com - 23rd October 2017
   ATtiny85 @ 8 MHz (internal oscillator; BOD disabled)

   CC BY 4.0
   Licensed under a Creative Commons Attribution 4.0 International license:
   http://creativecommons.org/licenses/by/4.0/
*/

/* Direct-coupled capacitorless output */

#include <avr/pgmspace.h>
#include <avr/sleep.h>
#define adc_disable()  (ADCSRA &= ~(1<<ADEN))
#define adc_enable()  (ADCSRA |= ~(1<<ADEN))
#include "holagenarosound2.h"

unsigned int p = 0, state = 0;
int c = 0;

void setup () {
  // Enable 64 MHz PLL and use as source for Timer1
  PLLCSR = 1 << PCKE | 1 << PLLE;

  // Set up Timer/Counter1 for PWM output
  TIMSK = 0;                              // Timer interrupts OFF
  TCCR1 = 1 << PWM1A | 2 << COM1A0 | 1 << CS10; // PWM A, clear on match, 1:1 prescale
  GTCCR = 1 << PWM1B | 2 << COM1B0;       // PWM B, clear on match
  OCR1A = 128; OCR1B = 128;               // 50% duty at start

  // Set up Timer/Counter0 for 8kHz interrupt to output samples.
  TCCR0A = 3 << WGM00;                    // Fast PWM
  TCCR0B = 1 << WGM02 | 2 << CS00;        // 1/8 prescale
  TIMSK = 1 << OCIE0A;                    // Enable compare match
  OCR0A = 124;                            // Divide by 1000

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  pinMode(1, OUTPUT); //speaker -
  pinMode(2, INPUT); //switch
  pinMode(3, OUTPUT); //transistor lights
  pinMode(4, OUTPUT); //speaker +

  digitalWrite(3, LOW); //turn off lights
  state = 0;
  p = 0;
}

void loop () {
}

void(* resetFunc) (void) = 0; //declare reset function @ address 0

// Sample interrupt
ISR (TIMER0_COMPA_vect) {

  int val = digitalRead(2);

  if (val == LOW && state == 0 && p == 0 && c == 0) {
    state = 1; //turn on
    digitalWrite(3, HIGH); //turn on lights
  }

  if (state == 1 && p <= quack_wav_len) {
    char sample = pgm_read_byte(&quack_wav[p++]);
    OCR1A = sample;
    OCR1B = sample ^ 255;
  }

  if (val == LOW && state == 1 && p >= quack_wav_len) {
    digitalWrite(3, LOW); //turn off lights
    state = 0;
    p = 0;
    c = -9999;
 }

  if (c < 0) c++;
}
