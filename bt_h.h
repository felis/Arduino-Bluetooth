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
#ifndef _BT_H_H_
#define _BT_H_H_


#include "Spi.h"
#include "Max3421e.h"
#include "Usb.h"

/* CSR Bluetooth data taken from descriptors */
#define BT_ADDR                           1
#define BT_CONFIGURATION                  1
#define BT_INTERFACE                      0 // Only use interface 0
#define INT_MAXPKTSIZE                    16
#define BULK_MAXPKTSIZE                   64

#define EP_INTERRUPT                      0x03 // endpoint types
#define EP_BULK                           0x02
#define EP_POLL                           0x01 // interrupt poll interval

#define CONTROL_PIPE                      0 // names we give to the 4 pipes
#define EVENT_PIPE                        1
#define DATAIN_PIPE                       2
#define DATAOUT_PIPE                      3

#define MAX_BUFFER_SIZE                   64 // size of general purpose data buffer
#define bmREQ_HCI_OUT                     USB_SETUP_HOST_TO_DEVICE|USB_SETUP_TYPE_CLASS|USB_SETUP_RECIPIENT_DEVICE
#define HCI_COMMAND_REQ                   0 

#define BT_STATE_INIT                     0
#define BT_STATE_MAX                      1
#define BT_STATE_USB                      2
#define BT_STATE_HCI                      3
#define BT_STATE_CONNECTED                4

#define HCI_STATE_INIT                    0x00
#define HCI_STATE_RESET_WAIT              0x01
#define HCI_STATE_READ_BD_ADDR_WAIT       0x02
#define HCI_STATE_SET_EVENT_FILTER_WAIT   0x03
#define HCI_STATE_WRITE_COD_WAIT          0x04
#define HCI_STATE_SET_LOCAL_NAME_WAIT     0x05
#define HCI_STATE_SET_PAGE_TIMEOUT_WAIT   0x06
#define HCI_STATE_INQUIRY_RESULT_WAIT     0x07
#define HCI_STATE_SCAN_ENABLE_WAIT        0x08
#define HCI_STATE_CONNECT_WAIT            0x09
#define HCI_STATE_CONNECTED               0x0a
#define HCI_STATE_PIN_REQUEST_WAIT        0x0b

#define BT_SERVER                         0
#define BT_CLIENT                         1
#define BT_CLIENT_ADDR                    2


typedef struct{
  uint8_t bytes[6];  
} Tbd_addr;


class TBT{
    public:
        TBT();
        void InitServer(void);
        void InitClient(void);
        void InitClientAddr(Tbd_addr Aaddr);
        uint8_t Task(int8_t * Astr);
        uint8_t GetState(void);
        Tbd_addr GetAddr(void);
        void Send(int8_t * Astr, uint8_t Acnt);
        void SetName(int8_t * Astr, uint8_t Acnt);
        void SetPin(int8_t * Astr, uint8_t Acnt);
        void SetEP(uint8_t Aep_addr1, uint8_t Aep_addr2, uint8_t Aep_addr3);
    private:  
        void Init(void);
        void Spp_Init(void);
        void HCI_Poll(void);
        uint8_t ACL_Poll(void);
        uint8_t HCI_Send(uint8_t Abytes_count, int8_t * Abytes);
        uint8_t ACL_Send(uint8_t Abytes_count, int8_t * Abytes);
        void Convert(uint8_t Abytes_count, const int8_t * Abytes1, int8_t * Abytes2);
        
        void HCI_Task(void);

        void HCI_Reset(void);
        void HCI_Read_Buf_Size(void);

        int8_t strcmp_p(int8_t Astr1[], const int8_t Astr2[], uint16_t Acnt);

        MAX3421E Max;
        USB Usb;
        uint8_t UsbState;
        EP_RECORD ep_record[4];                    //endpoint record structure for the Bluetooth controller
        int8_t buf[MAX_BUFFER_SIZE];                 //General purpose buffer for usb data
        uint8_t buf_count;
        uint8_t ep_addr[3];
        uint8_t cState, cState_p;
        uint8_t cBehaviour;
        
        uint8_t hci_state;
        
        Tbd_addr my_addr, dest_addr, user_addr;
        int8_t my_name[16], my_pin[8];
        uint8_t my_name_len, my_pin_len;
        uint8_t conn_handle[2];
};


#endif
