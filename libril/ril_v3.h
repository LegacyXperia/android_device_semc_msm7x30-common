/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define RIL_VERSION_V3 6

#define RIL_MAX_NETWORKS    2     /* Maximum number of voice, data networks possible */

typedef struct {
    int numElements;                    // No. of elements in following array
    char **regState;                    // Registration states for voice or data
} RIL_RegistrationStateRecord_v3;

typedef struct {
    RIL_RegistrationStateRecord_v3 records[RIL_MAX_NETWORKS];
} RIL_RegistrationStates_v3;

typedef struct {
    int              slot;       /* 0, 1, ... etc. */
    char             *aidPtr;    /* null terminated string, e.g., from 0xA0, 0x00
                                    0x41, 0x30*/
    char             *pin;
} RIL_SimPin_v3;

typedef struct {
    int              slot;       /* 0, 1, ... etc. */
    char             *aidPtr;    /* null terminated string, e.g., from 0xA0, 0x00
                                    0x41, 0x30*/
    char             *puk;
    char             *newPin;
} RIL_SimPuk_v3;

typedef struct {
    int              slot;       /* 0, 1, ... etc. */
    char             *aidPtr;    /* null terminated string, e.g., from 0xA0, 0x00
                                    0x41, 0x30*/
    char             *pin;
    char             *newPin;
} RIL_SimPinSet_v3;

typedef struct {
    int slot;       /* 0, 1, ... etc. */
    char *aidPtr;   /* null terminated string, e.g., from 0xA0, 0x00
                       0x41, 0x30. null for card files (outside the ADF)*/
    int command;    /* one of the commands listed for TS 27.007 +CRSM*/
    int fileid;     /* EF id */
    char *path;     /* "pathid" from TS 27.007 +CRSM command.
                       Path is in hex asciii format eg "7f205f70"
                       Path must always be provided.
                     */
    int p1;
    int p2;
    int p3;
    char *data;     /* May be NULL*/
    char *pin2;     /* May be NULL*/
} RIL_SIM_IO_v3;

#define RIL_CARD_MAX_APPS     8
#define RIL_MAX_CARDS         1

typedef struct
{
  RIL_CardState card_state;
  RIL_PinState  universal_pin_state;             /* applicable to USIM and CSIM: RIL_PINSTATE_xxx */
  int           num_current_3gpp_indexes;
  int           subscription_3gpp_app_index[RIL_CARD_MAX_APPS]; /* value < RIL_CARD_MAX_APPS */
  int           num_current_3gpp2_indexes;
  int           subscription_3gpp2_app_index[RIL_CARD_MAX_APPS]; /* value < RIL_CARD_MAX_APPS */
  int           num_applications;                /* value <= RIL_CARD_MAX_APPS */
  RIL_AppStatus applications[RIL_CARD_MAX_APPS];
} RIL_CardStatus_v3;

typedef struct
{
    int num_cards;
    RIL_CardStatus_v3 card[RIL_MAX_CARDS];
} RIL_CardList_v3;

typedef struct
{
    int slot;       /* 0, 1, ... etc. */
    char *aid_ptr;  /* null terminated string, e.g., from 0xA0, 0x00
                       0x41, 0x30*/
} RIL_RequestImsi_v3;

/**
 * RIL_REQUEST_VOICE_RADIO_TECH
 *
 * Query the radio technology type (3GPP/3GPP2) used for voice. Query is valid only
 * when radio state is RADIO_STATE_ON
 *
 * "data" is NULL
 * "response" is int *
 * ((int *) response)[0] is of type const RIL_RadioTechnologyFamily
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 */
#define RIL_REQUEST_VOICE_RADIO_TECH_V3 104

/**
 * RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE
 *
 * Request to query the location where the CDMA subscription shall
 * be retrieved
 *
 * "data" is NULL
 *
 * "response" is int *
 * ((int *)data)[0] is == 0 from RUIM/SIM (default)
 * ((int *)data)[0] is == 1 from NV
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 *  SUBSCRIPTION_NOT_AVAILABLE
 *
 * See also: RIL_REQUEST_CDMA_SET_SUBSCRIPTION_SOURCE
 */
#define RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE_V3 105

/**
 * RIL_REQUEST_CDMA_PRL_VERSION
 *
 * Request the PRL (preferred roaming list) version.
 *
 * "response" is const char *
 * (const char *)response is PRL version if PRL is loaded and NULL if not
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 *
 * See also: RIL_UNSOL_CDMA_PRL_CHANGED
 */
