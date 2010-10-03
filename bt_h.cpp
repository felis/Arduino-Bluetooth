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

const int8_t hci_reset_cmd[] PROGMEM =              {0x03, 0x0c, 0x00};
const int8_t hci_read_bd_addr_cmd[] PROGMEM =       {0x09, 0x10, 0x00};
const int8_t hci_set_event_filter_cmd[] PROGMEM =   {0x05, 0x0c, 0x03, 0x02, 0x00, 0x03};
const int8_t hci_write_cod_cmd[] PROGMEM =          {0x24, 0x0c, 0x03, 0x04, 0x02, 0x58};//0x08, 0x04, 0x24};
const int8_t hci_set_local_name_cmd[] PROGMEM =     {0x13, 0x0c};
const int8_t hci_set_page_timeout_cmd[] PROGMEM =   {0x18, 0x0c, 0x02, 0x00, 0x40};

const int8_t hci_inquiry_cmd[] PROGMEM =            {0x01, 0x04, 0x05, 0x33, 0x8b, 0x9e, 0x04, 0x01};
const int8_t hci_connect_cmd[] PROGMEM =            {0x05, 0x04, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xcc, 0xcc, 0x01, 0x00, 0x8d, 0x2a, 0x01};
const int8_t hci_scan_enable_cmd[] PROGMEM =        {0x1a, 0x0c, 0x01, 0x03};
const int8_t hci_pin_reply_cmd[] PROGMEM =          {0x0d, 0x04, 0x17};

const int8_t hci_reset_resp[] PROGMEM =             {0x0e, 0x04, 0x01, 0x03, 0x0c, 0x00};
const int8_t hci_read_bd_addr_resp[] PROGMEM =      {0x0e, 0x0a, 0x01, 0x09, 0x10, 0x00};
const int8_t hci_set_event_filter_resp[] PROGMEM =  {0x0e, 0x04, 0x01, 0x05, 0x0c, 0x00};
const int8_t hci_write_cod_resp[] PROGMEM =         {0x0e, 0x04, 0x01, 0x24, 0x0c, 0x00};
const int8_t hci_set_local_name_resp[] PROGMEM =    {0x0e, 0x04, 0x01, 0x13, 0x0c, 0x00};
const int8_t hci_set_page_timeout_resp[] PROGMEM =  {0x0e, 0x04, 0x01, 0x18, 0x0c, 0x00};

const int8_t hci_inquiry_resp[] PROGMEM =           {0x01, 0x01, 0x00};
const int8_t hci_inquiry_result_resp[] PROGMEM =    {0x02, 0x0f, 0x01};
const int8_t hci_connection_result_resp[] PROGMEM = {0x03, 0x0b, 0x00};
const int8_t hci_scan_enable_resp[] PROGMEM =       {0x0e, 0x04, 0x01, 0x1a, 0x0c, 0x00};
const int8_t hci_pin_request_event[] PROGMEM =      {0x16, 0x06};
const int8_t hci_role_change_event[] PROGMEM =      {0x12, 0x08, 0x00};

