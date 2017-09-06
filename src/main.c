#include "stm32f10x_conf.h"
#include "SEGGER_RTT.h"
#include <stdio.h>

#include "usbdefs.h"
#include "usbregs.h"

#define DBG     SEGGER_RTT_printf

// Globals:
volatile EP_BUF_DSCR *pBUF_DSCR = (EP_BUF_DSCR *)PMAAddr;
volatile EP_BUF_DSCR *EP1_BufDscr = (EP_BUF_DSCR *) 0x40006010;
uint32_t FreeBufAddr;
uint8_t EP0Buffer[64];
USB_SETUP_PACKET sp;

const USB_DEVICE_DESCRIPTOR dd;
const USB_INTERFACE_DESCRIPTOR id;
const USB_HID_DESCRIPTOR hd;
const USB_QUALIFIER_DESCRIPTOR qd;
const USB_ENDPOINT_DESCRIPTOR ed_in;
const USB_ENDPOINT_DESCRIPTOR ed_out;
const USB_CONFIGURATION_DESCRIPTOR cd;
const uint8_t HID_REPORT_DESCRIPTOR[47];
const USB_STRING_DESCRIPTOR *LangID_StringDescriptor;
const USB_STRING_DESCRIPTOR *Manufacturer_StringDescriptor;
const USB_STRING_DESCRIPTOR *Product_StringDescriptor;
const USB_STRING_DESCRIPTOR *Serial_StringDescriptor;

void USB_Reset();
void USB_CTR(uint16_t epid);
void USB_Error();
void USB_Suspend();
void USB_Wakeup();
void USB_PMAOverrun();
void USB_SetupDevice();
void USB_SetupInterface();
void USB_SetupEndpoint();

void USB_Init()
{
    //Set up USB clock configuration, enable USB HW clock
	RCC->CFGR &= ~RCC_CFGR_USBPRE;
	RCC->APB1ENR |= RCC_APB1ENR_USBEN;

    //Force reset of USB peripheral, clear any outstanding interrupts
	_SetCNTR(CNTR_FRES);
	_SetCNTR(0);
	_SetISTR(0);

    //Set up USB interrupts
    NVIC_ClearPendingIRQ(USB_HP_CAN1_TX_IRQn);
	NVIC_ClearPendingIRQ(USB_LP_CAN1_RX0_IRQn);
	NVIC_EnableIRQ(USB_HP_CAN1_TX_IRQn);
	NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);

	//Enable USB interrupt generation for CTR, Wakeup, Suspend, Reset, Error, and PMA Error events
	_SetCNTR(USB_CNTR_CTRM | USB_CNTR_WKUPM | USB_CNTR_SUSPM | USB_CNTR_RESETM | USB_CNTR_ERRM | USB_CNTR_PMAOVRM);
}



uint32_t USB_ReadEP (uint8_t EPNum, uint8_t *pData)
{
  uint32_t num, cnt, *pv, n;

  num = EPNum & 0x0F;

  pv  = (uint32_t *)(PMAAddr + 2*((pBUF_DSCR + num)->ADDR_RX));
  cnt = (pBUF_DSCR + num)->COUNT_RX & EP_COUNT_MASK;
  for (n = 0; n < (cnt + 1) / 2; n++) {
    *((__packed uint16_t *)pData) = *pv++;
    pData += 2;
  }

  _SetEPRxValid(num);

  return (cnt);
}



uint32_t USB_WriteEP (uint8_t EPNum, const uint8_t *pData, uint32_t cnt)
{
  uint32_t num, *pv, n;

  num = EPNum & 0x0F;

  pv  = (uint32_t *)(PMAAddr + 2*((pBUF_DSCR + num)->ADDR_TX));
  for (n = 0; n < (cnt + 1) / 2; n++) {
    *pv++ = *((__packed uint16_t *)pData);
    pData += 2;
  }
  (pBUF_DSCR + num)->COUNT_TX = cnt;

  _SetEPTxValid(num);

  return (cnt);
}



