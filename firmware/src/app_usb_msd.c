/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_usb_msd.c

  Summary:
    This file contains the source code for the MPLAB Harmony application code related to USB MSD functionality.

  Description:
 
 *******************************************************************************/

// *****************************************************************************


#include "app_usb_msd.h"
#include "wdrv_pic32mzw_client_api.h"
#include "wolfcrypt/asn.h"


#ifdef AZURE_CLOUD_DEMO
    #include "app_azure.h"
#elif defined AWS_CLOUD_DEMO
    #include "app_aws.h"
#endif

int g_msd_ready = 0;
// *****************************************************************************

#define APP_CTRL_CLIENTID_SIZE ((2 * KEYID_SIZE) + 1)

// *****************************************************************************

uint8_t CACHE_ALIGN work[SYS_FS_FAT_MAX_SS];
extern char g_Cloud_Endpoint[100];
#ifdef AWS_CLOUD_DEMO
    extern char g_Aws_ClientID[CLIENT_IDENTIFIER_MAX_LENGTH];
#endif

// *****************************************************************************
int APP_USB_MSD_Get_STATUS()
{
    return g_msd_ready;
}
/* USB event callback */
void USBDeviceEventHandler(USB_DEVICE_EVENT event, void * pEventData, uintptr_t context)
{
    APP_USB_MSD_DATA * appData = (APP_USB_MSD_DATA*) context;
    switch (event) {
    case USB_DEVICE_EVENT_RESET:
    case USB_DEVICE_EVENT_DECONFIGURED:
        break;

    case USB_DEVICE_EVENT_CONFIGURED:
        break;

    case USB_DEVICE_EVENT_SUSPENDED:
        break;

    case USB_DEVICE_EVENT_POWER_DETECTED:
        /* VBUS is detected. Attach the device. */
        USB_DEVICE_Attach(appData->usbDeviceHandle);
        break;

    case USB_DEVICE_EVENT_POWER_REMOVED:
        /* VBUS is not detected. Detach the device */
        USB_DEVICE_Detach(appData->usbDeviceHandle);
        break;

        /* These events are not used in this demo */
    case USB_DEVICE_EVENT_RESUMED:
    case USB_DEVICE_EVENT_ERROR:
    case USB_DEVICE_EVENT_SOF:
    default:
        break;
    }
}

#if SYS_FS_AUTOMOUNT_ENABLE
/* FS event callback*/
void SysFSEventHandler(SYS_FS_EVENT event, void* eventData, uintptr_t context)
{
    switch (event) {
    case SYS_FS_EVENT_MOUNT:
        if (strcmp((const char *) eventData, SYS_FS_MEDIA_IDX0_MOUNT_NAME_VOLUME_IDX0) == 0) {
            appUSBMSDData.fsMounted = true;
        }
        break;
    case SYS_FS_EVENT_UNMOUNT:
        if (strcmp((const char *) eventData, SYS_FS_MEDIA_IDX0_MOUNT_NAME_VOLUME_IDX0) == 0) {
            appUSBMSDData.fsMounted = false;
        }
        break;
    case SYS_FS_EVENT_ERROR:
        break;
    }
}
#endif








static bool checkFSMount(){
#if !SYS_FS_AUTOMOUNT_ENABLE

    if(SYS_FS_Mount(SYS_FS_MEDIA_IDX0_DEVICE_NAME_VOLUME_IDX0, SYS_FS_MEDIA_IDX0_MOUNT_NAME_VOLUME_IDX0, FAT, 0, NULL) != SYS_FS_RES_SUCCESS){
        appUSBMSDData.fsMounted = false;
    }
    else{
        appUSBMSDData.fsMounted = true;
    }
#endif  
    return appUSBMSDData.fsMounted;          
}

