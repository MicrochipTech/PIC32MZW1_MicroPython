/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * Development of the code in this file was sponsored by Microbric Pty Ltd
 * and Mnemote Pty Ltd
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016, 2017 Nick Moore @mnemote
 * Copyright (c) 2017 "Eric Poulsen" <eric@zyxod.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <string.h>

#include "py/objlist.h"
#include "py/runtime.h"
#include "modnetwork.h"

#include "wdrv_pic32mzw_client_api.h"
#include "definitions.h"

#define WIFI_IF_STA 0
#define WIFI_IF_AP 1

STATIC const mp_obj_type_t wlan_if_type;
STATIC const wlan_if_obj_t wlan_sta_obj = {{&wlan_if_type}, WIFI_IF_STA};
STATIC const wlan_if_obj_t wlan_ap_obj = {{&wlan_if_type}, WIFI_IF_AP};

static bool wifi_sta_connected = false;
static bool wifi_sta_is_connecting = false;
static bool wifi_init_flg = false;

void WiFiServCallback (uint32_t event, void * data,void *cookie )
{
    switch(event)
    {
        case SYS_WIFI_CONNECT:
        {
            //In STA mode, Wi-Fi service share IP address provided by AP in the callback
            wifi_sta_connected = true;
            wifi_sta_is_connecting = false;
#if 0
            //In AP mode, Wi-Fi service share MAC address and IP address of the connected STA in the callback
            SYS_WIFI_STA_APP_INFO    *StaConnInfo = (SYS_WIFI_STA_APP_INFO *)data;
            SYS_CONSOLE_PRINT("STA Connected to AP. Got IP address = %d.%d.%d.%d \r\n", 
                    StaConnInfo->ipAddr.v[0], StaConnInfo->ipAddr.v[1], StaConnInfo->ipAddr.v[2], StaConnInfo->ipAddr.v[3]);                
            SYS_CONSOLE_PRINT("STA Connected to AP. Got MAC address = %x:%x:%x:%x:%x:%x \r\n", 
                    StaConnInfo->macAddr[0], StaConnInfo->macAddr[1], StaConnInfo->macAddr[2],
                    StaConnInfo->macAddr[3], StaConnInfo->macAddr[4], StaConnInfo->macAddr[5]);
#endif
            break;
        }
        case SYS_WIFI_DISCONNECT:
        {
            SYS_CONSOLE_PRINT("Device DISCONNECTED \\r\\n");
            wifi_sta_connected = false;
            wifi_sta_is_connecting = false;
            break;
        }
        case SYS_WIFI_PROVCONFIG:
        {
            SYS_CONSOLE_PRINT("Received the Provisioning data \\r\\n");
            break;
        }
    }
}