inline void USB_Reset()
{
    DBG(0, "\nUSB Reset");

    FreeBufAddr = EP_BUF_ADDR;

    _SetISTR(0);
    _SetCNTR(USB_CNTR_CTRM | USB_CNTR_WKUPM | USB_CNTR_SUSPM | USB_CNTR_RESETM | USB_CNTR_ERRM | USB_CNTR_PMAOVRM);
    _SetBTABLE(0);

    //Global EP_BUF_DSCR structures map to hardware buffer descriptor table
    //Set up EP0
    pBUF_DSCR->ADDR_TX = FreeBufAddr;           // 0x20
    FreeBufAddr += USB_MAX_PACKET0;
    pBUF_DSCR->ADDR_RX = FreeBufAddr;           // 0x60
    FreeBufAddr += USB_MAX_PACKET0;
    if (USB_MAX_PACKET0 > 62) {
        pBUF_DSCR->COUNT_RX = ((USB_MAX_PACKET0 << 5) - 1) | 0x8000;
    } else {
        pBUF_DSCR->COUNT_RX =   USB_MAX_PACKET0 << 9;
    }
    pBUF_DSCR->COUNT_TX = 0;

    _SetEPType(0, EP_CONTROL);                      // Set EP0 as control endpoint
    _SetEPRxStatus(0, EP_RX_VALID);                 // Enable reception of data at EP0


    //Setup EP 1
    _SetEPAddress(1, 1);
    EP1_BufDscr->ADDR_TX = FreeBufAddr;         // 0xA0
    //_SetEPTxAddr(1, FreeBufAddr);
    FreeBufAddr += 64;
    EP1_BufDscr->ADDR_RX = FreeBufAddr;         // 0xE0
    //_SetEPRxAddr(1, FreeBufAddr);
    FreeBufAddr += 64;
    EP1_BufDscr->COUNT_RX = ((64 << 5) - 1) | 0x8000;
    EP1_BufDscr->COUNT_TX = 0;
    //_SetEPRxCount(1, (((64 << 5) -1) | 0x8000) );

    _SetEPType(1, EP_INTERRUPT);
    _SetEPRxStatus(1, EP_RX_VALID);
    //_ToggleDTOG_RX(1);


    _SetDADDR(DADDR_EF | 0);                        // Enable USB device, set address to 0 while enumerating


    //This is a bullshit test. Remove it......................
    uint16_t dummy = 0;
    USB_WriteEP(1, (uint8_t *)&dummy, 2);
    //End remove..............................................
}

void printEP0Buffer(uint32_t num)
{
    uint32_t i;
    for(i = 0; i < num; i++)
    {
        DBG(0, "%02x ", EP0Buffer[i]);
    }
}