// *****************************************************************************
void APP_SoftResetDevice(void) {
    bool int_flag = false;

    /*disable interrupts since we are going to do a sysKey unlock*/
    int_flag = (bool) __builtin_disable_interrupts();

    /* unlock system for clock configuration */
    SYSKEY = 0x00000000;
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;

    if (int_flag) {
        __builtin_mtc0(12, 0, (__builtin_mfc0(12, 0) | 0x0001)); /* enable interrupts */
    }

    RSWRSTbits.SWRST = 1;
    /*This read is what actually causes the reset*/
    RSWRST = RSWRSTbits.SWRST;

    /*Reference code. We will not hit this due to reset. This is here for reference.*/
    int_flag = (bool) __builtin_disable_interrupts();

    SYSKEY = 0x33333333;

    if (int_flag) /* if interrupts originally were enabled, re-enable them */ {
        __builtin_mtc0(12, 0, (__builtin_mfc0(12, 0) | 0x0001));
    }

}

void APP_RewriteWifiConfigFile(void) 
{
    appUSBMSDData.USBMSDTaskState = APP_USB_MSD_DEINIT;
    appUSBMSDData.wifiConfigRewrite = true;
}

void APP_USB_MSD_Initialize ( void )
{
    appUSBMSDData.USBMSDTaskState = APP_USB_MSD_INIT;
    appUSBMSDData.fsMounted = false;
    appUSBMSDData.wifiConfigRewrite = false;
    appUSBMSDData.usbDeviceHandle = USB_DEVICE_HANDLE_INVALID;
    memset(appUSBMSDData.ecc608SerialNum, 0, sizeof(appUSBMSDData.ecc608SerialNum));
#if SYS_FS_AUTOMOUNT_ENABLE
    SYS_FS_EventHandlerSet(SysFSEventHandler, (uintptr_t) NULL);
#endif
}