/////////////////////////////////////////////////////////////////////////////////////
TBT::TBT(){
    uint8_t i;
    const int8_t lname[]="Arduino", lpin[]="1234";

    UsbState = USB_STATE_DETACHED;  
    cState = BT_STATE_INIT;
    ep_addr[0] = 0x01;
    ep_addr[1] = 0x02;
    ep_addr[2] = 0x02;

    my_name_len = 7;
    my_pin_len = 4;
    for(i = 0; i < my_name_len; i++)
        my_name[i] = lname[i];
    for(i = 0; i < my_pin_len; i++)
        my_pin[i] = lpin[i];

};
/////////////////////////////////////////////////////////////////////////////////////
void TBT::InitServer(void){
    cBehaviour = BT_SERVER;
    Init();
};
/////////////////////////////////////////////////////////////////////////////////////
void TBT::InitClient(void){
    cBehaviour = BT_CLIENT;
    Init();
};
/////////////////////////////////////////////////////////////////////////////////////
void TBT::InitClientAddr(Tbd_addr Aaddr){
    user_addr = Aaddr;
    cBehaviour = BT_CLIENT_ADDR;
    Init();
};
/////////////////////////////////////////////////////////////////////////////////////
void TBT::Init(void){
    hci_state = HCI_STATE_INIT;
    buf_count = 0;
    cState_p = cState;
    cState = BT_STATE_MAX;
    Max.powerOn();
    delay(200);
};
/////////////////////////////////////////////////////////////////////////////////////
uint8_t TBT::Task(int8_t * Abuf){
    uint8_t res, i;
    res = 0;
    UsbState = Usb.getUsbTaskState();
    Max.Task();
    Usb.Task();
    delay(1);
    if(UsbState == USB_STATE_CONFIGURING){
        Spp_Init();
        Usb.setUsbTaskState(USB_STATE_RUNNING);
    }
  
    if(UsbState == USB_STATE_RUNNING){
        if(cState == BT_STATE_USB) {
            cState_p = cState;
            cState = BT_STATE_HCI;
        }
        res = ACL_Poll();
        if(res != 0){
            if(cState != BT_STATE_CONNECTED) res = 0;
            else {
                for(i = 4; i < res; i++)
                    Abuf[i-4] = buf[i];  
                res -= 4;
            }
        }
        HCI_Poll();
        HCI_Task();
    }  
    return res;
};

