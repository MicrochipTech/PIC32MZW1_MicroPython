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

#include "py/runtime.h"
#include "py/mperrno.h"
#include "shared/netutils/netutils.h"
#include "modnetwork.h"

#include "definitions.h"

#define MICROPY_PY_NETWORK_WLAN 1
#define WIFI_IF_STA 0
#define WIFI_IF_AP 1

#define MODNETWORK_INCLUDE_CONSTANTS (1)

STATIC mp_obj_t wfi32_network_init() {
    static int initialized = 0;
    if (!initialized) {
        //
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(wfi32_init_obj, wfi32_network_init);

STATIC mp_obj_t wfi32_ifconfig(size_t n_args, const mp_obj_t *args) {
    //wlan_if_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    IPV4_ADDR ipAddr;
    IPV4_ADDR netmask;
    IPV4_ADDR gateWayAddr;
    IPV4_ADDR dnsAddr;
    
    TCPIP_NET_HANDLE netHdl = TCPIP_STACK_NetHandleGet("PIC32MZW1");
    
        
    if (n_args == 1) {
        
    
        ipAddr.Val = TCPIP_STACK_NetAddress(netHdl);
        netmask.Val = TCPIP_STACK_NetMaskGet(netHdl);
        gateWayAddr.Val = TCPIP_STACK_NetAddressGateway(netHdl);
        dnsAddr.Val = TCPIP_STACK_NetAddressDnsPrimary(netHdl);
        // get
        mp_obj_t tuple[4] = {
            netutils_format_ipv4_addr((uint8_t *)&ipAddr.v, NETUTILS_BIG),
            netutils_format_ipv4_addr((uint8_t *)&netmask.v, NETUTILS_BIG),
            netutils_format_ipv4_addr((uint8_t *)&gateWayAddr.v, NETUTILS_BIG),
            netutils_format_ipv4_addr((uint8_t *)&dnsAddr.v, NETUTILS_BIG),
        };
        return mp_obj_new_tuple(4, tuple);
    } else {
        // set
        if (mp_obj_is_type(args[1], &mp_type_tuple) || mp_obj_is_type(args[1], &mp_type_list)) {
            mp_obj_t *items;
            mp_obj_get_array_fixed_n(args[1], 4, &items);
            netutils_parse_ipv4_addr(items[0], (void *)&ipAddr.v, NETUTILS_BIG);
            netutils_parse_ipv4_addr(items[1], (void *)&netmask.v, NETUTILS_BIG);           
            netutils_parse_ipv4_addr(items[2], (void *)&gateWayAddr.v, NETUTILS_BIG);
            netutils_parse_ipv4_addr(items[3], (void *)&dnsAddr.v, NETUTILS_BIG);
            
            TCPIP_STACK_NetAddressSet(netHdl, &ipAddr, &netmask, 0);
            TCPIP_STACK_NetAddressGatewaySet(netHdl, &gateWayAddr);
            TCPIP_STACK_NetAddressDnsPrimarySet(netHdl, &dnsAddr);
        }
    }

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(wfi32_ifconfig_obj, 1, 2, wfi32_ifconfig);



STATIC const mp_rom_map_elem_t mp_module_network_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_network) },
    { MP_ROM_QSTR(MP_QSTR___init__), MP_ROM_PTR(&wfi32_init_obj) },

    #if MICROPY_PY_NETWORK_WLAN
    { MP_ROM_QSTR(MP_QSTR_WLAN), MP_ROM_PTR(&get_wlan_obj) },
    #endif

    #if MODNETWORK_INCLUDE_CONSTANTS

    #if MICROPY_PY_NETWORK_WLAN
    { MP_ROM_QSTR(MP_QSTR_STA_IF), MP_ROM_INT(WIFI_IF_STA)},
    { MP_ROM_QSTR(MP_QSTR_AP_IF), MP_ROM_INT(WIFI_IF_AP)},
#if 0
    { MP_ROM_QSTR(MP_QSTR_MODE_11B), MP_ROM_INT(WIFI_PROTOCOL_11B) },
    { MP_ROM_QSTR(MP_QSTR_MODE_11G), MP_ROM_INT(WIFI_PROTOCOL_11G) },
    { MP_ROM_QSTR(MP_QSTR_MODE_11N), MP_ROM_INT(WIFI_PROTOCOL_11N) },

    { MP_ROM_QSTR(MP_QSTR_AUTH_OPEN), MP_ROM_INT(WIFI_AUTH_OPEN) },
    { MP_ROM_QSTR(MP_QSTR_AUTH_WEP), MP_ROM_INT(WIFI_AUTH_WEP) },
    { MP_ROM_QSTR(MP_QSTR_AUTH_WPA_PSK), MP_ROM_INT(WIFI_AUTH_WPA_PSK) },
    { MP_ROM_QSTR(MP_QSTR_AUTH_WPA2_PSK), MP_ROM_INT(WIFI_AUTH_WPA2_PSK) },
    { MP_ROM_QSTR(MP_QSTR_AUTH_WPA_WPA2_PSK), MP_ROM_INT(WIFI_AUTH_WPA_WPA2_PSK) },
    { MP_ROM_QSTR(MP_QSTR_AUTH_WPA2_ENTERPRISE), MP_ROM_INT(WIFI_AUTH_WPA2_ENTERPRISE) },
    { MP_ROM_QSTR(MP_QSTR_AUTH_WPA3_PSK), MP_ROM_INT(WIFI_AUTH_WPA3_PSK) },
    { MP_ROM_QSTR(MP_QSTR_AUTH_WPA2_WPA3_PSK), MP_ROM_INT(WIFI_AUTH_WPA2_WPA3_PSK) },
    { MP_ROM_QSTR(MP_QSTR_AUTH_MAX), MP_ROM_INT(WIFI_AUTH_MAX) },
#endif
    #endif


#endif
};

STATIC MP_DEFINE_CONST_DICT(mp_module_network_globals, mp_module_network_globals_table);

const mp_obj_module_t mp_module_network = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&mp_module_network_globals,
};
