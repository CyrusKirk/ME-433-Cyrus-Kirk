/*
 * File:   testSpeed.c
 * Author: Jackson
 *
 * Created on March 31, 2015, 3:11 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include<xc.h> // processor SFR definitions
#include<sys/attribs.h> // __ISR macro

#pragma config DEBUG = OFF // no debugging - DONE
#pragma config JTAGEN = OFF // no jtag - DONE
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1 - DONE
#pragma config PWP = OFF // no write protect - DONE
#pragma config BWP = OFF // not boot write protect - DONE
#pragma config CP = OFF // no code protect - DONE

// DEVCFG1
#pragma config FNOSC = PRIPLL // use primary oscillator with pll - DONE
#pragma config FSOSCEN = OFF // turn off secondary oscillator - DONE
#pragma config IESO = OFF // no switching clocks - DONE
#pragma config POSCMOD = HS // high speed crystal mode - DONE
#pragma config OSCIOFNC = ON // free up secondary osc pins - DONE
#pragma config FPBDIV = DIV_1 // divide CPU freq by 1 for peripheral bus clock - DONE
#pragma config FCKSM = CSDCMD // do not enable clock switch - DONE
#pragma config WDTPS = PS1 // slowest wdt - DONE
#pragma config WINDIS = OFF // no wdt window - DONE
#pragma config FWDTEN = OFF // wdt off by default - DONE
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25% - DONE

// DEVCFG2 - get the CPU clock to 40MHz
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz - DONE
#pragma config FPLLMUL = MUL_20 // multiply clock after FPLLIDIV - DONE
#pragma config UPLLIDIV = DIV_2 // divide clock after FPLLMUL - DONE
#pragma config UPLLEN = ON // USB clock on - DONE
#pragma config FPLLODIV = DIV_1 // divide clock by 1 to output on pin - DONE

// DEVCFG3
#pragma config USERID = 0 // some 16bit userid
#pragma config PMDL1WAY = ON // not multiple reconfiguration, check this - DONE
#pragma config IOL1WAY = ON // not multimple reconfiguration, check this - DONE
#pragma config FUSBIDIO = ON // USB pins controlled by USB module - DONE
#pragma config FVBUSONIO = ON // controlled by USB module - DONE

int readADC(void);
/*
 *
 */
int main(int argc, char** argv) {
    __builtin_disable_interrupts();

    // set the CP0 CONFIG register to indicate that
    // kseg0 is cacheable (0x3) or uncacheable (0x2)
    // see Chapter 2 "CPU for Devices with M4K Core"
    // of the PIC32 reference manual
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // no cache on this chip!

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to be able to use TDI, TDO, TCK, TMS as digital
    DDPCONbits.JTAGEN = 0;

    __builtin_enable_interrupts();

    // Setting up pins
    TRISBbits.TRISB13 = 1; // 1 for input
    ANSELBbits.ANSB13 = 0; // 0 for digital, 1 for analog

    TRISBbits.TRISB7 = 0; // setting 7 as analog output

    RPB15Rbits.RPB15R = 0b0101; // set B15 to OC1
    T2CONbits.TCKPS = 0;
    PR2 = 39999;
    TMR2 = 0;
    OC1CONbits.OCM = 0b110;
    T2CONbits.ON = 1;
    OC1CONbits.ON = 1;
    OC1RS = 0;

    ANSELAbits.ANSA0 = 1;
    AD1CON3bits.ADCS = 3;
    AD1CHSbits.CH0SA = 0;
    AD1CON1bits.ADON = 1;

    int val;
    while (1) {
        _CP0_SET_COUNT(0); // set core timer to 0, remember it counts at half the CPU clock
        LATBINV = 0x80;//7th pin; // invert a pin

        // wait for half a second, setting LED brightness to pot angle while waiting
        while (_CP0_GET_COUNT() < 20000000) {
            val = readADC();
            OC1RS = val * 40000 / 1024;//133333;// PR2/3

            if (PORTBbits.RB13 == 1) {
                // nothing
            } else {
                LATBINV = 0x80;
            }
        }
    }
    return (EXIT_SUCCESS);
}

int readADC(void) {
    int elapsed = 0;
    int finishtime = 0;
    int sampletime = 20;
    int a = 0;

    AD1CON1bits.SAMP = 1;
    elapsed = _CP0_GET_COUNT();
    finishtime = elapsed + sampletime;
    while (_CP0_GET_COUNT() < finishtime) {
    }
    AD1CON1bits.SAMP = 0;
    while (!AD1CON1bits.DONE) {
    }
    a = ADC1BUF0;
    return a;
}

