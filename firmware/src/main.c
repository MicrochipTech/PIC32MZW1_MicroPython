/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

/*******************************************************************************
Copyright (C) 2020 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes

#include "py/compile.h"
#include "py/runtime.h"
#include "py/gc.h"
#include "py/mphal.h"
#include "py/mperrno.h"
#include "py/builtin.h"
#include "shared/runtime/pyexec.h"
#include "shared/readline/readline.h"
// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************
static char heap[64000];

//void TIMER2_InterruptSvcRoutine(uint32_t status, uintptr_t context)
//{
//    SYS_CONSOLE_MESSAGE("TMR2 IRQ\r\n");
    //TMR2_Stop();
//}
#define ADC_VREF                (3.3f)
#define ADC_MAX_COUNT           (4095)

uint16_t adc_count;
float input_voltage;

int main ( void )
{
    /* Initialize all modules */
    SYS_Initialize ( NULL );

    // init MicroPython runtime
    int stack_dummy;
    MP_STATE_THREAD(stack_top) = (char *)&stack_dummy;
    gc_init(heap, heap + sizeof(heap));
    micropython_init();
    ///mp_hal_init();
    readline_init0();
    
    //TMR1_Start();
#if 0
    while(!ADCHS_ChannelResultIsReady(ADCHS_CH14))
    {

    };
    
    adc_count = ADCHS_ChannelResultGet(ADCHS_CH14);
    input_voltage = (float)adc_count * ADC_VREF / ADC_MAX_COUNT;

    SYS_CONSOLE_PRINT("ADC Count = 0x%03x, ADC Input Voltage = %d.%02d V \r", adc_count, (int)input_voltage, (int)((input_voltage - (int)input_voltage)*100.0));  
#endif    
    //TMR2_CallbackRegister(TIMER2_InterruptSvcRoutine, (uintptr_t)NULL);
    //TMR2_PeriodSet(2000 * 99998);
    //TMR2_Start();
    
    while ( true )
    {
        //SYS_CONSOLE_MESSAGE("log1\r\n");
        //vTaskStartScheduler();
        /* Maintain state machines of all polled MPLAB Harmony modules. */
        SYS_Tasks ( );
        //SYS_CONSOLE_MESSAGE("log2\r\n");
#if 0
        // REPL loop
        for (;;) {
            SYS_CONSOLE_MESSAGE("log3\r\n");

            if (pyexec_mode_kind == PYEXEC_MODE_RAW_REPL) {
                if (pyexec_raw_repl() != 0) {
                    break;
                }
            } else {
                SYS_CONSOLE_MESSAGE("log4\r\n");
                if (pyexec_friendly_repl() != 0) {
                    break;
                }
            }

 
        }
#endif
        
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}


void gc_collect(void) {
    // TODO possibly need to trace registers
    //gc_dump_info();
    void *dummy;
    printf("[%s] 1\r\n", __func__);
    gc_collect_start();
    //gc_helper_collect_regs_and_stack();
    gc_collect_root(&dummy, ((mp_uint_t)MP_STATE_THREAD(stack_top) - (mp_uint_t)&dummy) / sizeof(mp_uint_t));
    //gc_collect_root(&dummy, ((mp_uint_t)&dummy - (mp_uint_t)MP_STATE_THREAD(stack_top)) / sizeof(mp_uint_t));
    #if MICROPY_PY_THREAD
    mp_thread_gc_others();
    #endif
    gc_collect_end();
#if 0   
    void *dummy;
    SYS_CONSOLE_PRINT("[%s] In\r\n", __func__);
    gc_collect_start();
    // Node: stack is ascending
    gc_collect_root(&dummy, ((mp_uint_t)&dummy - (mp_uint_t)MP_STATE_THREAD(stack_top)) / sizeof(mp_uint_t));
    gc_collect_end();
#endif
}


typedef struct mp_reader_wfi32_t {
    bool close_fd;
    int fd;
    size_t len;
    size_t pos;
    byte buf[20];
} mp_reader_wfi32_t;


STATIC mp_uint_t mp_reader_wfi32_readbyte(void *data) {
    mp_reader_wfi32_t *reader = (mp_reader_wfi32_t *)data;
    if (reader->pos >= reader->len) {
        if (reader->len == 0) {
            return MP_READER_EOF;
        } else {
            MP_THREAD_GIL_EXIT();
            //int n = read(reader->fd, reader->buf, sizeof(reader->buf));
            int n = SYS_FS_FileRead(reader->fd, reader->buf, sizeof(reader->buf));
            MP_THREAD_GIL_ENTER();
            if (n <= 0) {
                reader->len = 0;
                return MP_READER_EOF;
            }
            reader->len = n;
            reader->pos = 0;
        }
    }
    return reader->buf[reader->pos++];
}

STATIC void mp_reader_wfi32_close(void *data) {
    mp_reader_wfi32_t *reader = (mp_reader_wfi32_t *)data;
    if (reader->close_fd) {
        MP_THREAD_GIL_EXIT();
        SYS_FS_FileClose(reader->fd);
        //close(reader->fd);
        MP_THREAD_GIL_ENTER();
    }
    m_del_obj(mp_reader_posix_t, reader);
}

