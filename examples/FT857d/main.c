/*************************************************************************
 * FT857D CAT Library, by Pavel Milanes, CO7WT, pavelmc@gmail.com
 *
 * The goal of this lib is to act as a Yaesu FT-857D radio from the
 * CAT point of view, then you can talk with your sketch from the PC like
 * if it was a real radio via CAT commands; to command a DDS for example.
 *
 * This work was a need from my side for the arduino-arcs project
 * see it here https://github.com/pavelmc/arduino-arcs
 *
 * This code has been built with the review of various sources:
 * - James Buck, VE3BUX, FT857D arduino Lib [http://www.ve3bux.com]
 * - Hamlib source code
 * - FLRig source code
 * - Chirp source code
 *
 * You can always found the last version in https://github.com/pavelmc/FT857d/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * **************************************************************************/

/*
 * This is the example for the library, just upload it to a Uno and configure
 * your software with 57600 @ 8N1 and enjoy
 */

/*
 * converted to run on Raspberry Pi by Alan Johnston, KU2Y
 * writes VFO A and B frequencies to file /home/pi/CubeSatSim/frequency.txt 
 * checks to make sure in amateur radio 2m or 70cm band
*/

#include "../../src/ft857d.h"
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

ft857d radio = ft857d();

// variables
//long freq = 7110000;
long freqA = 144890000;
long freqB = 434900000;
bool ptt = false;
bool splitActive = false;
bool vfoAActive = true;
uint8_t mode = 0;
FILE *file_ptr;

// radio modes
#define MODE_LSB 00
#define MODE_USB 01
#define MODE_CW 02

// DEBUG flag, uncomment it if you want to test it by hand
#define DEBUG true

void update_frequency() {

    int freq_A, freq_B;
    file_ptr = fopen("/home/pi/CubeSatSim/frequency.txt", "w");

    if (file_ptr == NULL) {
        printf("Error opening file!\n");
        return;
    }

	if (freqA > 450000000)
		freq_A = 435200000;
	else if ((freqA < 420000000) && (freqA > 148000000))
		freq_A = 434700000;
	else if (freqA < 144000000)
		freq_A = 434600000;
    else 
        freq_A = freqA;

    if (freqB > 450000000)
		freq_B = 435200000;
	else if ((freqB < 420000000) && (freqB > 148000000))
		freq_B = 434700000;
	else if (freqB < 144000000)
		freq_B = 434600000;
    else 
        freq_B = freqB;
    
    if ((freqA != freq_A) || (freqB != freq_B))
        printf("Frequency out of bounds error!\n");
         
    fprintf(file_ptr, "%d %d\n", freq_A, freq_B);

    fclose(file_ptr);
}

// function to run when we must put radio on TX/RX
void catGoPtt(bool pttf) {
    // the var ptt follows the value passed, but you can do a few more thing here
    ptt = pttf;

    #if defined (DEBUG)
    // debug
    printf("PTT Status is: %d\n", ptt);
    #endif
}

// function to run when VFOs A/B are toggled
void catGoToggleVFOs() {
    // here we simply toggle the value
    vfoAActive = !vfoAActive;

    #if defined (DEBUG)
    // debug
     if (vfoAActive)
         printf("VFO A active\n");
     else
         printf("VFO B active\n");
    #endif
}

// function to set a freq from CAT
void catSetFreq(long f) {
    // the var freq follows the value passed, but you can do a few more thing here

#if defined (DEBUG)
    // debug
    printf("Set frequency ");
#endif
    
    if (vfoAActive) {
        freqA = f;
#if defined (DEBUG)
    // debug
        printf("VFO A freq is now: %d\n", freqA);
#endif
    } else {
        freqB = f;
#if defined (DEBUG)
    // debug
        printf("VFO B freq is now: %d\n", freqB);
#endif        
    }

    update_frequency();

}

// function to set the mode from the cat command
void catSetMode(uint8_t m) {
    // the var mode follows the value passed, but you can do a few more thing here
    mode = m;

    #if defined (DEBUG)
    // debug
    printf("Active VFO mode is: %d\n", mode);
    #endif
}

// function to pass the freq to the cat library
long catGetFreq() {
    // this must return the freq as an unsigned long in Hz, you must prepare it before
    long freq = 0;
    
    #if defined (DEBUG)
    // debug
   printf("Asked for frequency ", freq);
    #endif


    if (vfoAActive) {
        freq = freqA;
#if defined (DEBUG)
    // debug
        printf("returned VFO A freq: %d\n", freqA);
#endif
    } else {
        freq = freqB;
#if defined (DEBUG)
    // debug
        printf("returned VFO B freq: %d\n", freqB);
#endif        
    }

    // pass it away
    return freq;
}

// function to pass the mode to the cat library
uint8_t catGetMode() {
    // this must return the mode in the wat the CAT protocol expect it

    #if defined (DEBUG)
    // debug
    printf("Requested mode, returned %d\n", mode);
    #endif

    // pass it away
    return mode;
}

// function to pass the smeter reading in RX mode
uint8_t catGetSMeter() {
    // this must return a byte in with the 4 LSB are the S meter data
    // so this procedure must take care of convert your S meter and scale it
    // up to just 4 bits

    #if defined (DEBUG)
    // debug
    printf("Asked for S meter returned 4\n");
    #endif

    // pass it away (fixed here just for testing)
    return uint8_t(4);
}

// function to pass the TX status
uint8_t catGetTXStatus() {
    /*
     * this must return a byte in wich the different bits means this:
     * 0b abcdefgh
     *  a = 0 = PTT on
     *  a = 1 = PTT off
     *  b = 0 = HI SWR off
     *  b = 1 = HI SWR on
     *  c = 0 = split on
     *  c = 1 = split off
     *  d = dummy data
     *  efgh = PO meter data
     */

    #if defined (DEBUG)
    // debug
     printf("Asked for TX status ");
    #endif

    // you have to craft the byte from your data, we will built it from
    // our data
    uint8_t r = 0;
//    uint8_t r = 1;
    // we fix the TX power to half scale (8)
//    printf("Send: %x\n", (ptt==false)*128 + (splitActive==false)*32 + 8);
//    r = ptt<<7 + splitActive<<5 + 8;
    r = (ptt==false)*128 + (splitActive==false)*32 + 8;
    printf("returned %x \n",r);

    return r;
}


void setup() {
    // preload the vars in the cat library
    radio.addCATPtt(catGoPtt);
    radio.addCATAB(catGoToggleVFOs);
    radio.addCATFSet(catSetFreq);
    radio.addCATMSet(catSetMode);
    radio.addCATGetFreq(catGetFreq);
    radio.addCATGetMode(catGetMode);
    radio.addCATSMeter(catGetSMeter);
    radio.addCATTXStatus(catGetTXStatus);

    // now we activate the library
//    radio.begin(57600, SERIAL_8N1);
    radio.begin();

    #if defined (DEBUG)
    // serial welcome
     printf("FT857 emulation setup complete\n");
    #endif
//    sleep(1);

}

int main() {

    setup();
    while(1) {
        radio.check();
        sleep(0.1);
//        printf("Radio Checked ");
    }
}