// Handles setup packets addressed to the device
inline void USB_SetupDevice()
{
    uint32_t numbytes;
    uint8_t address;
    uint8_t deviceStatus[2] = {0, 0};

    USB_CONFIG_DESCRIPTOR_FULL cdf;
    cdf.cd = cd;
    cdf.id = id;
    cdf.hd = hd;
    cdf.ed_in = ed_in;
    cdf.ed_out = ed_out;

    switch(sp.bRequest)
    {
    case USB_REQUEST_CLEAR_FEATURE:
        USB_WriteEP(0, NULL, 0);
        DBG(0, "\nDevice clear feature requested. Not implemented. ZLP sent.");
        break;
    case USB_REQUEST_SET_FEATURE:
        USB_WriteEP(0, NULL, 0);
        DBG(0, "\nDevice set feature requested. Not implemented. ZLP sent.");
        break;
    case USB_REQUEST_SET_ADDRESS:
        USB_WriteEP(0, NULL, 0);
        DBG(0, "\nDevice set address requested. ");
        address = sp.wValue.bytes.low;
        _SetDADDR((address & DADDR_ADD) | (1<<7));
        DBG(0, "Device address set to %d", address);
        break;
    case USB_REQUEST_SET_DESCRIPTOR:
        USB_WriteEP(0, NULL, 0);
        DBG(0, "\nDevice set descriptor requested. Not implemented. ZLP sent.");
        break;
    case USB_REQUEST_SET_CONFIGURATION:
        USB_WriteEP(0, NULL, 0);
        DBG(0, "\nDevice set configuration requested. Not implemented. ZLP sent.");
        break;

    case USB_REQUEST_GET_STATUS:
        USB_WriteEP(0, deviceStatus, 2);
        DBG(0, "\nDevice get status requested.");
        break;
    case USB_REQUEST_GET_DESCRIPTOR:
        switch(sp.wValue.bytes.high)
        {
        case USB_DEVICE_DESCRIPTOR_TYPE:
            numbytes = USB_WriteEP(0, (uint8_t *)&dd, sizeof(dd));
            DBG(0, "\nSent %d byte device descriptor to tx buffer.", numbytes);
            break;
        case USB_CONFIGURATION_DESCRIPTOR_TYPE:
            numbytes = USB_WriteEP(0, (uint8_t *)&cdf, sp.wLength > sizeof(cdf) ? sizeof(cdf) : sp.wLength);
            DBG(0, "\nSent %d byte configuration descriptor to tx buffer.", numbytes);
            break;
        case USB_STRING_DESCRIPTOR_TYPE:
            switch(sp.wValue.bytes.low)
            {
            case 0:     // Language ID string descriptor requested.
                USB_WriteEP(0, (uint8_t *)LangID_StringDescriptor, LangID_StringDescriptor->bLength);
                DBG(0, "\nLang ID string descriptor requested.");
                break;
            case 1:     // Vendor string descriptor requested.
                USB_WriteEP(0, (uint8_t *)Manufacturer_StringDescriptor, Manufacturer_StringDescriptor->bLength);
                DBG(0, "\nVendor ID string descriptor requested.");
                break;
            case 2:
                USB_WriteEP(0, (uint8_t *)Product_StringDescriptor, Product_StringDescriptor->bLength);
                DBG(0, "\nProduct ID string descriptor requested.");
                break;
            case 3:
                USB_WriteEP(0, (uint8_t *)Serial_StringDescriptor, Serial_StringDescriptor->bLength);
                DBG(0, "\nSerial string descriptor requested.");
                break;
            default:
                DBG(0, "\nInvalid string descriptor requested.");
                break;
            }
            break;
        case USB_INTERFACE_DESCRIPTOR_TYPE:
            numbytes = USB_WriteEP(0, (uint8_t *)&cd, sizeof(id));
            DBG(0, "\nSent %d byte interface descriptor to tx buffer.", numbytes);
            break;
        case USB_ENDPOINT_DESCRIPTOR_TYPE:
            DBG(0, "\nEndpoint descriptor requested. Not implemented.");
            break;
        case USB_DEVICE_QUALIFIER_DESCRIPTOR_TYPE:
            numbytes = USB_WriteEP(0, (uint8_t *)&qd, sizeof(qd));
            DBG(0, "\nWrote %d bytes qualifier descriptor.", numbytes);
            break;
        case USB_OTHER_SPEED_CONFIG_DESCRIPTOR_TYPE:
            DBG(0, "\nOther speed config descriptor requested. Not implemented.");
            break;
        case USB_INTERFACE_POWER_DESCRIPTOR_TYPE:
            DBG(0, "\nUSB interface power descriptor requested. Not implemented.");
            break;
        default:
            break;
        }
        break;
    case USB_REQUEST_GET_CONFIGURATION:
        DBG(0, "\nDevice get configuration requested. Not implemented.");
        break;

    default:
        break;
     }
}