/* Application USB MSD main task */
void APP_USB_MSD_Tasks ( void )
{

    SYS_FS_FORMAT_PARAM opt;

    switch (appUSBMSDData.USBMSDTaskState) {
        /* Application's initial state. */
        case APP_USB_MSD_INIT:
        {
            appUSBMSDData.USBMSDTaskState = APP_USB_MSD_WAIT_FS_MOUNT;
            break;
        }
        
        /* Pending state */
        case APP_USB_MSD_PENDING:
        {
            break;
        }
        
        /* Waiting for FS mount */
        case APP_USB_MSD_WAIT_FS_MOUNT:
        {
            if (checkFSMount()) {
                APP_USB_MSD_PRNT("FS Mounted \r\n");
                if (0) 
                {
                    APP_USB_MSD_PRNT("Factory config reset requested\r\n");
                    
                    appUSBMSDData.USBMSDTaskState = APP_USB_MSD_CLEAR_DRIVE;
                } 
                else if (SYS_FS_ERROR_NO_FILESYSTEM==SYS_FS_Error())
                {
                    APP_USB_MSD_PRNT("No Filesystem. Doing a format\r\n");
                    appUSBMSDData.USBMSDTaskState = APP_USB_MSD_CLEAR_DRIVE;
                }
                else 
                {
                    appUSBMSDData.USBMSDTaskState = APP_USB_MSD_TOUCH_CLOUD_FILES;
                }
                
                SYS_FS_FileDirectoryRemove("FILE.txt");
            }
            break;
        }
        
        case APP_USB_MSD_CLEAR_DRIVE:
        {
            opt.fmt = SYS_FS_FORMAT_FAT;
            opt.au_size = 0;
            if (SYS_FS_DriveFormat (SYS_FS_MEDIA_IDX0_MOUNT_NAME_VOLUME_IDX0, &opt, (void *)work, SYS_FS_FAT_MAX_SS) != SYS_FS_RES_SUCCESS)
            {
                /* Format of the disk failed. */
                APP_USB_MSD_DBG(SYS_ERROR_ERROR, "Media Format failed\r\n");
                appUSBMSDData.USBMSDTaskState = APP_USB_MSD_ERROR;
            }
            else
            {
                /* Format succeeded. Open a file. */
                appUSBMSDData.USBMSDTaskState = APP_USB_MSD_TOUCH_CLOUD_FILES;
            }
            break;
        }
        
        /* Write cloud config file and web pages' files if not already existing*/
        case APP_USB_MSD_TOUCH_CLOUD_FILES:
        {
            SYS_FS_DriveLabelSet(SYS_FS_MEDIA_IDX0_MOUNT_NAME_VOLUME_IDX0,APP_USB_MSD_DRIVE_NAME);
            SYS_FS_CurrentDriveSet(SYS_FS_MEDIA_IDX0_MOUNT_NAME_VOLUME_IDX0);


            
            appUSBMSDData.USBMSDTaskState = APP_USB_MSD_TOUCH_WIFI_CONFIG_FILE;          
            break;
        }

        /* Write and Read Wi-Fi configuration file */
        case APP_USB_MSD_TOUCH_WIFI_CONFIG_FILE:
        {   

            appUSBMSDData.USBMSDTaskState = APP_USB_MSD_CONNECT;
            break;
        }
        
        case APP_USB_MSD_CONNECT:
        {
            appUSBMSDData.usbDeviceHandle = USB_DEVICE_Open(USB_DEVICE_INDEX_0, DRV_IO_INTENT_READWRITE);
            if (appUSBMSDData.usbDeviceHandle != USB_DEVICE_HANDLE_INVALID) {
                /* Set the Event Handler. We will start receiving events after
                 * the handler is set */
                USB_DEVICE_EventHandlerSet(appUSBMSDData.usbDeviceHandle, USBDeviceEventHandler, (uintptr_t) & appUSBMSDData);
                APP_USB_MSD_PRNT("USB Drive init finish \r\n");
                g_msd_ready = 1;
                appUSBMSDData.USBMSDTaskState = APP_USB_MSD_IDLE;
            } else {
                appUSBMSDData.USBMSDTaskState = APP_USB_MSD_ERROR;
            }
            break;
        }

        /* Unmount FS */
        case APP_USB_MSD_FS_UNMOUNT:
        {
            if(SYS_FS_Unmount(SYS_FS_MEDIA_IDX0_MOUNT_NAME_VOLUME_IDX0) != SYS_FS_RES_SUCCESS)
            {
                APP_USB_MSD_DBG(SYS_ERROR_ERROR, "FS unmount failed\r\n");
                appUSBMSDData.USBMSDTaskState = APP_USB_MSD_ERROR;
            }
            else
            {
                appUSBMSDData.USBMSDTaskState = APP_USB_MSD_DEINIT;
            }
            break;
        }
        
        /* USB de-init */
        case APP_USB_MSD_DEINIT:
        {
            USB_DEVICE_Detach(appUSBMSDData.usbDeviceHandle);
            USB_DEVICE_Close(appUSBMSDData.usbDeviceHandle);
            APP_USB_MSD_DBG(SYS_ERROR_INFO, "USB device closed\r\n");
            if(appUSBMSDData.wifiConfigRewrite){
                SYS_FS_FileDirectoryRemove(APP_USB_MSD_WIFI_CONFIG_FILE_NAME);
                appUSBMSDData.USBMSDTaskState = APP_USB_MSD_TOUCH_WIFI_CONFIG_FILE;
            }
                
            break;
        }
        
        /* Idle */
        case APP_USB_MSD_IDLE:
        {
            break;
        }
        
        /* Error */
        case APP_USB_MSD_ERROR:
        {
            APP_USB_MSD_DBG(SYS_ERROR_ERROR, "APP_USB_MSD_ERROR\r\n");
            /* Always inform APP_Task() about credentials validity to avoid code blocking*/

            appUSBMSDData.USBMSDTaskState = APP_USB_MSD_IDLE;
            break;
        }
        
        default:
        {
            break;
        }
    }
}

/*******************************************************************************
 End of File
 */

