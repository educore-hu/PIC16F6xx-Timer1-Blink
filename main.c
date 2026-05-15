/*
 * ============================================================================
 * File        : main.c
 * Author      : Educore
 * Target MCU  : PIC16F690
 * Compiler    : XC8
 * IDE         : MPLAB X IDE v5.00
 *
 * Description :
 * LED BLINK example using Timer1 Interrupt instead of blocking delays.
 *
 * Tested with:
 * - MPLAB X IDE v5.00
 * - XC8 Compiler
 * - PIC16F690
 * ============================================================================
 */
/*
 * ============================================================================
 * PIC16F690 - DIP20 PINOUT
 * ============================================================================
 *
 *                  +----\/----+
 *   VDD        --- |1      20| --- VSS
 *   RA5/T1CKI  --- |2      19| --- RA0/AN0
 *   RA4/AN3    --- |3      18| --- RA1/AN1
 *   RA3/MCLR   --- |4      17| --- RA2/AN2/VREF
 *   RC5        --- |5      16| --- RC0
 *   RC4        --- |6      15| --- RC1
 *   RC3        --- |7      14| --- RC2
 *   RC6/TX     --- |8      13| --- RB4
 *   RC7/RX     --- |9      12| --- RB5
 *   RB7        --- |10     11| --- RB6/ICSPCLK
 *                  +----------+
 *
 * Example in this project:
 * RB4 -> LED output Pin 13
 * LED resistor value 330 Ohm
 *
 * ============================================================================
 */

#include <xc.h>

//=============================================================================
// CONFIGURATION BITS
//=============================================================================
#pragma config FOSC  = INTRCIO   // Internal oscillator, RA4/RA5 usable as I/O
#pragma config WDTE  = OFF       // Watchdog Timer disabled
#pragma config PWRTE = OFF       // Power-up Timer disabled
#pragma config MCLRE = ON        // RA3/MCLR pin used as Reset
#pragma config CP    = OFF       // Program memory code protection OFF
#pragma config CPD   = OFF       // Data memory code protection OFF
#pragma config BOREN = ON        // Brown-out Reset enabled
#pragma config IESO  = OFF       // Internal/External Switchover disabled
#pragma config FCMEN = OFF       // Fail-Safe Clock Monitor disabled

//=============================================================================
// PARAMETERS
//=============================================================================
#define _XTAL_FREQ 4000000UL

// Globális változó az id?zítéshez
volatile unsigned int timer_counter = 0;
volatile char led_state = 0;

//=============================================================================
// INTERRUPT SERVICE ROUTINE (ISR)
//=============================================================================
void __interrupt() isr(void)
{
    // Timer1 Megszakítás ellen?rzése
    if (TMR1IF)
    {
        /* 
         * Timer1 újratöltése a pontos 100ms-hoz:
         * 4MHz / 4 = 1MHz instruction clock (1us per tick)
         * Prescaler 1:8 -> 8us per tick
         * 100ms / 8us = 12500 tick
         * 65536 - 12500 = 53036 (0xCECC)
         */
        TMR1H = 0xCE; 
        TMR1L = 0xCC;

        timer_counter++;
        
        // Itt kezeljük a villogást a számláló alapján
        // Ha pl. 1000ms kell (1 másodperc), akkor 10-ig számolunk (10 * 100ms)
        if (timer_counter >= 10) 
        {
            led_state = !led_state; // Állapotváltás
            PORTBbits.RB4 = led_state;
            timer_counter = 0;      // Számláló nullázása
        }

        TMR1IF = 0; // Flag törlése
    }
}

//=============================================================================
// MAIN PROGRAM
//=============================================================================
void main(void)
{
    // INTERNAL OSCILLATOR SETUP (4 MHz)
    OSCCON = 0b01100000;

    // DISABLE ANALOG FUNCTIONS
    ANSEL  = 0x00;
    ANSELH = 0x00;
    
    // PORT INITIALIZATION
    PORTB = 0x00;
    TRISBbits.TRISB4 = 0; // RB4 -> Output Pin 13

    //-------------------------------------------------------------------------
    // TIMER1 CONFIGURATION
    //-------------------------------------------------------------------------
    T1CONbits.TMR1CS = 0;   // Internal clock (Fosc/4)
    T1CONbits.T1CKPS = 0b11; // Prescaler 1:8
    
    // Kezd?érték beállítása (100ms-hoz)
    TMR1H = 0xCE;
    TMR1L = 0xCC;

    // Megszakítások engedélyezése
    TMR1IF = 0;             // Timer1 flag törlése
    TMR1IE = 1;             // Timer1 megszakítás engedélyezése
    PEIE = 1;               // Periféria megszakítás engedélyezése
    GIE = 1;                // Globális megszakítás engedélyezése

    T1CONbits.TMR1ON = 1;   // Timer1 indítása

    //-------------------------------------------------------------------------
    // MAIN LOOP
    //-------------------------------------------------------------------------
    while(1)
    {
        // Itt a processzor szabad! 
        // A villogás a háttérben (ISR-ben) történik.
        // Ide írhatsz bármi mást, nem fogja akasztani a késleltetés.
    }
}