// Handles setup packets addressed to the interface
inline void USB_SetupInterface()
{
    uint8_t interfaceStatus[2] = {0,0};
    switch(sp.bRequest)
    {
    case USB_REQUEST_GET_DESCRIPTOR:
        if(sp.wValue.bytes.high == USB_HID_REPORT_DESCRIPTOR_TYPE)
        {
            USB_WriteEP(0, HID_REPORT_DESCRIPTOR, 47);
            //USB_WriteEP(0, NULL, 64);
            DBG(0, "\nUSB HID report descriptor requested.");
        }

        break;
    case USB_REQUEST_GET_STATUS:
        USB_WriteEP(0, interfaceStatus, 2);
        DBG(0, "\nInterface get status requested.");
        break;
    case USB_REQUEST_CLEAR_FEATURE:
        USB_WriteEP(0, NULL, 0);
        DBG(0, "\nInterface clear feature requested. Not implemented. ZLP sent.");
        break;
    case USB_REQUEST_SET_FEATURE:
        USB_WriteEP(0, NULL, 0);
        DBG(0, "\nInterface set feature requested. Not implemented. ZLP sent.");
        break;
    case USB_REQUEST_GET_INTERFACE:
        USB_WriteEP(0, NULL, 0);
        DBG(0, "\nInterface get interface requested. Not implemented. ZLP sent.");
        break;
    case USB_REQUEST_SET_INTERFACE:
        USB_WriteEP(0, NULL, 0);
        DBG(0, "\nInterface set interface requested. Not implemented. ZLP sent.");
        break;
    }

}

// Handles setup packets addressed to the endpoint
inline void USB_SetupEndpoint()
{
    uint8_t endpointStatus[2] = {0,0};
    switch(sp.bRequest)
    {
    case USB_REQUEST_GET_STATUS:
        USB_WriteEP(0, endpointStatus, 2);
        DBG(0, "\nEndpoint get status requested");
        break;
    case USB_REQUEST_CLEAR_FEATURE:
        DBG(0, "\nEndpoint clear feature requested. Not implemented.");
        break;
    case USB_REQUEST_SET_FEATURE:
        DBG(0, "\nEndpoint set feature requested. Not implemented.");
        break;
    case USB_REQUEST_SYNC_FRAME:
        DBG(0, "\nEndpoint sync frame requested. Not implemented.");
        break;
    }
}


void printEP(uint16_t epid)
{
    DBG(0, "\nEP %d:", epid);

    if(sp.bmRequestType.bits.direction == 0)
        DBG(0, "[To Device] ");
    else
        DBG(0, "[To Host] ");


    if(sp.bmRequestType.bits.type == 0)
        DBG(0, "[Standard] ");
    else if(sp.bmRequestType.bits.type == 1)
        DBG(0, "[Class] ");
    else if(sp.bmRequestType.bits.type == 2)
        DBG(0, "[Vendor] ");
    else
        DBG(0, "[Reserved] ");


    if(sp.bmRequestType.bits.recipient == 0)
        DBG(0, "[Device] ");
    else if(sp.bmRequestType.bits.recipient == 1)
        DBG(0, "[Interface] ");
    else if(sp.bmRequestType.bits.recipient == 2)
        DBG(0, "[Endpoint] ");
    else if(sp.bmRequestType.bits.recipient == 3)
        DBG(0, "[Other] ");
    else
        DBG(0, "[Reserved] ");


    if(sp.bRequest == 0)
        DBG(0, "[Get_Status] ");
    else if(sp.bRequest == 1)
        DBG(0, "[Clear_Feature] ");
    else if(sp.bRequest == 3)
        DBG(0, "[Set_Feature] ");
    else if(sp.bRequest == 5)
        DBG(0, "[Set_Address] ");
    else if(sp.bRequest == 6)
        DBG(0, "[Get_Descriptor] ");
    else if(sp.bRequest == 7)
        DBG(0, "[Set_Descriptor] ");
    else if(sp.bRequest == 8)
        DBG(0, "[Get_Configuration] ");
    else if(sp.bRequest == 9)
        DBG(0, "[Set_Configuration] ");
    else if(sp.bRequest == 10)
        DBG(0, "[Get_Interface] ");
    else if(sp.bRequest == 17)
        DBG(0, "[Set_Interface] ");
    else
        DBG(0, "[Sync_Frame] ");

    DBG(0, "%d %d %d", sp.wValue, sp.wIndex, sp.wLength);

}

