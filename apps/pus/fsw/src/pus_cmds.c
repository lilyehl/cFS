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
 *   This file contains the source code for the PUS app Ground Command-handling functions
 */

/*
** Include Files:
*/
#include "pus.h"
#include "pus_cmds.h"
#include "pus_msgids.h"
#include "pus_eventids.h"
#include "pus_version.h"
#include "pus_tbl.h"
#include "pus_utils.h"
#include "pus_msg.h"

/* The sample_lib module provides the SAMPLE_Function() prototype */
#include "sample_lib.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function is triggered in response to a task telemetry request */
/*         from the housekeeping task. This function will gather the Apps     */
/*         telemetry, packetize it and send it to the housekeeping task via   */
/*         the software bus                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t PUS_SendHkCmd(const PUS_SendHkCmd_t *Msg)
{
    int i;

    /*
    ** Get command execution counters...
    */
    PUS_Data.HkTlm.Payload.CommandErrorCounter = PUS_Data.CommandErrorCounter;
    PUS_Data.HkTlm.Payload.CommandCounter      = PUS_Data.CommandCounter;

    /*
    ** Send housekeeping telemetry packet...
    */
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(PUS_Data.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(PUS_Data.HkTlm.TelemetryHeader), true);

    /*
    ** Manage any pending table loads, validations, etc.
    */
    for (i = 0; i < PUS_PLATFORM_NUMBER_OF_TABLES; i++)
    {
        CFE_TBL_Manage(PUS_Data.TblHandles[i]);
    }

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PUS NOOP commands                                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PUS_NoopCmd(const PUS_NoopCmd_t *Msg)
{
    PUS_Data.CommandCounter++;

    CFE_EVS_SendEvent(PUS_NOOP_INF_EID,
                      CFE_EVS_EventType_INFORMATION,
                      "PUS: NOOP command %s",
                      PUS_VERSION);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function resets all the global counter variables that are     */
/*         part of the task telemetry.                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t PUS_ResetCountersCmd(const PUS_ResetCountersCmd_t *Msg)
{
    PUS_Data.CommandCounter      = 0;
    PUS_Data.CommandErrorCounter = 0;

    CFE_EVS_SendEvent(PUS_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "PUS: RESET command");

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function Process Ground Station Command                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t PUS_ProcessCmd(const PUS_ProcessCmd_t *Msg)
{
    CFE_Status_t               Status;
    void                      *TblAddr;
    PUS_ExampleTable_t *TblPtr;
    const char                *TableName = "PUS.ExampleTable";

    /* Sample Use of Example Table */
    PUS_Data.CommandCounter++;
    Status = CFE_TBL_GetAddress(&TblAddr, PUS_Data.TblHandles[0]);
    if (Status < CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("PUS app: Fail to get table address: 0x%08lx", (unsigned long)Status);
    }
    else
    {
        TblPtr = TblAddr;
        CFE_ES_WriteToSysLog("PUS app: Example Table Value 1: %d  Value 2: %d", TblPtr->Int1, TblPtr->Int2);

        PUS_GetCrc(TableName);

        Status = CFE_TBL_ReleaseAddress(PUS_Data.TblHandles[0]);
        if (Status != CFE_SUCCESS)
        {
            CFE_ES_WriteToSysLog("PUS app: Fail to release table address: 0x%08lx", (unsigned long)Status);
        }
        else
        {
            /* Invoke a function provided by PUS_LIB */
            SAMPLE_LIB_Function();
        }
    }

    return Status;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* A simple example command that displays a passed-in value                   */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PUS_DisplayParamCmd(const PUS_DisplayParamCmd_t *Msg)
{
    PUS_Data.CommandCounter++;
    CFE_EVS_SendEvent(PUS_VALUE_INF_EID,
                      CFE_EVS_EventType_INFORMATION,
                      "PUS: ValU32=%lu, ValI16=%d, ValStr=%s",
                      (unsigned long)Msg->Payload.ValU32,
                      (int)Msg->Payload.ValI16,
                      Msg->Payload.ValStr);

    return CFE_SUCCESS;
}