mp_lexer_t *mp_lexer_new_from_file(const char *filename) {
    //mp_raise_OSError(MP_ENOENT);
    
    mp_reader_t reader;
    SYS_FS_HANDLE fd = (SYS_FS_HANDLE)NULL;
    size_t n;
#ifdef DEV_DEBUG
    SYS_CONSOLE_MESSAGE("mp_lexer_new_from_file\r\n");
#endif
    //mp_reader_new_file(&reader, filename);
    
    MP_THREAD_GIL_EXIT();
    //int fd = open(filename, O_RDONLY, 0644);
    fd = SYS_FS_FileOpen(filename, SYS_FS_FILE_OPEN_READ);
    MP_THREAD_GIL_ENTER();
    if (fd < 0) {
        SYS_CONSOLE_PRINT("No script files .. ");
        mp_raise_OSError(MP_EPERM);
    }
    //mp_reader_new_file_from_fd(reader, fd, true);
    
    mp_reader_wfi32_t *rp = m_new_obj(mp_reader_wfi32_t);
    rp->close_fd = true;
    rp->fd = fd;
    MP_THREAD_GIL_EXIT();
    //int n = read(fd, rp->buf, sizeof(rp->buf));
    n = SYS_FS_FileRead(fd, rp->buf, sizeof(rp->buf));
    if (n == -1) {
        if (rp->close_fd) {
            //close(fd);
            SYS_FS_FileClose(fd);
        }
        MP_THREAD_GIL_ENTER();
        SYS_CONSOLE_PRINT("No script files.. ");
        mp_raise_OSError(MP_EPERM);
    }
    MP_THREAD_GIL_ENTER();
    rp->len = n;
    rp->pos = 0;
    reader.data = rp;
    reader.readbyte = mp_reader_wfi32_readbyte;
    reader.close = mp_reader_wfi32_close;
    
    
    
    return mp_lexer_new(qstr_from_str(filename), reader);
}

mp_import_stat_t mp_import_stat(const char *path) {
    //SYS_CONSOLE_PRINT("mp_import_stat, path= %s\r\n", path);
    
    SYS_FS_HANDLE dirHandle;
    SYS_FS_FSTAT stat;
    char longFileName[300];
    char dir_path[30] = {0};
    bool second_lv = false;

    int i = 0;
    //SYS_CONSOLE_PRINT("path len= %d\r\n", strlen(path));
    for (i = 0; i<strlen(path); i++) {
        if (path[i] == '/')
        {
            memcpy(dir_path, path, i);
            second_lv = true;
            //SYS_CONSOLE_PRINT("i= %d\r\n", i);
            path += i + 1;
           
            break;
        }
    }
    
    // If long file name is used, the following elements of the "stat"
    // structure needs to be initialized with address of proper buffer.
    stat.lfname = longFileName;
    stat.lfsize = 300;
    
    dirHandle = SYS_FS_DirOpen("");

    if(dirHandle == SYS_FS_HANDLE_INVALID)
    {
        SYS_CONSOLE_PRINT("directory open fail\r\n");
        return MP_IMPORT_STAT_NO_EXIST;
    }

    if(SYS_FS_DirSearch(dirHandle, path, SYS_FS_ATTR_DIR, &stat) == SYS_FS_RES_FAILURE)
    {
       SYS_FS_DirClose(dirHandle);
    }
    else
    {
       SYS_FS_DirClose(dirHandle);
       return MP_IMPORT_STAT_DIR;
    }
    
    if (second_lv)
    {
        dirHandle = SYS_FS_DirOpen(dir_path);
        if(dirHandle == SYS_FS_HANDLE_INVALID)
        {
            SYS_CONSOLE_PRINT("directory open fail\r\n");
            return MP_IMPORT_STAT_NO_EXIST;
        }


        if(SYS_FS_DirSearch(dirHandle, path, SYS_FS_ATTR_FILE, &stat) == SYS_FS_RES_FAILURE)
        {
           SYS_FS_DirClose(dirHandle);
           //SYS_CONSOLE_PRINT("directory open log3\r\n");
           return MP_IMPORT_STAT_NO_EXIST;
        }
        else
        {
           SYS_FS_DirClose(dirHandle);
           //SYS_CONSOLE_PRINT("directory open log4\r\n");
           return MP_IMPORT_STAT_FILE;
        }
        SYS_FS_DirClose(dirHandle);
    }
    return MP_IMPORT_STAT_NO_EXIST;
}

mp_obj_t mp_builtin_open(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(mp_builtin_open_obj, 1, mp_builtin_open);

void nlr_jump_fail(void *val) {
    while (1) {
        ;
    }
}

void NORETURN __fatal_error(const char *msg) {
    while (1) {
        ;
    }
}
/*******************************************************************************
 End of File
*/

