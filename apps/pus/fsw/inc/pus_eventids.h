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
 * @file
 *
 * Define PUS app Events IDs
 */

#ifndef PUS_EVENTS_H
#define PUS_EVENTS_H

#define PUS_RESERVED_EID      0
#define PUS_INIT_INF_EID      1
#define PUS_CC_ERR_EID        2
#define PUS_NOOP_INF_EID      3
#define PUS_RESET_INF_EID     4
#define PUS_MID_ERR_EID       5
#define PUS_CMD_LEN_ERR_EID   6
#define PUS_PIPE_ERR_EID      7
#define PUS_VALUE_INF_EID     8
#define PUS_CR_PIPE_ERR_EID   9
#define PUS_SUB_HK_ERR_EID    10
#define PUS_SUB_CMD_ERR_EID   11
#define PUS_TABLE_REG_ERR_EID 12

#define PUS_TC_17_1_INF_EID 13

#endif /* PUS_EVENTS_H */
