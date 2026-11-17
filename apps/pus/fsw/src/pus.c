/************************************************************************
 * NASA Docket No. GSC-19,200-1, and identified as "cFS Draco"
 *
 * Copyright (c) 2023 United States Government as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ************************************************************************/

/**
 * \file
 *   This file contains the source code for the PUS app.
 */

/*
** Include Files:
*/
#include "pus.h"
#include "pus_cmds.h"
#include "pus_utils.h"
#include "pus_eventids.h"
#include "pus_dispatch.h"
#include "pus_tbl.h"
#include "pus_version.h"

/*
** global data
*/
PUS_Data_t PUS_Data;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
/*                                                                            */
/* Application entry point and main process loop                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void PUS_Main(void)
{
    CFE_Status_t     status;
    CFE_SB_Buffer_t *SBBufPtr;

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(PUS_PERF_ID);

    /*
    ** Perform application-specific initialization
    ** If the Initialization fails, set the RunStatus to
    ** CFE_ES_RunStatus_APP_ERROR and the App will not enter the RunLoop
    */
    status = PUS_Init();
    if (status != CFE_SUCCESS)
    {
        PUS_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /*
    ** PUS app Runloop
    */
    while (CFE_ES_RunLoop(&PUS_Data.RunStatus) == true)
    {
        /*
        ** Performance Log Exit Stamp
        */
        CFE_ES_PerfLogExit(PUS_PERF_ID);

        /* Pend on receipt of command packet */
        status = CFE_SB_ReceiveBuffer(&SBBufPtr, PUS_Data.CommandPipe, CFE_SB_PEND_FOREVER);

        /*
        ** Performance Log Entry Stamp
        */
        CFE_ES_PerfLogEntry(PUS_PERF_ID);

        if (status == CFE_SUCCESS)
        {
            PUS_TaskPipe(SBBufPtr);
        }
        else
        {
            CFE_EVS_SendEvent(PUS_PIPE_ERR_EID,
                              CFE_EVS_EventType_ERROR,
                              "PUS: SB Pipe Read Error, App Will Exit");

            PUS_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    /*
    ** Performance Log Exit Stamp
    */
    CFE_ES_PerfLogExit(PUS_PERF_ID);

    CFE_ES_ExitApp(PUS_Data.RunStatus);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* Initialization                                                             */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PUS_Init(void)
{
    CFE_Status_t status;
    char         VersionString[PUS_CFG_MAX_VERSION_STR_LEN];

    /* Zero out the global data structure */
    memset(&PUS_Data, 0, sizeof(PUS_Data));

    PUS_Data.RunStatus = CFE_ES_RunStatus_APP_RUN;

    /*
    ** Register the events
    */
    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("PUS app: Error Registering Events, RC = 0x%08lX\n", (unsigned long)status);
    }
    else
    {
        /*
         ** Initialize housekeeping packet (clear user data area).
         */
        CFE_MSG_Init(CFE_MSG_PTR(PUS_Data.HkTlm.TelemetryHeader),
                     CFE_SB_ValueToMsgId(PUS_HK_TLM_MID),
                     sizeof(PUS_Data.HkTlm));

        /*
         ** Create Software Bus message pipe.
         */
        status = CFE_SB_CreatePipe(&PUS_Data.CommandPipe,
                                   PUS_PLATFORM_PIPE_DEPTH,
                                   PUS_PLATFORM_PIPE_NAME);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(PUS_CR_PIPE_ERR_EID,
                              CFE_EVS_EventType_ERROR,
                              "PUS app: Error creating SB Command Pipe, RC = 0x%08lX",
                              (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to Housekeeping request commands
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(PUS_SEND_HK_MID), PUS_Data.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(PUS_SUB_HK_ERR_EID,
                              CFE_EVS_EventType_ERROR,
                              "PUS app: Error Subscribing to HK request, RC = 0x%08lX",
                              (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to ground command packets
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(PUS_CMD_MID), PUS_Data.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(PUS_SUB_CMD_ERR_EID,
                              CFE_EVS_EventType_ERROR,
                              "PUS app: Error Subscribing to Commands, RC = 0x%08lX",
                              (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Register Example Table(s)
        */
        status = CFE_TBL_Register(&PUS_Data.TblHandles[0],
                                  "ExampleTable",
                                  sizeof(PUS_ExampleTable_t),
                                  CFE_TBL_OPT_DEFAULT,
                                  PUS_TblValidationFunc);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(PUS_TABLE_REG_ERR_EID,
                              CFE_EVS_EventType_ERROR,
                              "PUS app: Error Registering Example Table, RC = 0x%08lX",
                              (unsigned long)status);
        }
        else
        {
            status = CFE_TBL_Load(PUS_Data.TblHandles[0], CFE_TBL_SRC_FILE, PUS_PLATFORM_TABLE_FILE);
        }

        CFE_Config_GetVersionString(VersionString,
                                    PUS_CFG_MAX_VERSION_STR_LEN,
                                    "PUS app",
                                    PUS_VERSION,
                                    PUS_BUILD_CODENAME,
                                    PUS_LAST_OFFICIAL);

        CFE_EVS_SendEvent(PUS_INIT_INF_EID,
                          CFE_EVS_EventType_INFORMATION,
                          "PUS app Initialized.%s",
                          VersionString);
    }

    return status;
}
