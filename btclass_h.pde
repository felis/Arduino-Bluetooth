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
#include "serial.h"
#include "bt_h.h"
#include "Max3421e.h"
#include "Usb.h"

TBT BT;

uint8_t state;

extern const int8_t uname[], welcome[]; 

void setup(void){
    Serial.begin(115200);
    welcome_banner();
    state = 0;
}

void loop(void){
    uint8_t len, i;
    int8_t str[48];
    if(state == 0){
        SerialHandler();
    } else {
        if((state == 1) && (BT.GetState() == BT_STATE_CONNECTED)){
            Serial.println("Connected");
            state = 2;
        }

        SerialHandler();
        len = BT.Task(str);
        if(len > 0){
            for(i = 0; i < len; i++) {
// Arduino 1.0 compatibility
#if defined(ARDUINO) && ARDUINO >= 100
                Serial.write(str[i]);
#else
                Serial.print(str[i], BYTE);
#endif
            }
            Serial.print("\r\n");
        }  
    }
}
