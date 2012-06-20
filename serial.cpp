/*****************************************************************************
*
* Copyright (C) 2010 Circuits At Home, LTD. All rights reserved.
*
* This software may be distributed and modified under the terms of the GNU
* General Public License version 2 (GPL) as published by the Free Software
* Foundation and appearing in the file GPL.TXT included in the packaging of
* this file. Please note that GPL Section 2[b] requires that all works based
* on this software must also be made publicly available under the terms of
* the GPL ("Copyleft").
*
* Contact information:
* Circuits At Home Web site:  http://www.circuitsathome.com
* e-mail:                     support@circuitsathome.com
*****************************************************************************/
#include "bt_h.h"
#include "serial.h"

#include <avr/pgmspace.h>

int8_t inbuf[INBUF_SIZE];
uint16_t incount = 0;
const int8_t uname[] PROGMEM = "BT terminal v1.0";
const int8_t srvst[] PROGMEM = "Server started... ";
const int8_t conn[] PROGMEM = "Connecting... ";
const int8_t connto[] PROGMEM = "Connecting to";
const int8_t mybd[] PROGMEM = "My BDADDR";
const int8_t welcome[] PROGMEM = "Press:\r\nS - to start server\r\nC - to connect as a client\r\nA - to connect to predefined address\r\nM - to show local BD-address";

extern TBT BT;
extern uint8_t state;

///////////////////////////////////////////////////////////////////////////////////////
void (*soft_reset)(void) = 0x0000; // Set up function pointer to RESET vector.


///////////////////////////////////////////////////////////////////////////////////////
uint8_t InbufAnalyse(uint16_t Aincount, int8_t *Ainbuf){ 
    uint8_t res, i;
    Tbd_addr addrA, my_addr;
    addrA.bytes[5] = 0x00;
    addrA.bytes[4] = 0x11;
    addrA.bytes[3] = 0x67;
    addrA.bytes[2] = 0xd6;
    addrA.bytes[1] = 0xab;
    addrA.bytes[0] = 0xc7;

    
    res = 0;
    if(BT.GetState() != BT_STATE_CONNECTED){
        if(Aincount == 1){
            if(Ainbuf[0] == 'S') {
                BT.InitServer(); 
                state = 1; 
                dbgn(srvst);
                res = 1;
            } else if(Ainbuf[0] == 'C') {
                BT.InitClient(); 
                state = 1; 
                dbgn(conn);
                res = 1;
            } else if(Ainbuf[0] == 'A') {
                BT.InitClientAddr(addrA); 
                state = 1; 
                dbgn(connto);
                for(i=0;i<6;i++){
                      Serial.print(":");
                      Serial.print(addrA.bytes[5-i], HEX);
                }
                res = 1;
           } else if(Ainbuf[0] == 'M') {
                dbg(mybd);
                my_addr = BT.GetAddr();
                for(i=0;i<6;i++){
                      Serial.print(":");
                      Serial.print(my_addr.bytes[5-i], HEX);
                }
                Serial.print("\r\n");
                res = 1;
            }
        }
    } else {
        BT.Send(Ainbuf, Aincount);
        res = 1;
    }
    if(Aincount == 5){
        if(strcmp_(Ainbuf, (int8_t*)"uname", 5)) {
            dbgn(uname);
            res = 1;
        }
        if(strcmp_(Ainbuf, (int8_t*)"reset", 5)) {
            soft_reset();
            res = 1;
        }
    }      
    return res;
}
///////////////////////////////////////////////////////////////////////////////////////
void SerialHandler(void){
    int8_t linbyte;
    linbyte = Serial.read();
    if((linbyte > 0) && (linbyte < 128)) {
        inbuf[incount] = linbyte;
        incount++;
        if(incount >= INBUF_SIZE) incount = 0;
        if(linbyte == EOL_SYM) {
            if(InbufAnalyse(incount - 1, inbuf) == 0) InbufAnalyse(incount - 2, inbuf); 
            incount = 0;
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////
void dbg(const int8_t * str){
    int8_t c;
    while((c = pgm_read_byte(str++))) {
// Arduino 1.0 compatibility
#if defined(ARDUINO) && ARDUINO >= 100
        Serial.write(c);
#else
        Serial.print(c, BYTE);
#endif
    }
}
///////////////////////////////////////////////////////////////////////////////////////
void dbgn(const int8_t * str){
    dbg(str);
    Serial.print("\r\n");
}
///////////////////////////////////////////////////////////////////////////////////////
int8_t strcmp_(int8_t Astr1[], int8_t Astr2[], uint16_t Acnt){
    uint16_t i;
    for(i = 0; i < Acnt; i++){
        if(Astr1[i] != Astr2[i]) return 0;
    }  
    return 1;
}
///////////////////////////////////////////////////////////////////////////////////////
void welcome_banner(void){
    dbgn(uname);
    dbgn(welcome);
}