STATIC mp_obj_t get_wlan(size_t n_args, const mp_obj_t *args) {
    static int initialized = 0;
    if (!initialized) {
        initialized = 1;
    }
    int idx = (n_args > 0) ? mp_obj_get_int(args[0]) : WIFI_IF_STA;
    if (idx == WIFI_IF_STA) {
        return MP_OBJ_FROM_PTR(&wlan_sta_obj);
    } else if (idx == WIFI_IF_AP) {
        return MP_OBJ_FROM_PTR(&wlan_ap_obj);
    } else {
        mp_raise_ValueError(MP_ERROR_TEXT("invalid WLAN interface identifier"));
    }
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(get_wlan_obj, 0, 1, get_wlan);

SYS_WIFI_CONFIG    wifiSrvcConfig;
STATIC mp_obj_t wlan_connect(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_ssid, ARG_password, ARG_bssid };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_bssid, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };
    
    

    memset(&wifiSrvcConfig, 0, sizeof(wifiSrvcConfig));
    // Set mode as STA 
    wifiSrvcConfig.mode = SYS_WIFI_STA;

    // Disable saving wifi configuration 
    wifiSrvcConfig.saveConfig = false;   

    // Set the auth type to SYS_WIFI_WPA2
    wifiSrvcConfig.staConfig.authType = SYS_WIFI_WPAWPA2MIXED;//SYS_WIFI_WPA2;

    // Enable all the channels(0)
    wifiSrvcConfig.staConfig.channel = 0;

    // Device doesn't wait for user request
    wifiSrvcConfig.staConfig.autoConnect = 1;

    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // configure any parameters that are given
    if (n_args > 1) {
        size_t len;
        const char *p;
        if (args[ARG_ssid].u_obj != mp_const_none) {
            p = mp_obj_str_get_data(args[ARG_ssid].u_obj, &len);
#ifdef PIC32MZW1_DEBUG
            SYS_CONSOLE_PRINT("ssid = %s, len = %d\r\n", p, len);
#endif
            memcpy(wifiSrvcConfig.staConfig.ssid, p, len);
        }
        if (args[ARG_password].u_obj != mp_const_none) {
            p = mp_obj_str_get_data(args[ARG_password].u_obj, &len);
#ifdef PIC32MZW1_DEBUG          
            SYS_CONSOLE_PRINT("pw = %s, len = %d\r\n", p, len);
#endif
            memcpy(wifiSrvcConfig.staConfig.psk, p, len);
        }
        if (args[ARG_bssid].u_obj != mp_const_none) {
            p = mp_obj_str_get_data(args[ARG_bssid].u_obj, &len);
#ifdef PIC32MZW1_DEBUG
            SYS_CONSOLE_PRINT("bssid = %x:%x:%x\r\n", p[0], p[1], p[1]);
#endif
        }

    }

    MP_THREAD_GIL_EXIT();

    if (SYS_WIFI_OBJ_INVALID == SYS_WIFI_CtrlMsg (sysObj.syswifi, SYS_WIFI_CONNECT, &wifiSrvcConfig, sizeof(SYS_WIFI_CONFIG)))
    {
        SYS_CONSOLE_PRINT("[%s] fail to send wifi connect command\r\n", __func__);
    }

    
    wifi_sta_is_connecting = true;
    
    MP_THREAD_GIL_ENTER();

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(wlan_connect_obj, 1, wlan_connect);

