#include <msp430.h> 

/**
 * Define the P1.x pin for PWM
 *  - Use P1.6
 */
#define PWM_PIN BIT6

/**
 * Define the P1.x pin for input switch
 *  - Use P1.3
 */
#define SWITCH_PIN BIT3

/**
 * Initialize the clocks
 */
void initClock()
{
    // Setup for 1MHz
    DCOCTL = CAL_DCO_1MHZ;
    BCSCTL1 = CAL_BC1_1MHZ;
}

/**
 * Initialize the I/O pins
 */
void initPins()
{
    // Disable all output
    P1OUT = 0x00;
    P2OUT = 0x00;
    P3OUT = 0x00;

    // Set all pints output
    P1DIR = 0xff;
    P2DIR = 0xff;
    P3DIR = 0xff;

    // Enable all resistors
    P1REN = 0xff;
    P2REN = 0xff;
    P3REN = 0xff;

    // Set all edge selects to low -> high
    P1IES = 0x00;
    P2IES = 0x00;

    // Set all pins I/O mode
    P1SEL = 0x00;
    P1SEL2 = 0x00;
    P2SEL = 0x00;
    P2SEL2 = 0x00;
    P3SEL = 0x00;
    P3SEL2 = 0x00;

    // Reset all interrupt flags
    P1IFG = 0x00;
    P2IFG = 0x00;
}

/**
 * Setup the input pin for the PIR sensor
 */
void setupInputPin()
{
    // Set P1.3 as Input for PIR sensor
    P1DIR &= ~SWITCH_PIN;
    // Enable resister
    P1REN |= SWITCH_PIN;
    // Set High
    P1OUT |= SWITCH_PIN;
    // Set edge to high -> low
    P1IES |= SWITCH_PIN;
    // Clear the interrupt flag
    P1IFG &= ~SWITCH_PIN;
    // Enable interrupts
    P1IE |= SWITCH_PIN;
}

/**
 * Setup the PWM pin for motor output
 */
void setupPwmPin()
{
    // Use P1.6 for PWM output
    // Set P1.6 output
    P1DIR |= PWM_PIN;
    // Disable pup resistor
    P1REN &= ~PWM_PIN;
    // Set TA0.0 mode on P1.6
    P1SEL |= PWM_PIN;
}

/**
 * Setup the PWM timer registers
 */
void setupPwmTimer()
{
    // Set PWM period
    TA0CCR0 = 1000;
    // Set PWM duty cycle
    TA0CCR1 = 750;
    // Set output mode to 'Set/Reset'
    TA0CCTL1 = OUTMOD_7;
}

/**
 * main.c
 */
int main(void)
{
    // Stop watchdog
    WDTCTL = WDTPW | WDTHOLD;

    // Setup the clocks
    initClock();

    // Setup I/O pins
    initPins();

    // Setup the I/O for PIR sensor
    setupInputPin();

    // Setup the PWM pin
    setupPwmPin();

    // Setup the PWM timer
    setupPwmTimer();

    // Start timer
    // - Use SMCLK
    // - 'Up' mode
    TA0CTL = TASSEL_2 + MC_1;

    // Enter Low Power Mode 1
    // - CPU disabled
    // - SMCLK active
    // - MCLK disable
    // - DCO generator disabled
    __bis_SR_register(LPM1_bits + GIE);

}

/**
 * Interrupt function on input pin change
 */
#pragma vector=PORT1_VECTOR
__interrupt void PORT1_INTERRUPT(void)
{
    // If the pin is high
    if((P1IN & SWITCH_PIN) != 0)
    {
        // Enable PWM mode on output pin
        P1SEL |= PWM_PIN;
        // Set interrupt for high -> low (catch release)
        P1IES |= SWITCH_PIN;
    }
    else
    {
        // Disable PWM mode on output pin
        P1SEL &= ~PWM_PIN;
        // Set interrupt for low -> high (catch press)
        P1IES &= ~SWITCH_PIN;
    }
    // Reset the P1 interrupts
    P1IFG = 0x00;
}