#define RIL_REQUEST_CDMA_PRL_VERSION_V3 106

/**
 * RIL_REQUEST_IMS_REGISTRATION_STATE
 *
 * Request current IMS registration state
 *
 * "data" is NULL
 *
 * "response" is int *
 * ((int *)response)[0] is == 0 for IMS not registered
 * ((int *)response)[0] is == 1 for IMS registered
 *
 * If ((int*)response)[0] is = 1, then ((int *) response)[1]
 * must follow with IMS SMS encoding:
 *
 * ((int *) response)[1] is of type const RIL_RadioTechnologyFamily
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 */
#define RIL_REQUEST_IMS_REGISTRATION_STATE_V3 107

/**
 * RIL_REQUEST_IMS_SEND_SMS
 *
 * Send a SMS message over IMS
 *
 * "data" is const RIL_IMS_SMS_Message *
 *
 * "response" is a const RIL_SMS_Response *
 *
 * Based on the return error, caller decides to resend if sending sms
 * fails.
 * SUCCESS is error class 0 (no error)
 * SMS_SEND_FAIL_RETRY will cause re-send using RIL_REQUEST_CDMA_SEND_SMS
 *   or RIL_REQUEST_SEND_SMS based on Voice Technology available.
 * and GENERIC_FAILURE means no retry.
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  SMS_SEND_FAIL_RETRY
 *  FDN_CHECK_FAILURE
 *  GENERIC_FAILURE
 *
 */
#define RIL_REQUEST_IMS_SEND_SMS_V3 108

/**
 * RIL_UNSOL_VOICE_RADIO_TECH_CHANGED
 *
 * Indicates that voice technology has changed.
 * Callee will invoke the following requests on main thread: RIL_REQUEST_VOICE_RADIO_TECH
 *
 */
#define RIL_UNSOL_VOICE_RADIO_TECH_CHANGED_V3 1031

/**
 * RIL_UNSOL_RESPONSE_TETHERED_MODE_STATE_CHANGED
 *
 * Called when tethered mode is enabled or disabled
 *
 *
 * "data" is an int 0 - tethered mode off, 1 - tethered mode on
 *
 */
#define RIL_UNSOL_RESPONSE_TETHERED_MODE_STATE_CHANGED_V3 1032

/**
 * RIL_UNSOL_RESPONSE_DATA_NETWORK_STATE_CHANGED
 *
 * Called when data network states has changed
  *
 * Callee will invoke the following requests on main thread:
 *
 * RIL_REQUEST_DATA_REGISTRATION_STATE
  *
 * "data" is NULL
 *
 */
#define RIL_UNSOL_RESPONSE_DATA_NETWORK_STATE_CHANGED_V3 1033

/**
 * RIL_UNSOL_CDMA_SUBSCRIPTION_SOURCE_CHANGED
 *
 * Called when CDMA subscription source changes.
 *
 * Callee will invoke the following request on the main thread:
 *
 * RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE
 *
 * "data" is NULL
 */
#define RIL_UNSOL_CDMA_SUBSCRIPTION_SOURCE_CHANGED_V3 1034

/**
 * RIL_UNSOL_CDMA_PRL_CHANGED
 *
 * Called when PRL (preferred roaming list) changes.
 *
 * Callee will invoke the following request on the main thread:
 *
 * RIL_REQUEST_CDMA_PRL_VERSION
 *
 * "data" is NULL
 */
#define RIL_UNSOL_CDMA_PRL_CHANGED_V3 1035

/**
 * RIL_UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED
 *
 * Called when data network states has changed
 *
 * Callee will invoke the following requests on main thread:
 *
 * RIL_REQUEST_IMS_REGISTRATION_STATE
  *
 * "data" is NULL
 *
 */
#define RIL_UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED_V3 1036

/**
 * RIL_UNSOL_EXIT_EMERGENCY_CALLBACK_MODE
 *
 * Called when data network states has changed
 *
 * Indicates that the radio system selection module has
 * exited emergency callback mode.
 *
 * "data" is NULL
 *
 */
#define RIL_UNSOL_EXIT_EMERGENCY_CALLBACK_MODE_V3 1037

/**
 * RIL_UNSOL_RIL_CONNECTED
 *
 * Called the ril connects and returns the version
 *
 * "data" is int *
 * ((int *)data)[0] is RIL_VERSION
 */
/* Virtual code to match latest stack. */
#define RIL_UNSOL_RIL_CONNECTED_V3 1040