STATIC mp_obj_t network_wlan_isconnected(mp_obj_t self_in) {
    wlan_if_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (self->if_id == WIFI_IF_STA) {
        return mp_obj_new_bool(wifi_sta_connected);
    } else {
        // ToDo: the handle of AP mode
        return mp_obj_new_bool(wifi_sta_connected);
    }
    
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(network_wlan_isconnected_obj, network_wlan_isconnected);

STATIC mp_obj_t network_wlan_active(size_t n_args, const mp_obj_t *args) {
    //wlan_if_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    
    if (n_args > 1) {
        bool active = mp_obj_is_true(args[1]);

        if (active && !wifi_init_flg)
        {
#ifdef PIC32MZW1_DEBUG
            SYS_CONSOLE_MESSAGE("Turn on WiFi\r\n");
#endif
            SYS_MODULE_OBJ wifi_obj = SYS_WIFI_Initialize(NULL, NULL, NULL);
            if (wifi_obj != SYS_MODULE_OBJ_INVALID)
            {
                
                wifi_init_flg = true;
                sysObj.syswifi = wifi_obj;

                while (SYS_WIFI_SERVICE_UNINITIALIZE == SYS_WIFI_CtrlMsg (sysObj.syswifi, SYS_WIFI_GETWIFICONFIG, &wifiSrvcConfig, sizeof(SYS_WIFI_CONFIG)));

                SYS_WIFI_CtrlMsg(sysObj.syswifi,SYS_WIFI_REGCALLBACK,WiFiServCallback,sizeof(uint8_t *));
            }
        }
        else if (active)
        {
#ifdef PIC32MZW1_DEBUG            
            SYS_CONSOLE_MESSAGE("Turn on WiFi again\r\n");
#endif
            SYS_WIFI_RESULT res = SYS_WIFI_Deinitialize(sysObj.syswifi);
            if (res == SYS_WIFI_SUCCESS)
                wifi_init_flg = false;

            SYS_MODULE_OBJ wifi_obj = SYS_WIFI_Initialize(NULL, NULL, NULL);
            if (wifi_obj != SYS_MODULE_OBJ_INVALID)
            {
                wifi_init_flg = true;
                sysObj.syswifi = wifi_obj;
                
                while (SYS_WIFI_SERVICE_UNINITIALIZE == SYS_WIFI_CtrlMsg (sysObj.syswifi, SYS_WIFI_GETWIFICONFIG, &wifiSrvcConfig, sizeof(SYS_WIFI_CONFIG)));

                SYS_WIFI_CtrlMsg(sysObj.syswifi,SYS_WIFI_REGCALLBACK,WiFiServCallback,sizeof(uint8_t *));
            }
        }
        else if (!active && wifi_init_flg)
        {
            SYS_CONSOLE_MESSAGE("Turn off WiFi\r\n");
            SYS_WIFI_RESULT res = SYS_WIFI_Deinitialize(sysObj.syswifi);
            if (res == SYS_WIFI_SUCCESS)
                wifi_init_flg = false;
        }
    }
    
    return (wifi_init_flg) ? mp_const_true : mp_const_false;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(network_wlan_active_obj, 1, 2, network_wlan_active);

STATIC mp_obj_t network_wlan_disconnect(mp_obj_t self_in) {
    // disconnect to the WiFi AP
    MP_THREAD_GIL_EXIT();
        
    if (SYS_WIFI_OBJ_INVALID == SYS_WIFI_CtrlMsg (sysObj.syswifi, SYS_WIFI_DISCONNECT, NULL, 0))
    {
        SYS_CONSOLE_PRINT("[%s] fail to send wifi disconnect command\r\n", __func__);
    }
    
    MP_THREAD_GIL_ENTER();
    
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(network_wlan_disconnect_obj, network_wlan_disconnect);

STATIC mp_obj_t network_wlan_status(size_t n_args, const mp_obj_t *args) {
    wlan_if_obj_t *self = MP_OBJ_TO_PTR(args[0]);

    if (n_args == 1) {
        if (self->if_id == WIFI_IF_STA) {
            // Case of no arg is only for the STA interface
            if (wifi_sta_connected) {
                // Happy path, connected with IP
                return MP_OBJ_NEW_SMALL_INT(STAT_GOT_IP);
            } else if (wifi_sta_is_connecting) {
                // No connection or error, but is requested = Still connecting
                return MP_OBJ_NEW_SMALL_INT(STAT_CONNECTING);
            } else if (!wifi_sta_connected) {
                // No activity, No error = Idle
                return MP_OBJ_NEW_SMALL_INT(STAT_IDLE);
            } else {

            }
        }
        return mp_const_none;
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(network_wlan_status_obj, 1, 2, network_wlan_status);

bool scan_is_done = false;
int scan_AP_num = 0;
WDRV_PIC32MZW_BSS_INFO scan_list[15];

/* Wi-Fi driver update the Scan result on callback*/
void SYS_WIFI_ScanHandler
(
    DRV_HANDLE handle, 
    uint8_t index, 
    uint8_t ofTotal, 
    WDRV_PIC32MZW_BSS_INFO *pBSSInfo
) 
{
    if (0 == ofTotal) 
    {
        SYS_CONSOLE_MESSAGE("No AP Found Rescan\r\n");
        scan_is_done = true;
        scan_AP_num = ofTotal;
    } 
    else 
    {
        if (1 == index) 
        {
            
            char cmdTxt[10];
            sprintf(cmdTxt, "SCAN#%02d", ofTotal);
            SYS_CONSOLE_PRINT("#%02d\r\n", ofTotal);
            SYS_CONSOLE_MESSAGE(">>#  RI  Sec  Recommend CH BSSID             SSID\r\n");
            SYS_CONSOLE_MESSAGE(">>#      Cap  Auth Type\r\n>>#\r\n");
        
            memset(scan_list, 0, sizeof(scan_list));
        }
        
        if (ofTotal == index )
        {
            scan_is_done = true;
            scan_AP_num = ofTotal;
            SYS_CONSOLE_PRINT("ofTotal = %d\r\n", ofTotal);
        }
        SYS_CONSOLE_PRINT(">>%02d %d 0x%02x ", index, pBSSInfo->rssi, pBSSInfo->secCapabilities);
        switch (pBSSInfo->authTypeRecommended) 
        {
            case WDRV_PIC32MZW_AUTH_TYPE_OPEN:
            {
                SYS_CONSOLE_MESSAGE("OPEN     ");
                break;
            }

            case WDRV_PIC32MZW_AUTH_TYPE_WEP:
            {
                SYS_CONSOLE_MESSAGE("WEP      ");
                break;
            }

            case WDRV_PIC32MZW_AUTH_TYPE_WPAWPA2_PERSONAL:
            {
                SYS_CONSOLE_MESSAGE("WPA/2 PSK");
                break;
            }

            case WDRV_PIC32MZW_AUTH_TYPE_WPA2_PERSONAL:
            {
                SYS_CONSOLE_MESSAGE("WPA2 PSK ");
                break;
            }

            default:
            {
                SYS_CONSOLE_MESSAGE("Not Avail");
                break;
            }

        }
        /*
        SYS_CONSOLE_PRINT(" %02d %02X:%02X:%02X:%02X:%02X:%02X %s\r\n", pBSSInfo->ctx.channel,
                pBSSInfo->ctx.bssid.addr[0], pBSSInfo->ctx.bssid.addr[1], pBSSInfo->ctx.bssid.addr[2],
                pBSSInfo->ctx.bssid.addr[3], pBSSInfo->ctx.bssid.addr[4], pBSSInfo->ctx.bssid.addr[5],
                pBSSInfo->ctx.ssid.name);
        */
        if (index > 0)
        {
            memcpy(&scan_list[index -1], pBSSInfo, sizeof(WDRV_PIC32MZW_BSS_INFO));
        }
    }
}

STATIC mp_obj_t network_wlan_scan(mp_obj_t self_in) {
    // check that STA mode is active
    
    SYS_WIFI_SCAN_CONFIG wifiSrvcScanConfig;
    if(SYS_WIFI_SUCCESS == SYS_WIFI_CtrlMsg(sysObj.syswifi, SYS_WIFI_GETSCANCONFIG, &wifiSrvcScanConfig, sizeof(SYS_WIFI_SCAN_CONFIG)))
    {
          //Received the wifiSrvcScanConfig data
    }

    // update desired parameters
    //char myAPlist[] = "openAP,SecuredAP,DEMO_AP,my_cell_hotspot";
    char myAPlist[] = "";
    char delimiter  = ',';
    wifiSrvcScanConfig.channel    =  0;
    wifiSrvcScanConfig.mode       =  SYS_WIFI_SCAN_MODE_ACTIVE;
    wifiSrvcScanConfig.pSsidList  =  myAPlist;
    wifiSrvcScanConfig.delimChar  =  delimiter;
    wifiSrvcScanConfig.pNotifyCallback = SYS_WIFI_ScanHandler;
    // pass structure in scan request
    SYS_WIFI_RESULT res = SYS_WIFI_CtrlMsg(sysObj.syswifi, SYS_WIFI_SCANREQ, &wifiSrvcScanConfig, sizeof(SYS_WIFI_SCAN_CONFIG));
    SYS_CONSOLE_PRINT("res = %d\r\n", res);
    //if ( SYS_WIFI_SUCCESS != SYS_WIFI_CtrlMsg(sysObj.syswifi, SYS_WIFI_SCANREQ, &wifiSrvcScanConfig, sizeof(SYS_WIFI_SCAN_CONFIG)));
    //{            
    //    return mp_const_none;
    //}
            
    int counter = 100;
    scan_is_done = false;
    while (!scan_is_done && counter > 0)
    {
        counter--;
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    
    SYS_CONSOLE_MESSAGE("Printing...\r\n");
    for (int i = 0; i<scan_AP_num; i++)
    {
        SYS_CONSOLE_PRINT(" %02d %02X:%02X:%02X:%02X:%02X:%02X %s\r\n", scan_list[i].ctx.channel,
                scan_list[i].ctx.bssid.addr[0], scan_list[i].ctx.bssid.addr[1], scan_list[i].ctx.bssid.addr[2],
                scan_list[i].ctx.bssid.addr[3], scan_list[i].ctx.bssid.addr[4], scan_list[i].ctx.bssid.addr[5],
                scan_list[i].ctx.ssid.name);
    }
    
    mp_obj_t list = mp_obj_new_list(0, NULL);
    
    for (uint16_t i = 0; i < scan_AP_num; i++) {
        mp_obj_tuple_t *t = mp_obj_new_tuple(6, NULL);
        t->items[0] = mp_obj_new_bytes(scan_list[i].ctx.ssid.name, scan_list[i].ctx.ssid.length);
        t->items[1] = mp_obj_new_bytes(scan_list[i].ctx.bssid.addr, sizeof(scan_list[i].ctx.bssid.addr));
        t->items[2] = MP_OBJ_NEW_SMALL_INT(scan_list[i].ctx.channel);
        t->items[3] = MP_OBJ_NEW_SMALL_INT(scan_list[i].rssi);
        t->items[4] = MP_OBJ_NEW_SMALL_INT(scan_list[i].authTypeRecommended);
        t->items[5] = mp_const_false; // XXX hidden?
        mp_obj_list_append(list, MP_OBJ_FROM_PTR(t));
    }
    
    return list;
    
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(network_wlan_scan_obj, network_wlan_scan);


STATIC mp_obj_t network_wlan_config(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
    
    if (n_args != 1 && kwargs->used != 0) {
        mp_raise_TypeError(MP_ERROR_TEXT("either pos or kw args are allowed"));
    }

    wlan_if_obj_t *self = MP_OBJ_TO_PTR(args[0]);

    bool is_wifi_ap = self->if_id == WIFI_IF_AP;

    if (kwargs->used != 0) {
        if (!is_wifi_ap) {
            goto unknown;
        }

        for (size_t i = 0; i < kwargs->alloc; i++) {
            if (mp_map_slot_is_filled(kwargs, i)) {
                int req_if = -1;

                switch (mp_obj_str_get_qstr(kwargs->table[i].key)) {
                    case MP_QSTR_mac: {
                        
                        break;
                    }
                    case MP_QSTR_essid: {
                        
                        req_if = WIFI_IF_AP;
                        size_t len;
                        const char *s = mp_obj_str_get_data(kwargs->table[i].value, &len);
                        len = MIN(len, sizeof(wifiSrvcConfig.apConfig.ssid));
                        memcpy(wifiSrvcConfig.apConfig.ssid, s, len);
                        wifiSrvcConfig.mode = SYS_WIFI_AP;
                        wifiSrvcConfig.apConfig.authType = WDRV_PIC32MZW_AUTH_TYPE_WPAWPA2_PERSONAL;
                        wifiSrvcConfig.apConfig.channel = 0;
                        wifiSrvcConfig.apConfig.ssidVisibility = 1;
                        wifiSrvcConfig.saveConfig = false;
                        //wifiSrvcConfig.apConfig.ssid = len;
                        break;
                    }
                    case MP_QSTR_hidden: {
                        
                        break;
                    }
                    case MP_QSTR_authmode: {
                        req_if = WIFI_IF_AP;
                        wifiSrvcConfig.apConfig.authType = mp_obj_get_int(kwargs->table[i].value);
                        break;
                    }
                    case MP_QSTR_password: {
                        
                        req_if = WIFI_IF_AP;
                        size_t len;
                        const char *s = mp_obj_str_get_data(kwargs->table[i].value, &len);
                        len = MIN(len, sizeof(wifiSrvcConfig.apConfig.psk) - 1);
                        memcpy(wifiSrvcConfig.apConfig.psk, s, len);
                        wifiSrvcConfig.apConfig.psk[len] = 0;
                        break;
                    }
                    case MP_QSTR_channel: {
                        req_if = WIFI_IF_AP;
                        wifiSrvcConfig.apConfig.channel = mp_obj_get_int(kwargs->table[i].value);
                        break;
                    }
                    case MP_QSTR_max_clients: {
                        
                        break;
                    }
                    default:
                        goto unknown;
                }
            }
        }
        
        

        if (SYS_WIFI_OBJ_INVALID == SYS_WIFI_CtrlMsg (sysObj.syswifi, SYS_WIFI_CONNECT, &wifiSrvcConfig, sizeof(SYS_WIFI_CONFIG)))
        {
            SYS_CONSOLE_PRINT("[%s] fail to send wifi connect command\r\n", __func__);
            return mp_const_false;
        }

        return mp_const_true;

    }



    // Get config

    if (n_args != 2) {
        mp_raise_TypeError(MP_ERROR_TEXT("can query only one param"));
    }

    int req_if = -1;
    mp_obj_t val = mp_const_none;

    switch (mp_obj_str_get_qstr(args[1])) {
        case MP_QSTR_mac: {
            uint8_t mac[6];
            switch (self->if_id) {
                case WIFI_IF_AP: // fallthrough intentional
                case WIFI_IF_STA:
                    // To do
                    return mp_obj_new_bytes(mac, sizeof(mac));
                default:
                    goto unknown;
            }
        }
        case MP_QSTR_essid:
            switch (self->if_id) {
                case WIFI_IF_STA:
                    val = mp_obj_new_str((char *)wifiSrvcConfig.staConfig.ssid, strlen((char *)wifiSrvcConfig.staConfig.ssid));
                    break;
                case WIFI_IF_AP:
                    val = mp_obj_new_str((char *)wifiSrvcConfig.apConfig.ssid, strlen((char *)wifiSrvcConfig.apConfig.ssid));
                    break;
                default:
                    req_if = WIFI_IF_AP;
            }
            break;
        case MP_QSTR_hidden:
            req_if = WIFI_IF_AP;
            val = mp_obj_new_bool(wifiSrvcConfig.apConfig.ssidVisibility);
            break;
        case MP_QSTR_authmode:
            req_if = WIFI_IF_AP;
            val = MP_OBJ_NEW_SMALL_INT(wifiSrvcConfig.apConfig.authType);
            break;
        case MP_QSTR_channel:
            req_if = WIFI_IF_AP;
            val = MP_OBJ_NEW_SMALL_INT(wifiSrvcConfig.apConfig.channel);
            break;
        
        case MP_QSTR_max_clients: {
            val = MP_OBJ_NEW_SMALL_INT(8);
            break;
        }
        default:
            goto unknown;
    }

    return val;
    
unknown:
    mp_raise_ValueError(MP_ERROR_TEXT("unknown config param"));

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(network_wlan_config_obj, 1, network_wlan_config);


STATIC const mp_rom_map_elem_t wlan_if_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_active), MP_ROM_PTR(&network_wlan_active_obj) },
    { MP_ROM_QSTR(MP_QSTR_connect), MP_ROM_PTR(&wlan_connect_obj) },
    { MP_ROM_QSTR(MP_QSTR_disconnect), MP_ROM_PTR(&network_wlan_disconnect_obj) },
    { MP_ROM_QSTR(MP_QSTR_status), MP_ROM_PTR(&network_wlan_status_obj) },
    { MP_ROM_QSTR(MP_QSTR_scan), MP_ROM_PTR(&network_wlan_scan_obj) },
    { MP_ROM_QSTR(MP_QSTR_isconnected), MP_ROM_PTR(&network_wlan_isconnected_obj) },
    { MP_ROM_QSTR(MP_QSTR_config), MP_ROM_PTR(&network_wlan_config_obj) },
    { MP_ROM_QSTR(MP_QSTR_ifconfig), MP_ROM_PTR(&wfi32_ifconfig_obj) },
};
STATIC MP_DEFINE_CONST_DICT(wlan_if_locals_dict, wlan_if_locals_dict_table);

STATIC const mp_obj_type_t wlan_if_type = {
    { &mp_type_type },
    .name = MP_QSTR_WLAN,
    .locals_dict = (mp_obj_t)&wlan_if_locals_dict,
};


