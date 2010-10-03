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
#ifndef _SERIAL_H_
#define _SERIAL_H_

//#ifdef __cplusplus
//extern "C" {
//#endif

#define INBUF_SIZE     50
#define EOL_SYM        0x0d

#include <stdint.h>

uint8_t InbufAnalyse(uint16_t Aincount, int8_t *Ainbuf);
void SerialHandler(void);

void dbg(const int8_t * str);
void dbgn(const int8_t * str);

void welcome_banner(void);

int8_t strcmp_(int8_t Astr1[], int8_t Astr2[], uint16_t Acnt);

//#ifdef __cplusplus
//}
//#endif

#endif