/////////////////////////////////////////////////////////////////////////////////////
void TBT::Spp_Init(void){
    uint8_t rcode = 0;  //return code
    cState_p = cState;
    cState = BT_STATE_USB;

    // Initialize data structures for endpoints of device
    ep_record[ CONTROL_PIPE ] = *( Usb.getDevTableEntry( 0,0 ));  //copy endpoint 0 parameters
    ep_record[ EVENT_PIPE ].epAddr = ep_addr[0];    // Bluetooth event endpoint
    ep_record[ EVENT_PIPE ].Attr  = EP_INTERRUPT;
    ep_record[ EVENT_PIPE ].MaxPktSize = INT_MAXPKTSIZE;
    ep_record[ EVENT_PIPE ].Interval  = EP_POLL;
    ep_record[ EVENT_PIPE ].sndToggle = bmSNDTOG0;
    ep_record[ EVENT_PIPE ].rcvToggle = bmRCVTOG0;
    ep_record[ DATAIN_PIPE ].epAddr = ep_addr[1];    // Bluetooth data endpoint
    ep_record[ DATAIN_PIPE ].Attr  = EP_BULK;
    ep_record[ DATAIN_PIPE ].MaxPktSize = BULK_MAXPKTSIZE;
    ep_record[ DATAIN_PIPE ].Interval  = 0;
    ep_record[ DATAIN_PIPE ].sndToggle = bmSNDTOG0;
    ep_record[ DATAIN_PIPE ].rcvToggle = bmRCVTOG0;
    ep_record[ DATAOUT_PIPE ].epAddr = ep_addr[2];    // Bluetooth data endpoint
    ep_record[ DATAOUT_PIPE ].Attr  = EP_BULK;
    ep_record[ DATAOUT_PIPE ].MaxPktSize = BULK_MAXPKTSIZE;
    ep_record[ DATAOUT_PIPE ].Interval  = 0;
    ep_record[ DATAOUT_PIPE ].sndToggle = bmSNDTOG0;
    ep_record[ DATAOUT_PIPE ].rcvToggle = bmRCVTOG0;
    Usb.setDevTableEntry(BT_ADDR, ep_record); 
       
    //Configure the device 
    rcode = Usb.setConf(BT_ADDR, ep_record[ CONTROL_PIPE ].epAddr, BT_CONFIGURATION);                    
    if( rcode ) {
        //dbgn("Err Set Conf");
        while(1);  //stop
    }
    delay(200);
    //dbgn("usb init\n");  
};
/////////////////////////////////////////////////////////////////////////////////////
void TBT::HCI_Poll(void){
    uint8_t rcode, i;
    rcode = Usb.inTransfer(BT_ADDR, ep_record[EVENT_PIPE].epAddr, MAX_BUFFER_SIZE, (char*)buf, USB_NAK_NOWAIT); 
    if(!rcode){
        buf_count = buf[1] + 2;

        /*Serial.print("HCI_evt:");
        Serial.print(buf_count, HEX);
        Serial.print(":");
        for(i = 0; i < buf_count; i++){
          Serial.print((buf)[i], HEX);
          Serial.print(" ");
        }
        Serial.print("\r\n");*/
    }    
};
/////////////////////////////////////////////////////////////////////////////////////
uint8_t TBT::ACL_Poll(void){
    uint8_t rcode, i;
    rcode = Usb.inTransfer(BT_ADDR, ep_record[DATAIN_PIPE].epAddr, MAX_BUFFER_SIZE, (char*)buf, USB_NAK_NOWAIT); 
    if(!rcode){
        buf_count = buf[2] + 4;

        /*Serial.print("ACL_evt:");
        Serial.print(buf_count, HEX);
        Serial.print(":");
        for(i = 0; i < buf_count; i++){
          Serial.print((buf)[i], HEX);
          Serial.print(" ");
        }
        Serial.print("\r\n");*/
        return buf_count;
    }  
    return 0;
};
/////////////////////////////////////////////////////////////////////////////////////
uint8_t TBT::HCI_Send(uint8_t Abytes_count, int8_t * Abytes) {
    uint8_t i;
    buf_count = 0;
      
    /*Serial.print("HCI_com:");
    Serial.print(Abytes_count, HEX);
    Serial.print(":");
    for(i=0;i<Abytes_count;i++){
        Serial.print(Abytes[i], HEX);
        Serial.print(" ");
    }
    Serial.print("\r\n");*/


    return(Usb.ctrlReq(BT_ADDR, ep_record[CONTROL_PIPE].epAddr, bmREQ_HCI_OUT, HCI_COMMAND_REQ, 0x00, 0x00 ,0, Abytes_count, (char*)Abytes)); 
};
/////////////////////////////////////////////////////////////////////////////////////
uint8_t TBT::ACL_Send(uint8_t Abytes_count, int8_t * Abytes) {
    uint8_t i;
    buf_count = 0;
      
    /*Serial.print("ACL_com:");
    Serial.print(Abytes_count, HEX);
    Serial.print(":");
    for(i=0;i<Abytes_count;i++){
        Serial.print(Abytes[i], HEX);
        Serial.print(" ");
    }
    Serial.print("\r\n");*/

    return(Usb.outTransfer(BT_ADDR, ep_record[DATAOUT_PIPE].epAddr, Abytes_count, (char*)Abytes));
};
/////////////////////////////////////////////////////////////////////////////////////
uint8_t TBT::GetState(void){
    return cState;
};
/////////////////////////////////////////////////////////////////////////////////////
void TBT::Convert(uint8_t Abytes_count, const int8_t * Abytes1, int8_t * Abytes2){
    uint8_t i;
    for(i = 0; i < Abytes_count; i++){
        Abytes2[i] = pgm_read_byte(Abytes1++);  
    }
          
};
/////////////////////////////////////////////////////////////////////////////////////
void TBT::HCI_Task(void){
    uint8_t i;
    if(cState >= BT_STATE_HCI){
        if(hci_state == HCI_STATE_INIT) {
            hci_state = HCI_STATE_RESET_WAIT;
            Convert(3, hci_reset_cmd, buf);
            HCI_Send(3, buf);
        }
        if(hci_state == HCI_STATE_RESET_WAIT) {
            if((buf_count == 6) && strcmp_p(buf, hci_reset_resp, 6)){
                hci_state = HCI_STATE_READ_BD_ADDR_WAIT;
                Convert(3, hci_read_bd_addr_cmd, buf);
                HCI_Send(3, buf);
            }
        }//else dbgn("could not reset device");

        if(hci_state == HCI_STATE_READ_BD_ADDR_WAIT) {
            if((buf_count == 12) && strcmp_p(buf, hci_read_bd_addr_resp, 6)){
                for(i = 0; i < 6; i++)
                    my_addr.bytes[i] = buf[i + 6];
              
                hci_state = HCI_STATE_SET_EVENT_FILTER_WAIT;
                Convert(6, hci_set_event_filter_cmd, buf);
                HCI_Send(6, buf);
            }
        }//else dbgn("could not read own bd address");
        
        if(hci_state == HCI_STATE_SET_EVENT_FILTER_WAIT) {
            if((buf_count == 6) && strcmp_p(buf, hci_set_event_filter_resp, 6)){
                hci_state = HCI_STATE_WRITE_COD_WAIT;
                Convert(6, hci_write_cod_cmd, buf);
                HCI_Send(6, buf);
            }
        }//else dbgn("could not set event filter");

        if(hci_state == HCI_STATE_WRITE_COD_WAIT){
            if((buf_count == 6) && strcmp_p(buf, hci_write_cod_resp, 6)){
                hci_state = HCI_STATE_SET_LOCAL_NAME_WAIT;
                Convert(2, hci_set_local_name_cmd, buf);
                buf[2] = my_name_len+1;
                for(i = 0; i < my_name_len; i++)
                    buf[i + 3] = my_name[i];
                buf[i + 3] = 0;
                HCI_Send(4 + my_name_len, buf);
            }
        }//else dbgn("could not write code of device");

        if(hci_state == HCI_STATE_SET_LOCAL_NAME_WAIT) {
            if((buf_count == 6) && strcmp_p(buf, hci_set_local_name_resp, 6)){
                hci_state = HCI_STATE_SET_PAGE_TIMEOUT_WAIT;
                Convert(5, hci_set_page_timeout_cmd, buf);
                HCI_Send(5, buf);
            }
        }//else dbgn("could not set own local name");

        if(hci_state == HCI_STATE_SET_PAGE_TIMEOUT_WAIT) {
            if((buf_count == 6) && strcmp_p(buf, hci_set_page_timeout_resp, 6)){
                if(cBehaviour == BT_CLIENT){// scan for devices
                    hci_state = HCI_STATE_INQUIRY_RESULT_WAIT;
                    Convert(8, hci_inquiry_cmd, buf);
                    HCI_Send(8, buf);
                } else if(cBehaviour == BT_SERVER){// be visible for incoming connection
                    hci_state = HCI_STATE_SCAN_ENABLE_WAIT;
                    Convert(4, hci_scan_enable_cmd, buf);
                    HCI_Send(4, buf);
                } else {// connect to predefined address
                    Convert(16, hci_connect_cmd, buf);
                    for(i=0;i<6;i++)
                        buf[i+3] = user_addr.bytes[i];
                    for(i=0;i<2;i++)
                        buf[i+13] = buf[i+15];
                    hci_state = HCI_STATE_CONNECT_WAIT;
                    HCI_Send(16, buf);
                }
            }
        }//else dbgn("could not set page timeout");
        
        if(hci_state == HCI_STATE_INQUIRY_RESULT_WAIT) {
            if((buf_count == 3) && strcmp_p(buf, hci_inquiry_resp, 3)){
                hci_state = HCI_STATE_INQUIRY_RESULT_WAIT;
                Convert(8, hci_inquiry_cmd, buf);
                HCI_Send(8, buf);
            }
            if((buf_count == 17) && strcmp_p(buf, hci_inquiry_result_resp, 3)){
                for(i=0;i<6;i++)
                    dest_addr.bytes[i] = buf[i+3];
                Convert(16, hci_connect_cmd, buf);
                for(i=0;i<6;i++)
                    buf[i+3] = dest_addr.bytes[i];
                for(i=0;i<2;i++)
                    buf[i+13] = buf[i+15];
                hci_state = HCI_STATE_CONNECT_WAIT;
                HCI_Send(16, buf);
            }
        }
        if((hci_state == HCI_STATE_CONNECT_WAIT) || (hci_state == HCI_STATE_PIN_REQUEST_WAIT) || (hci_state == HCI_STATE_CONNECTED)){
            if((buf_count == 13) && (strcmp_p(buf, hci_connection_result_resp, 3)) && (buf[12] == 0x00)){
                hci_state = HCI_STATE_CONNECTED;
                buf_count = 0;
                conn_handle[0] = buf[3];
                conn_handle[1] = buf[4] | 0x20;
                cState_p = cState;
                cState = BT_STATE_CONNECTED;
            }

            if((buf_count == 8) && strcmp_p(buf, hci_pin_request_event, 2)){
                for(i=0;i<6;i++)
                    dest_addr.bytes[i] = buf[i+2];
  
                if(hci_state != HCI_STATE_CONNECTED) hci_state = HCI_STATE_CONNECT_WAIT;

                Convert(3, hci_pin_reply_cmd, buf);
                for(i=0;i<6;i++)
                    buf[i+3] = dest_addr.bytes[i];
                buf[9] = my_pin_len;
                for(i=0;i<16;i++)
                    buf[i+10] = 0;
                for(i=0;i<my_pin_len;i++)
                    buf[i+10] = my_pin[i];
                HCI_Send(26, buf);
            }
        }
        
        if(hci_state == HCI_STATE_SCAN_ENABLE_WAIT) {
            if((buf_count == 6) && strcmp_p(buf, hci_scan_enable_resp, 6)){
                hci_state = HCI_STATE_PIN_REQUEST_WAIT;
            }
        }
    }
};


