#include "includes.h"
#include <stdio.h>
/*
*********************************************************************************************************
*                                              CONSTANTS
*********************************************************************************************************
*/
#define  TASK_STK_SIZE   512
#define  START_NUM       13         /* last two digits of B1229013 */

/*
*********************************************************************************************************
*                                              VARIABLES
*********************************************************************************************************
*/
OS_STK    TaskStartStk[TASK_STK_SIZE];

/*
*********************************************************************************************************
*                                          FUNCTION PROTOTYPES
*********************************************************************************************************
*/
void  TaskStart(void *data);

/*
*********************************************************************************************************
*                                                MAIN
*********************************************************************************************************
*/
void  main (void)
{
    PC_DispClrScr(DISP_FGND_WHITE + DISP_BGND_BLACK);
    OSInit();
    PC_DOSSaveReturn();
    PC_VectSet(uCOS, OSCtxSw);
    OSTaskCreate(TaskStart,
                 (void *)0,
                 (void *)&TaskStartStk[TASK_STK_SIZE - 1],
                 0);
    OSStart();
}

/*
*********************************************************************************************************
*                                              STARTUP TASK
*********************************************************************************************************
*/
void  TaskStart (void *pdata)
{
    INT16S key;
    INT16S n;
    INT8U  pos_x;
    char   numbuf[8];

    pdata = pdata;                              /* prevent compiler warning */

    OS_ENTER_CRITICAL();
    PC_VectSet(0x08, OSTickISR);                /* install timer ISR    */
    PC_SetTickRate(OS_TICKS_PER_SEC);           /* 200 ticks per second */
    OS_EXIT_CRITICAL();

    OSStatInit();                               /* statistics task      */

    /* ---- Step 1: print the banner ----------------------------------- */
    PC_DispStr(0, 0, "This is OPS homework!",
               DISP_FGND_WHITE + DISP_BGND_BLACK);

    /* ---- Step 2: delay 3 seconds (HMSM form) ------------------------ */
    OSTimeDlyHMSM(0, 0, 3, 0);

    /* ---- Step 3: print STUDENT ID ----------------------------------- */
    PC_DispStr(0, 2, "B1229013",
               DISP_FGND_WHITE + DISP_BGND_BLACK);

    /* ---- Step 4: delay 2 seconds ------------------------------------ */
    OSTimeDlyHMSM(0, 0, 2, 0);

    /* ---- Steps 5,6,7 loop forever ----------------------------------- */
    for (;;) {

        /* Step 5: print 13, 12, ... 0 side by side, 1 sec between each */
        pos_x = 0;
        for (n = START_NUM; n >= 0; n--) {
            sprintf(numbuf, "%d ", n);          /* "13 ", "12 ", ... "0 " */
            PC_DispStr(pos_x, 5, numbuf,
                       DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
            pos_x += (n >= 10) ? 3 : 2;         /* 2-digit takes 3 cols */
            OSTimeDly(200);                     /* 200 ticks = 1 sec    */
        }

        /* Step 6: clear row 5, then delay 3 seconds */
        PC_DispStr(0, 5,
                   "                                                  ",
                   DISP_FGND_WHITE + DISP_BGND_BLACK);
        OSTimeDlyHMSM(0, 0, 3, 0);

        /* Step 7: implicit -- the for(;;) jumps back to Step 5 */

        /* allow ESC to quit at any tick */
        if (PC_GetKey(&key) == TRUE) {
            if (key == 0x1B) PC_DOSReturn();
        }
    }
}