inline void USB_CTR(uint16_t epid)
{
    volatile uint16_t epreg = _GetENDPOINT(epid);

    if(epreg & EP_CTR_RX)                  // If we have correctly received a packet...
    {
        if(epreg & EP_SETUP)
        {
            USB_ReadEP(epid, (uint8_t *)&sp);
            printEP(epid);

            switch(sp.bmRequestType.bits.recipient)
            {
            case TO_DEVICE:
                USB_SetupDevice();
                break;
            case TO_INTERFACE:
                USB_SetupInterface();
                break;
            case TO_ENDPOINT:
                USB_SetupEndpoint();
                break;
            default:
                DBG(0, "\nSetup packet not addressed to device, interface, or endpoint. Feature not implemented.");
                break;
            }
        }
        else
        {
            uint32_t numbytes = USB_ReadEP(epid, EP0Buffer);
            DBG(0, "\nCTR RX -- Out Request on endpoint %d of %d bytes: ", epid, numbytes);
            printEP0Buffer(numbytes);
        }

        _ClearEP_CTR_RX(epid);
    }

    if(epreg & EP_CTR_TX)
    {
        //uint32_t numbytes = USB_ReadEP(0, EP0Buffer);
        if(epid == 1)
        {
            static uint16_t junk = 0;
            USB_WriteEP(1, (uint8_t *)&junk, 2);
            junk++;
        }
        _ClearEP_CTR_TX(epid);
        //DBG(0, "\nCTR TX Complete on EP %d. ", epid);
        //printEP0Buffer(numbytes);
    }


}




inline void USB_Error()
{
    DBG(0, "\nUSB Error");
}



inline void USB_Suspend()
{
    DBG(0, "\nUSB Suspend");
    uint16_t cntr = _GetCNTR();
    _SetCNTR(cntr | CNTR_FSUSP | CNTR_LPMODE);
}



inline void USB_Wakeup()
{
    DBG(0, "\nUSB Wakeup");
    uint16_t cntr = _GetCNTR();
    _SetCNTR(cntr & ~CNTR_FSUSP);
}



inline void USB_PMAOverrun()
{
    DBG(0, "\nUSB PMA Overrun");
}



void USB_LP_CAN1_RX0_IRQHandler()
{
    volatile uint16_t istr = _GetISTR();

    if(istr & ISTR_RESET)
    {
        USB_Reset();
        _SetISTR(istr & CLR_RESET);
    }
    if(istr & ISTR_SUSP)
    {
        USB_Suspend();
        _SetISTR(istr & CLR_SUSP);
    }
    if(istr & ISTR_WKUP)
    {
        USB_Wakeup();
        _SetISTR(istr & CLR_WKUP);
    }
    if(istr & ISTR_POVR)
    {
        USB_PMAOverrun();
        _SetISTR(istr & CLR_POVR);
    }
    if(istr & ISTR_ERR)
    {
        USB_Error();
        _SetISTR(istr & CLR_ERR);
    }

    while((istr = _GetISTR()) & ISTR_CTR)
    {
        _SetISTR(~ISTR_CTR);
        uint16_t epid = istr & ISTR_EP_ID;
        USB_CTR(epid);
        //DBG(0, "\nCTR on EP %d", epid);
    }
}


int main(void)
{
    USB_Init();
    while(1)
    {

    }
}