////////////////////////////////////////////////////////////////////////////////////////////////////////
void TBT::Send(int8_t * Astr, uint8_t Acnt){
    uint8_t i;
    buf[0] = conn_handle[0];
    buf[1] = conn_handle[1];
    buf[2] = Acnt;
    buf[3] = 0;
    for(i = 0; i < Acnt; i++)
        buf[i+4] = Astr[i];
    ACL_Send(Acnt+4, buf);
};
////////////////////////////////////////////////////////////////////////////////////////////////////////
void TBT::SetName(int8_t * Astr, uint8_t Acnt){
    uint8_t i;
    my_name_len = Acnt;
    if(my_name_len > 16) my_name_len = 16;
    for(i=0;i<my_name_len;i++)
        my_name[i] = Astr[i];
};
////////////////////////////////////////////////////////////////////////////////////////////////////////
void TBT::SetPin(int8_t * Astr, uint8_t Acnt){
    uint8_t i;
    my_pin_len = Acnt;
    if(my_pin_len > 8) my_pin_len = 8;
    for(i=0;i<my_pin_len;i++)
        my_pin[i] = Astr[i];
};
///////////////////////////////////////////////////////////////////////////////////////
void TBT::SetEP(uint8_t Aep_addr1, uint8_t Aep_addr2, uint8_t Aep_addr3){
    ep_addr[0] = Aep_addr1;
    ep_addr[1] = Aep_addr2;
    ep_addr[2] = Aep_addr3; 
};
///////////////////////////////////////////////////////////////////////////////////////
Tbd_addr TBT::GetAddr(void){
    return my_addr;  
};
///////////////////////////////////////////////////////////////////////////////////////
int8_t TBT::strcmp_p(int8_t Astr1[], const int8_t Astr2[], uint16_t Acnt){
    uint16_t i;
    for(i = 0; i < Acnt; i++){
        if(Astr1[i] != pgm_read_byte(Astr2++)) return 0;
    }  
    return 1;
};
