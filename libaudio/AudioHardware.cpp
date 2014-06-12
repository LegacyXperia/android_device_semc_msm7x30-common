/*
** Copyright 2008, The Android Open-Source Project
** Copyright (c) 2010-2013, The Linux Foundation. All rights reserved.
** Not a Contribution, Apache license notifications and license are retained
** for attribution purposes only.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#define LOG_TAG "AudioHardware7x30"
//#define LOG_NDDEBUG 0

#include <dlfcn.h>
#include <fcntl.h>
#include <math.h>
#include <media/AudioSystem.h>
#include <cutils/properties.h>

extern "C" {
#include "control.h"
#ifdef WITH_QCOM_CALIBRATION
#include "initialize_audcal7x30.h"
#endif
}

// hardware specific functions

#include "AudioHardware.h"
#include <linux/msm_audio_mvs.h>

#define ECHO_SUPRESSION "ec_supported"

#define DUALMIC_KEY "dualmic_enabled"
#define TTY_MODE_KEY "tty_mode"
#define BTHEADSET_VGS "bt_headset_vgs"
#define AMRNB_DEVICE_IN "/dev/msm_amrnb_in"
#define EVRC_DEVICE_IN "/dev/msm_evrc_in"
#define QCELP_DEVICE_IN "/dev/msm_qcelp_in"
#define AAC_DEVICE_IN "/dev/msm_aac_in"
#define FM_DEVICE  "/dev/msm_fm"
#define MVS_DEVICE "/dev/msm_mvs"

#define FM_A2DP_REC 1
#define FM_FILE_REC 2

#define AMRNB_FRAME_SIZE 32
#define EVRC_FRAME_SIZE 23
#define QCELP_FRAME_SIZE 35



namespace android_audio_legacy {

Mutex   mDeviceSwitchLock;
static int audpre_index, tx_iir_index;
static void * acoustic;
const uint32_t AudioHardware::inputSamplingRates[] = {
        8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000
};
static uint32_t INVALID_DEVICE = 65535;
static uint32_t SND_DEVICE_CURRENT =-1;
static uint32_t SND_DEVICE_HANDSET = 0;
static uint32_t SND_DEVICE_SPEAKER = 1;
static uint32_t SND_DEVICE_HEADSET = 2;
static uint32_t SND_DEVICE_FM_HANDSET = 3;
static uint32_t SND_DEVICE_FM_SPEAKER = 4;
static uint32_t SND_DEVICE_FM_HEADSET = 5;
static uint32_t SND_DEVICE_BT = 6;
static uint32_t SND_DEVICE_BT_EC_OFF =-1;
static uint32_t SND_DEVICE_HEADSET_AND_SPEAKER = 7;
static uint32_t SND_DEVICE_NO_MIC_HEADSET = 8;
static uint32_t SND_DEVICE_IN_S_SADC_OUT_HANDSET = 9;
static uint32_t SND_DEVICE_IN_S_SADC_OUT_SPEAKER_PHONE = 10;
static uint32_t SND_DEVICE_TTY_HEADSET = 11;
static uint32_t SND_DEVICE_TTY_HCO = 12;
static uint32_t SND_DEVICE_TTY_VCO = 13;
static uint32_t SND_DEVICE_TTY_FULL = 14;
static uint32_t SND_DEVICE_CARKIT = -1;
static uint32_t SND_DEVICE_HDMI = 15;
static uint32_t SND_DEVICE_FM_TX = 16;
static uint32_t SND_DEVICE_FM_TX_AND_SPEAKER = 17;
static uint32_t SND_DEVICE_HEADPHONE_AND_SPEAKER = 18;
#ifdef SAMSUNG_AUDIO
static uint32_t SND_DEVICE_VOIP_HANDSET = 50;
static uint32_t SND_DEVICE_VOIP_SPEAKER = 51;
static uint32_t SND_DEVICE_VOIP_HEADSET = 52;
static uint32_t SND_DEVICE_CALL_HANDSET = 60;
static uint32_t SND_DEVICE_CALL_SPEAKER = 61;
static uint32_t SND_DEVICE_CALL_HEADSET = 62;
#endif
#ifdef SEMC_AUDIO
static uint32_t SND_DEVICE_SPEAKER_DUAL_MIC_REAL_STEREO = 19;
#endif

static uint32_t DEVICE_HANDSET_RX = 0;           /* handset_rx */
static uint32_t DEVICE_HANDSET_TX = 1;           /* handset_tx */
static uint32_t DEVICE_SPEAKER_RX = 2;           /* speaker_stereo_rx
                                                    sam: speaker_rx */
static uint32_t DEVICE_SPEAKER_TX = 3;           /* speaker_mono_tx
                                                    sam: speaker_tx */
static uint32_t DEVICE_HEADSET_RX = 4;           /* headset_stereo_rx
                                                    sam: headset_rx */
static uint32_t DEVICE_HEADSET_TX = 5;           /* headset_mono_tx
                                                    sam: headset_tx */
static uint32_t DEVICE_FMRADIO_HANDSET_RX = 6;   /* fmradio_handset_rx */
static uint32_t DEVICE_FMRADIO_HEADSET_RX = 7;   /* fmradio_headset_rx */
static uint32_t DEVICE_FMRADIO_SPEAKER_RX = 8;   /* fmradio_speaker_rx */
static uint32_t DEVICE_DUALMIC_HANDSET_TX = 9;   /* handset_dual_mic_endfire_tx */
static uint32_t DEVICE_DUALMIC_SPEAKER_TX = 10;  /* speaker_dual_mic_endfire_tx */
static uint32_t DEVICE_TTY_HEADSET_MONO_RX = 11; /* tty_headset_mono_rx */
static uint32_t DEVICE_TTY_HEADSET_MONO_TX = 12; /* tty_headset_mono_tx */
static uint32_t DEVICE_SPEAKER_HEADSET_RX = 13;  /* headset_stereo_speaker_stereo_rx
                                                    sam: speaker_headset_rx */
static uint32_t DEVICE_FMRADIO_STEREO_TX = 14;
static uint32_t DEVICE_HDMI_STERO_RX = 15;       /* hdmi_stereo_rx */
static uint32_t DEVICE_FMRADIO_STEREO_RX = 16;
static uint32_t DEVICE_BT_SCO_RX = 17;           /* bt_sco_rx */
static uint32_t DEVICE_BT_SCO_TX = 18;           /* bt_sco_tx */
#ifdef SAMSUNG_AUDIO
static uint32_t DEVICE_HANDSET_VOIP_RX = 40;     /* handset_voip_rx */
static uint32_t DEVICE_HANDSET_VOIP_TX = 41;     /* handset_voip_tx */
static uint32_t DEVICE_SPEAKER_VOIP_RX = 42;     /* speaker_voip_rx */
static uint32_t DEVICE_SPEAKER_VOIP_TX = 43;     /* speaker_voip_tx */
static uint32_t DEVICE_HEADSET_VOIP_RX = 44;     /* headset_voip_rx */
static uint32_t DEVICE_HEADSET_VOIP_TX = 45;     /* headset_voip_tx */
static uint32_t DEVICE_HANDSET_CALL_RX = 60;     /* handset_call_rx */
static uint32_t DEVICE_HANDSET_CALL_TX = 61;     /* handset_call_tx */
static uint32_t DEVICE_SPEAKER_CALL_RX = 62;     /* speaker_call_rx */
static uint32_t DEVICE_SPEAKER_CALL_TX = 63;     /* speaker_call_tx */
static uint32_t DEVICE_HEADSET_CALL_RX = 64;     /* headset_call_rx */
static uint32_t DEVICE_HEADSET_CALL_TX = 65;     /* headset_call_tx */
static uint32_t DEVICE_COUNT = DEVICE_HEADSET_CALL_TX + 1;
#elif defined(SEMC_AUDIO)
static uint32_t DEVICE_SPEAKER_DUAL_MIC_REAL_STEREO_TX = 19;
static uint32_t DEVICE_COUNT = DEVICE_SPEAKER_DUAL_MIC_REAL_STEREO_TX + 1;
#else
static uint32_t DEVICE_COUNT = DEVICE_BT_SCO_TX + 1;
#endif


int dev_cnt = 0;
const char ** name = NULL;
int mixer_cnt = 0;
static uint32_t cur_tx = INVALID_DEVICE;
static uint32_t cur_rx = INVALID_DEVICE;
bool vMicMute = false;
typedef struct routing_table
{
    unsigned short dec_id;
    int dev_id;
    int dev_id_tx;
    int stream_type;
    bool active;
    struct routing_table *next;
} Routing_table;
Routing_table* head;
Mutex       mRoutingTableLock;

typedef struct device_table
{
    int dev_id;
    int class_id;
    int capability;
}Device_table;
Device_table* device_list;

static unsigned char build_id[20];

static void amr_transcode(unsigned char *src, unsigned char *dst);

enum STREAM_TYPES {
PCM_PLAY=1,
PCM_REC,
LPA_DECODE,
VOICE_CALL,
FM_RADIO,
FM_REC,
FM_A2DP,
INVALID_STREAM
};

typedef struct ComboDeviceType
{
    uint32_t DeviceId;
    STREAM_TYPES StreamType;
}CurrentComboDeviceStruct;
CurrentComboDeviceStruct CurrentComboDeviceData;
Mutex   mComboDeviceLock;

enum FM_STATE {
    FM_INVALID=1,
    FM_OFF,
    FM_ON
};

FM_STATE fmState = FM_INVALID;
static uint32_t fmDevice = INVALID_DEVICE;

#define DEV_ID(X) device_list[X].dev_id
void addToTable(int decoder_id,int device_id,int device_id_tx,int stream_type,bool active) {
    Routing_table* temp_ptr;
    Mutex::Autolock lock(mRoutingTableLock);
    temp_ptr = (Routing_table* ) malloc(sizeof(Routing_table));
    temp_ptr->next = NULL;
    temp_ptr->dec_id = decoder_id;
    temp_ptr->dev_id = device_id;
    temp_ptr->dev_id_tx = device_id_tx;
    temp_ptr->stream_type = stream_type;
    temp_ptr->active = active;
    //add new Node to head.
    temp_ptr->next =head->next;
    head->next = temp_ptr;
}
bool isStreamOn(int Stream_type) {
    Routing_table* temp_ptr;
    Mutex::Autolock lock(mRoutingTableLock);
    temp_ptr = head->next;
    while(temp_ptr!=NULL) {
        if(temp_ptr->stream_type == Stream_type)
                return true;
        temp_ptr=temp_ptr->next;
    }
    return false;
}
bool isStreamOnAndActive(int Stream_type) {
    Routing_table* temp_ptr;
    Mutex::Autolock lock(mRoutingTableLock);
    temp_ptr = head->next;
    while(temp_ptr!=NULL) {
        if(temp_ptr->stream_type == Stream_type) {
            if(temp_ptr->active == true) {
                return true;
            }
            else {
                return false;
            }
        }
        temp_ptr=temp_ptr->next;
    }
    return false;
}
bool isStreamOnAndInactive(int Stream_type) {
    Routing_table* temp_ptr;
    Mutex::Autolock lock(mRoutingTableLock);
    temp_ptr = head->next;
    while(temp_ptr!=NULL) {
        if(temp_ptr->stream_type == Stream_type) {
            if(temp_ptr->active == false) {
                return true;
            }
            else {
                return false;
            }
        }
        temp_ptr=temp_ptr->next;
    }
    return false;
}
Routing_table*  getNodeByStreamType(int Stream_type) {
    Routing_table* temp_ptr;
    Mutex::Autolock lock(mRoutingTableLock);
    temp_ptr = head->next;
    while(temp_ptr!=NULL) {
        if(temp_ptr->stream_type == Stream_type) {
            return temp_ptr;
        }
        temp_ptr=temp_ptr->next;
    }
    return NULL;
}
void modifyActiveStateOfStream(int Stream_type, bool Active) {
    Routing_table* temp_ptr;
    Mutex::Autolock lock(mRoutingTableLock);
    temp_ptr = head->next;
    while(temp_ptr!=NULL) {
        if(temp_ptr->stream_type == Stream_type) {
            temp_ptr->active = Active;
            return;
        }
        temp_ptr=temp_ptr->next;
    }
}
void modifyActiveDeviceOfStream(int Stream_type,int Device_id,int Device_id_tx) {
    Routing_table* temp_ptr;
    temp_ptr = head->next;
    while(temp_ptr!=NULL) {
        if(temp_ptr->stream_type == Stream_type) {
            temp_ptr->dev_id = Device_id;
            temp_ptr->dev_id_tx = Device_id_tx;
            return;
        }
        temp_ptr=temp_ptr->next;
    }
}
void printTable()
{
    Routing_table * temp_ptr;
    Mutex::Autolock lock(mRoutingTableLock);
    temp_ptr = head->next;
    while(temp_ptr!=NULL) {
        printf("%d %d %d %d %d\n",temp_ptr->dec_id,temp_ptr->dev_id,temp_ptr->dev_id_tx,temp_ptr->stream_type,temp_ptr->active);
        temp_ptr = temp_ptr->next;
    }
}
void deleteFromTable(int Stream_type) {
    Routing_table *temp_ptr,*temp1;
    Mutex::Autolock lock(mRoutingTableLock);
    temp_ptr = head;
    while(temp_ptr->next!=NULL) {
        if(temp_ptr->next->stream_type == Stream_type) {
            temp1 = temp_ptr->next;
            temp_ptr->next = temp_ptr->next->next;
            free(temp1);
            return;
        }
        temp_ptr=temp_ptr->next;
    }

}

bool isDeviceListEmpty() {
    if(head->next == NULL)
        return true;
    else
        return false;
}

int enableDevice(int device,short enable) {
    ALOGV("value of device and enable is %d %d ALSA dev id:%d",device,enable,DEV_ID(device));
    if( msm_en_device(DEV_ID(device), enable)) {
        ALOGE("msm_en_device(%d, %d) failed errno = %d",DEV_ID(device), enable, errno);
        return -1;
    }
    return 0;
}

static status_t updateDeviceInfo(int rx_device,int tx_device) {
    ALOGV("updateDeviceInfo: E rx_device %d and tx_device %d", rx_device, tx_device);
    bool isRxDeviceEnabled = false,isTxDeviceEnabled = false;
    Routing_table *temp_ptr,*temp_head;
    int tx_dev_prev = INVALID_DEVICE;
    temp_head = head;
    Mutex::Autolock lock(mDeviceSwitchLock);

    if(!getNodeByStreamType(VOICE_CALL) && !getNodeByStreamType(PCM_PLAY) &&
       !getNodeByStreamType(LPA_DECODE) && !getNodeByStreamType(FM_RADIO)) {
        ALOGV("No active voicecall/playback, disabling cur_rx %d", cur_rx);
        if(cur_rx != INVALID_DEVICE && enableDevice(cur_rx, 0)) {
            ALOGE("Disabling device failed for cur_rx %d", cur_rx);
        }
        cur_rx = rx_device;

    }

    if(!getNodeByStreamType(VOICE_CALL) && !getNodeByStreamType(PCM_REC)) {
        ALOGV("No active voicecall/recording, disabling cur_tx %d", cur_tx);
        if(cur_tx != INVALID_DEVICE && enableDevice(cur_tx, 0)) {
            ALOGE("Disabling device failed for cur_tx %d", cur_tx);
        }
        cur_tx = tx_device;
    }
    Mutex::Autolock lock_1(mRoutingTableLock);

    while(temp_head->next != NULL) {
        temp_ptr = temp_head->next;
        switch(temp_ptr->stream_type) {
            case PCM_PLAY:
            case LPA_DECODE:
            case FM_RADIO:
                ALOGD("The node type is %d and cur device %d new device %d ", temp_ptr->stream_type, temp_ptr->dev_id, rx_device);
                if(rx_device == INVALID_DEVICE)
                    return -1;
                if(rx_device == temp_ptr->dev_id)
                    break;
                ALOGV("rx_device = %d,temp_ptr->dev_id = %d",rx_device,temp_ptr->dev_id);
                if(isRxDeviceEnabled == false) {
                    enableDevice(temp_ptr->dev_id,0);
                    enableDevice(rx_device,1);
                    isRxDeviceEnabled = true;
                }
                if(msm_route_stream(PCM_PLAY,temp_ptr->dec_id,DEV_ID(temp_ptr->dev_id),0)) {
                    ALOGE("msm_route_stream(PCM_PLAY,%d,%d,0) failed",temp_ptr->dec_id,DEV_ID(temp_ptr->dev_id));
                }
                if(msm_route_stream(PCM_PLAY,temp_ptr->dec_id,DEV_ID(rx_device),1)) {
                    ALOGE("msm_route_stream(PCM_PLAY,%d,%d,1) failed",temp_ptr->dec_id,DEV_ID(rx_device));
                }
                modifyActiveDeviceOfStream(temp_ptr->stream_type,rx_device,INVALID_DEVICE);
                cur_tx = tx_device ;
                cur_rx = rx_device ;
                break;
            case PCM_REC:

                ALOGD("case PCM_REC");
                if(tx_device == INVALID_DEVICE)
                    return -1;
                if(tx_device == temp_ptr->dev_id)
                    break;

                if(isTxDeviceEnabled == false) {
                    enableDevice(temp_ptr->dev_id,0);
                    enableDevice(tx_device,1);
                   isTxDeviceEnabled = true;
                }
                if(msm_route_stream(PCM_REC,temp_ptr->dec_id,DEV_ID(temp_ptr->dev_id),0)) {
                    ALOGE("msm_route_stream(PCM_REC,%d,%d,0) failed",temp_ptr->dec_id,DEV_ID(temp_ptr->dev_id));
                }
                if(msm_route_stream(PCM_REC,temp_ptr->dec_id,DEV_ID(tx_device),1)) {
                    ALOGE("msm_route_stream(PCM_REC,%d,%d,1) failed",temp_ptr->dec_id,DEV_ID(tx_device));
                }
                modifyActiveDeviceOfStream(PCM_REC,tx_device,INVALID_DEVICE);
                tx_dev_prev = cur_tx;
                cur_tx = tx_device ;
                cur_rx = rx_device ;
                if((vMicMute == true) && (tx_dev_prev != cur_tx)) {
                    ALOGD("REC:device switch with mute enabled :tx_dev_prev %d cur_tx: %d",tx_dev_prev, cur_tx);
                    msm_device_mute(DEV_ID(cur_tx), true);
                }
                break;
            case VOICE_CALL:

                ALOGD("case VOICE_CALL");
                if(rx_device == INVALID_DEVICE || tx_device == INVALID_DEVICE)
                    return -1;
                if(rx_device == temp_ptr->dev_id && tx_device == temp_ptr->dev_id_tx)
                    break;

                msm_route_voice(DEV_ID(rx_device),DEV_ID(tx_device),1);

                // Temporary work around for Speaker mode. The driver is not
                // supporting Speaker Rx and Handset Tx combo
                if(isRxDeviceEnabled == false) {
                    if (rx_device != temp_ptr->dev_id)
                    {
                        enableDevice(temp_ptr->dev_id,0);
                    }
                    isRxDeviceEnabled = true;
                }
                if(isTxDeviceEnabled == false) {
                    if (tx_device != temp_ptr->dev_id_tx)
                    {
                        enableDevice(temp_ptr->dev_id_tx,0);
                    }
                    isTxDeviceEnabled = true;
                }

                if (rx_device != temp_ptr->dev_id)
                {
                    enableDevice(rx_device,1);
                }

                if (tx_device != temp_ptr->dev_id_tx)
                {
                    enableDevice(tx_device,1);
                }

                cur_rx = rx_device;
                cur_tx = tx_device;
                modifyActiveDeviceOfStream(VOICE_CALL,cur_rx,cur_tx);
                break;
            default:
                break;
        }
        temp_head = temp_head->next;
    }

    ALOGV("updateDeviceInfo: X cur_rx %d cur_tx %d", cur_rx, cur_tx);
    return NO_ERROR;
}

void freeMemory() {
    Routing_table *temp;
    while(head != NULL) {
        temp = head->next;
        free(head);
        head = temp;
    }
free(device_list);
}

//
// ----------------------------------------------------------------------------

AudioHardware::AudioHardware() :
    mInit(false), mMicMute(true), mBluetoothNrec(true), mBluetoothId(0),
    mOutput(0),mBluetoothVGS(false), mVoiceVolume(1),
    mCurSndDevice(-1),mTtyMode(TTY_OFF), mDualMicEnabled(false), mFmFd(-1),
    mVoipFd(-1), mVoipInActive(false), mVoipOutActive(false), mDirectOutput(0)
{

        int control;
        int i = 0,index = 0;

        head = (Routing_table* ) malloc(sizeof(Routing_table));
        head->next = NULL;

        ALOGD("msm_mixer_open: Opening the device");
        control = msm_mixer_open("/dev/snd/controlC0", 0);
        if(control< 0)
                ALOGE("ERROR opening the device");

        if((fp = fopen("/sys/devices/system/soc/soc0/build_id","r")) == NULL) {
            ALOGE("Cannot open build_id file.");
        }
        else {
            (void)fgets((char *)build_id,sizeof(build_id),fp);
        }

        if(msm_reset_all_device() < 0)
            ALOGE("msm_reset_all_device() failed");

        mixer_cnt = msm_mixer_count();
        ALOGD("msm_mixer_count:mixer_cnt =%d",mixer_cnt);

        dev_cnt = msm_get_device_count();
        ALOGV("got device_count %d",dev_cnt);
        if (dev_cnt <= 0) {
                ALOGE("NO devices registered\n");
                return;
        }
        name = msm_get_device_list();
        device_list = (Device_table* )malloc(sizeof(Device_table)*DEVICE_COUNT);
        if(device_list == NULL) {
            ALOGE("malloc failed for device list");
            return;
        }
        for(i = 0;i<DEVICE_COUNT;i++)
            device_list[i].dev_id = INVALID_DEVICE;

        for(i = 0; i < dev_cnt;i++) {
            if(strcmp((char* )name[i],"handset_rx") == 0)
                index = DEVICE_HANDSET_RX;
            else if(strcmp((char* )name[i],"handset_tx") == 0)
                index = DEVICE_HANDSET_TX;
#ifndef WITH_STEREO_HW_SPEAKER
            else if(strcmp((char* )name[i],"speaker_mono_rx") == 0)
                index = DEVICE_SPEAKER_RX;
#endif
            else if((strcmp((char* )name[i],"speaker_stereo_rx") == 0) ||
                    (strcmp((char* )name[i],"speaker_rx") == 0))
                index = DEVICE_SPEAKER_RX;
            else if((strcmp((char* )name[i],"speaker_mono_tx") == 0) ||
                    (strcmp((char* )name[i],"speaker_tx") == 0))
                index = DEVICE_SPEAKER_TX;
            else if((strcmp((char* )name[i],"headset_stereo_rx") == 0) ||
                    (strcmp((char* )name[i],"headset_rx") == 0))
                index = DEVICE_HEADSET_RX;
            else if((strcmp((char* )name[i],"headset_mono_tx") == 0) ||
                    (strcmp((char* )name[i],"headset_tx") == 0))
                index = DEVICE_HEADSET_TX;
            else if(strcmp((char* )name[i],"fmradio_handset_rx") == 0)
                index = DEVICE_FMRADIO_HANDSET_RX;
            else if(strcmp((char* )name[i],"fmradio_headset_rx") == 0)
                index = DEVICE_FMRADIO_HEADSET_RX;
            else if(strcmp((char* )name[i],"fmradio_speaker_rx") == 0)
                index = DEVICE_FMRADIO_SPEAKER_RX;
            else if(strcmp((char* )name[i],"handset_dual_mic_broadside_tx") == 0)
                index = DEVICE_DUALMIC_HANDSET_TX;
            else if(strcmp((char* )name[i],"handset_dual_mic_endfire_tx") == 0)
                index = DEVICE_DUALMIC_HANDSET_TX;
            else if(strcmp((char* )name[i],"speaker_dual_mic_broadside_tx") == 0)
                index = DEVICE_DUALMIC_SPEAKER_TX;
            else if(strcmp((char* )name[i],"speaker_dual_mic_endfire_tx") == 0)
                index = DEVICE_DUALMIC_SPEAKER_TX;
            else if(strcmp((char* )name[i],"tty_headset_mono_rx") == 0)
                index = DEVICE_TTY_HEADSET_MONO_RX;
            else if(strcmp((char* )name[i],"tty_headset_mono_tx") == 0)
                index = DEVICE_TTY_HEADSET_MONO_TX;
            else if(strcmp((char* )name[i],"bt_sco_rx") == 0)
                index = DEVICE_BT_SCO_RX;
            else if(strcmp((char* )name[i],"bt_sco_tx") == 0)
                index = DEVICE_BT_SCO_TX;
            else if((strcmp((char*)name[i],"headset_stereo_speaker_stereo_rx") == 0) ||
                    (strcmp((char*)name[i],"speaker_headset_rx") == 0))
                index = DEVICE_SPEAKER_HEADSET_RX;
            else if(strcmp((char*)name[i],"fmradio_stereo_tx") == 0)
                index = DEVICE_FMRADIO_STEREO_TX;
            else if(strcmp((char*)name[i],"hdmi_stereo_rx") == 0)
                index = DEVICE_HDMI_STERO_RX;
            else if(strcmp((char*)name[i],"fmradio_stereo_rx") == 0)
                index = DEVICE_FMRADIO_STEREO_RX;
#ifdef SAMSUNG_AUDIO
            else if(strcmp((char* )name[i], "handset_voip_rx") == 0)
                index = DEVICE_HANDSET_VOIP_RX;
            else if(strcmp((char* )name[i], "handset_voip_tx") == 0)
                index = DEVICE_HANDSET_VOIP_TX;
            else if(strcmp((char* )name[i], "speaker_voip_rx") == 0)
                index = DEVICE_SPEAKER_VOIP_RX;
            else if(strcmp((char* )name[i], "speaker_voip_tx") == 0)
                index = DEVICE_SPEAKER_VOIP_TX;
            else if(strcmp((char* )name[i], "headset_voip_rx") == 0)
                index = DEVICE_HEADSET_VOIP_RX;
            else if(strcmp((char* )name[i], "headset_voip_tx") == 0)
                index = DEVICE_HEADSET_VOIP_TX;
            else if(strcmp((char* )name[i], "handset_call_rx") == 0)
                index = DEVICE_HANDSET_CALL_RX;
            else if(strcmp((char* )name[i], "handset_call_tx") == 0)
                index = DEVICE_HANDSET_CALL_TX;
            else if(strcmp((char* )name[i], "speaker_call_rx") == 0)
                index = DEVICE_SPEAKER_CALL_RX;
            else if(strcmp((char* )name[i], "speaker_call_tx") == 0)
                index = DEVICE_SPEAKER_CALL_TX;
            else if(strcmp((char* )name[i], "headset_call_rx") == 0)
                index = DEVICE_HEADSET_CALL_RX;
            else if(strcmp((char* )name[i], "headset_call_tx") == 0)
                index = DEVICE_HEADSET_CALL_TX;
#endif
#ifdef SEMC_AUDIO
            else if(strcmp((char* )name[i], "speaker_dual_mic_endfire_tx_real_stereo") == 0)
                index = DEVICE_SPEAKER_DUAL_MIC_REAL_STEREO_TX;
#endif
            else
                continue;
            ALOGV("index = %d",index);

            device_list[index].dev_id = msm_get_device((char* )name[i]);
            if(device_list[index].dev_id >= 0) {
                    ALOGV("Found device: %s:index = %d,dev_id: %d",( char* )name[i], index,device_list[index].dev_id);
            }
            device_list[index].class_id = msm_get_device_class(device_list[index].dev_id);
            device_list[index].capability = msm_get_device_capability(device_list[index].dev_id);
            ALOGV("class ID = %d,capablity = %d for device %d",device_list[index].class_id,device_list[index].capability,device_list[index].dev_id);
        }
#ifdef WITH_QCOM_CALIBRATION
        audcal_initialize();
#endif
        mInit = true;

        CurrentComboDeviceData.DeviceId = INVALID_DEVICE;
        CurrentComboDeviceData.StreamType = INVALID_STREAM;
}

AudioHardware::~AudioHardware()
{
    for (size_t index = 0; index < mInputs.size(); index++) {
        closeInputStream((AudioStreamIn*)mInputs[index]);
    }
    mInputs.clear();
    closeOutputStream((AudioStreamOut*)mOutput);
    if (acoustic) {
        ::dlclose(acoustic);
        acoustic = 0;
    }
    msm_mixer_close();
#ifdef WITH_QCOM_CALIBRATION
    audcal_deinitialize();
#endif
    freeMemory();
    fclose(fp);
    mInit = false;
}

status_t AudioHardware::initCheck()
{
    return mInit ? NO_ERROR : NO_INIT;
}

AudioStreamOut* AudioHardware::openOutputStream(
        uint32_t devices,  int *format, uint32_t *channels,
        uint32_t *sampleRate, status_t *status)

{
     ALOGD("AudioHardware::openOutputStream devices %x format %d channels %d samplerate %d",
        devices, *format, *channels, *sampleRate);

     audio_output_flags_t flags = static_cast<audio_output_flags_t> (*status);

     if (!audio_is_output_device(devices))
        return 0;


    { // scope for the lock
        Mutex::Autolock lock(mLock);

        // only one output stream allowed
        if (mOutput && !((flags & AUDIO_OUTPUT_FLAG_DIRECT) && (flags & AUDIO_OUTPUT_FLAG_VOIP_RX))
                    && !(flags & AUDIO_OUTPUT_FLAG_LPA)) {
            if (status) {
                *status = INVALID_OPERATION;
            }
            ALOGE(" AudioHardware::openOutputStream Only one output stream allowed \n");
            return 0;
        }
        status_t lStatus;
        if((flags & AUDIO_OUTPUT_FLAG_DIRECT) && (flags & AUDIO_OUTPUT_FLAG_VOIP_RX)){
            if(mDirectOutput == 0) {
                // open direct output stream
                ALOGV(" AudioHardware::openOutputStream Direct output stream \n");
                AudioStreamOutDirect* out = new AudioStreamOutDirect();
                status_t lStatus = out->set(this, devices, format, channels, sampleRate);
                if (status) {
                    *status = lStatus;
                }
                if (lStatus == NO_ERROR) {
                    mDirectOutput = out;
                    ALOGV(" \n set sucessful for AudioStreamOutDirect");
                } else {
                    ALOGE(" \n set Failed for AudioStreamOutDirect");
                    delete out;
                }
            }
            else
                ALOGE(" \n AudioHardware::AudioStreamOutDirect is already open");
            return mDirectOutput;
        }else if (flags & AUDIO_OUTPUT_FLAG_LPA) {
            status_t err = BAD_VALUE;
            // create new output LPA stream
            AudioSessionOutLPA* out = new AudioSessionOutLPA(this, devices, *format, *channels,*sampleRate,0,&err);
            if(err != NO_ERROR) {
                delete out;
                out = NULL;
            }
            if (status) *status = err;
            mOutputLPA = out;
            return mOutputLPA;
        }
        else
        {
            ALOGV(" AudioHardware::openOutputStream AudioStreamOutMSM72xx output stream \n");
            // only one output stream allowed
            if (mOutput) {
                if (status) {
                  *status = INVALID_OPERATION;
                }
                ALOGE(" AudioHardware::openOutputStream Only one output stream allowed \n");
                return 0;
            }

            // create new output stream
            AudioStreamOutMSM72xx* out = new AudioStreamOutMSM72xx();
            lStatus = out->set(this, devices, format, channels, sampleRate);
            if (status) {
                *status = lStatus;
            }
            if (lStatus == NO_ERROR) {
                mOutput = out;
            } else {
                delete out;
            }
            return mOutput;
         }
     }
}

void AudioHardware::closeOutputStream(AudioStreamOut* out) {
    Mutex::Autolock lock(mLock);
    if ((mOutput == 0 && mDirectOutput == 0 && mOutputLPA == 0) ||
        ((mOutput != out) && (mDirectOutput != out) && (mOutputLPA != out))){
        ALOGW("Attempt to close invalid output stream");
    }
    else if (mOutput == out) {
        delete mOutput;
        mOutput = 0;
    }
    else if (mDirectOutput == out){
        ALOGV(" deleting  mDirectOutput \n");
        delete mDirectOutput;
        mDirectOutput = 0;
    }
    else if (mOutputLPA == out) {
        ALOGV("Deleting  mOutputLPA \n");
        delete mOutputLPA;
        mOutputLPA = 0;
    }
}

AudioStreamIn* AudioHardware::openInputStream(
        uint32_t devices, int *format, uint32_t *channels, uint32_t *sampleRate, status_t *status,
        AudioSystem::audio_in_acoustics acoustic_flags)
{
    // check for valid input source
    ALOGD("AudioHardware::openInputStream devices %x format %d channels %d samplerate %d",
        devices, *format, *channels, *sampleRate);

    if (!audio_is_input_device(devices))
        return 0;

    mLock.lock();

    AudioStreamIn *in;
    if((devices == AUDIO_DEVICE_IN_COMMUNICATION)&& (*sampleRate == 8000)) {
        ALOGV("Create Audio stream Voip \n");
        AudioStreamInVoip* inVoip = new AudioStreamInVoip();
        status_t lStatus = NO_ERROR;
        lStatus =  inVoip->set(this, devices, format, channels, sampleRate, acoustic_flags);
        if (status) {
            *status = lStatus;
        }
        if (lStatus != NO_ERROR) {
            ALOGE(" Error creating voip input \n");
            mLock.unlock();
            delete inVoip;
            return 0;
        }
        mVoipInputs.add(inVoip);
        mLock.unlock();
        return inVoip;
    } else {
        AudioStreamInMSM72xx* in72xx = new AudioStreamInMSM72xx();
        status_t lStatus = in72xx->set(this, devices, format, channels, sampleRate, acoustic_flags);
        if (status) {
            *status = lStatus;
        }
        if (lStatus != NO_ERROR) {
            ALOGE("Error creating Audio stream AudioStreamInMSM72xx \n");
            mLock.unlock();
            delete in72xx;
            return 0;
        }
        mInputs.add(in72xx);
        mLock.unlock();
        return in72xx;
    }
}

void AudioHardware::closeInputStream(AudioStreamIn* in) {
    Mutex::Autolock lock(mLock);

    ssize_t index = -1;
    if((index = mInputs.indexOf((AudioStreamInMSM72xx *)in)) >= 0) {
        ALOGV("closeInputStream AudioStreamInMSM72xx");
        mLock.unlock();
        delete mInputs[index];
        mLock.lock();
        mInputs.removeAt(index);
    } else if ((index = mVoipInputs.indexOf((AudioStreamInVoip *)in)) >= 0) {
        ALOGV("closeInputStream mVoipInputs");
        mLock.unlock();
        delete mVoipInputs[index];
        mLock.lock();
        mVoipInputs.removeAt(index);
    } else {
        ALOGE("Attempt to close invalid input stream");
     }
}

status_t AudioHardware::setMode(int mode)
{
    status_t status = AudioHardwareBase::setMode(mode);
    if (status == NO_ERROR) {
        // make sure that doAudioRouteOrMute() is called by doRouting()
        // even if the new device selected is the same as current one.
        clearCurDevice();
    }
    return status;
}

bool AudioHardware::checkOutputStandby()
{
    if (mOutput)
        if (!mOutput->checkStandby())
            return false;

    return true;
}

status_t AudioHardware::setMicMute(bool state)
{
    Mutex::Autolock lock(mLock);
    return setMicMute_nosync(state);
}

// always call with mutex held
status_t AudioHardware::setMicMute_nosync(bool state)
{
    if (mMicMute != state) {
        mMicMute = state;
        ALOGD("setMicMute_nosync calling voice mute with the mMicMute %d", mMicMute);
        if (isStreamOnAndActive(PCM_REC) && (mMode == AUDIO_MODE_IN_COMMUNICATION)) {
            vMicMute = state;
            ALOGD("VOIP Active: vMicMute %d\n", vMicMute);
            msm_device_mute(DEV_ID(cur_tx), vMicMute);
        } else {
            ALOGD("setMicMute_nosync:voice_mute\n");
            msm_set_voice_tx_mute(mMicMute);
        }
    }
    return NO_ERROR;
}

status_t AudioHardware::getMicMute(bool* state)
{
    *state = mMicMute;
    return NO_ERROR;
}

status_t AudioHardware::setParameters(const String8& keyValuePairs)
{
    AudioParameter param = AudioParameter(keyValuePairs);
    String8 value;
    String8 key;
    const char BT_NREC_KEY[] = "bt_headset_nrec";
    const char BT_NAME_KEY[] = "bt_headset_name";
    const char BT_NREC_VALUE_ON[] = "on";


    ALOGV("setParameters() %s", keyValuePairs.string());

    if (keyValuePairs.length() == 0) return BAD_VALUE;

    key = String8(BT_NREC_KEY);
    if (param.get(key, value) == NO_ERROR) {
        if (value == BT_NREC_VALUE_ON) {
            mBluetoothNrec = true;
        } else {
            mBluetoothNrec = false;
            ALOGI("Turning noise reduction and echo cancellation off for BT "
                 "headset");
        }
    }
    key = String8(BTHEADSET_VGS);
    if (param.get(key, value) == NO_ERROR) {
        if (value == BT_NREC_VALUE_ON) {
            mBluetoothVGS = true;
        } else {
            mBluetoothVGS = false;
        }
    }
    key = String8(BT_NAME_KEY);
    if (param.get(key, value) == NO_ERROR) {
       doRouting(NULL, 0);
   }
    key = String8(DUALMIC_KEY);
    if (param.get(key, value) == NO_ERROR) {
        if (value == "true") {
            mDualMicEnabled = true;
            ALOGI("DualMike feature Enabled");
        } else {
            mDualMicEnabled = false;
            ALOGI("DualMike feature Disabled");
        }
        doRouting(NULL, 0);
    }

    key = String8(TTY_MODE_KEY);
    if (param.get(key, value) == NO_ERROR) {
        if (value == "full" || value == "tty_full") {
            mTtyMode = TTY_FULL;
        } else if (value == "hco" || value == "tty_hco") {
            mTtyMode = TTY_HCO;
        } else if (value == "vco" || value == "tty_vco") {
            mTtyMode = TTY_VCO;
        } else {
            mTtyMode = TTY_OFF;
        }
        if(mMode != AUDIO_MODE_IN_CALL){
           return NO_ERROR;
        }
        ALOGI("Changed TTY Mode=%s", value.string());
        if((mMode == AUDIO_MODE_IN_CALL) &&
          (cur_rx == DEVICE_HEADSET_RX) &&
          (cur_tx == DEVICE_HEADSET_TX))
          doRouting(NULL, 0);
    }
    return NO_ERROR;
}

String8 AudioHardware::getParameters(const String8& keys)
{
    AudioParameter param = AudioParameter(keys);
    String8 value;

    String8 key = String8(DUALMIC_KEY);
    if (param.get(key, value) == NO_ERROR) {
        value = String8(mDualMicEnabled ? "true" : "false");
        param.add(key, value);
    }
    key = String8(BTHEADSET_VGS);
    if (param.get(key, value) == NO_ERROR) {
        if(mBluetoothVGS)
           param.addInt(String8("isVGS"), true);
    }
    key = String8("tunneled-input-formats");
    if ( param.get(key,value) == NO_ERROR ) {
        param.addInt(String8("AMR"), true );
        if (build_id[17] != '1') {
            param.addInt(String8("EVRC"), true );
            param.addInt(String8("QCELP"), true );
        }
    }

    ALOGV("AudioHardware::getParameters() %s", param.toString().string());
    return param.toString();
}


static unsigned calculate_audpre_table_index(unsigned index)
{
    switch (index) {
        case 48000:    return SAMP_RATE_INDX_48000;
        case 44100:    return SAMP_RATE_INDX_44100;
        case 32000:    return SAMP_RATE_INDX_32000;
        case 24000:    return SAMP_RATE_INDX_24000;
        case 22050:    return SAMP_RATE_INDX_22050;
        case 16000:    return SAMP_RATE_INDX_16000;
        case 12000:    return SAMP_RATE_INDX_12000;
        case 11025:    return SAMP_RATE_INDX_11025;
        case 8000:    return SAMP_RATE_INDX_8000;
        default:     return -1;
    }
}
size_t AudioHardware::getInputBufferSize(uint32_t sampleRate, int format, int channelCount)
{

    if ((format != AUDIO_FORMAT_PCM_16_BIT) &&
        (format != AUDIO_FORMAT_AMR_NB)      &&
        (format != AUDIO_FORMAT_EVRC)      &&
        (format != AUDIO_FORMAT_QCELP)  &&
        (format != AUDIO_FORMAT_AAC)){
        ALOGW("getInputBufferSize bad format: %d", format);
        return 0;
    }
    if (channelCount < 1 || channelCount > 2) {
        ALOGW("getInputBufferSize bad channel count: %d", channelCount);
        return 0;
    }

    if (format == AUDIO_FORMAT_AMR_NB)
       return 320*channelCount;
    else if (format == AUDIO_FORMAT_EVRC)
       return 230*channelCount;
    else if (format == AUDIO_FORMAT_QCELP)
       return 350*channelCount;
    else if (format == AUDIO_FORMAT_AAC)
       return 2048;
    else if (sampleRate == AUDIO_HW_VOIP_SAMPLERATE_8K)
       return 320*channelCount;
    else if (sampleRate == AUDIO_HW_VOIP_SAMPLERATE_16K)
       return 640*channelCount;
    else
    {
        if (build_id[17] == '1') {
            /*
            Return pcm record buffer size based on the sampling rate:
            If sampling rate >= 44.1 Khz, use 512 samples/channel pcm recording and
            If sampling rate < 44.1 Khz, use 256 samples/channel pcm recording
            */
           if(sampleRate>=44100)
               return 1024*channelCount;
           else
               return 512*channelCount;
        }
        else {
           return 2048*channelCount;
        }
    }
    return 0;
}
static status_t set_volume_rpc(uint32_t device,
                               uint32_t method,
                               uint32_t volume)
{
    ALOGV("set_volume_rpc(%d, %d, %d)\n", device, method, volume);

    if (device == -1UL) return NO_ERROR;
     return NO_ERROR;
}

status_t AudioHardware::setVoiceVolume(float v)
{
    if (v < 0.0) {
        ALOGW("setVoiceVolume(%f) under 0.0, assuming 0.0\n", v);
        v = 0.0;
    } else if (v > 1.0) {
        ALOGW("setVoiceVolume(%f) over 1.0, assuming 1.0\n", v);
        v = 1.0;
    }

    mVoiceVolume = v;

    int vol = lrint(v * 100.0);
    ALOGD("setVoiceVolume(%f)\n", v);
    ALOGI("Setting in-call volume to %d (available range is 0 to 100)\n", vol);

    if(msm_set_voice_rx_vol(vol)) {
        ALOGE("msm_set_voice_rx_vol(%d) failed errno = %d",vol,errno);
        return -1;
    }
    ALOGV("msm_set_voice_rx_vol(%d) succeeded",vol);
    return NO_ERROR;
}
#ifdef QCOM_FM_ENABLED
status_t AudioHardware::setFmVolume(float value)
{
    mFmVolume = value;

    Routing_table *temp = getNodeByStreamType(FM_RADIO);
    if(temp == NULL)
        return NO_ERROR;

    float vol;
    if (value < 0.0) {
        ALOGW("%s: (%f) Under 0.0, assuming 0.0\n", __FUNCTION__, value);
        value = 0.0;
    } else if (value > 1.0) {
        ALOGW("%s: (%f) Over 1.0, assuming 1.0\n", __FUNCTION__, value);
        value = 1.0;
    }

    vol = value * 100;

    ALOGD("Setting FM volume to %f", vol);

    if(msm_set_volume(temp->dec_id, vol)) {
        ALOGE("msm_set_volume(%d) failed for FM errno = %d", vol, errno);
        return -1;
    }

    return NO_ERROR;
}
#endif

status_t AudioHardware::setMasterVolume(float v)
{
    Mutex::Autolock lock(mLock);
    int vol = ceil(v * 7.0);
    ALOGI("Set master volume to %d.\n", vol);

    set_volume_rpc(SND_DEVICE_HANDSET, SND_METHOD_VOICE, vol);
    set_volume_rpc(SND_DEVICE_SPEAKER, SND_METHOD_VOICE, vol);
    set_volume_rpc(SND_DEVICE_BT,      SND_METHOD_VOICE, vol);
    set_volume_rpc(SND_DEVICE_HEADSET, SND_METHOD_VOICE, vol);
    //TBD - does HDMI require this handling

    // We return an error code here to let the audioflinger do in-software
    // volume on top of the maximum volume that we set through the SND API.
    // return error - software mixer will handle it
    return -1;
}

static status_t do_route_audio_rpc(uint32_t device,
                                   bool ear_mute, bool mic_mute)
{
    if(device == -1)
        return 0;

    int new_rx_device = INVALID_DEVICE,new_tx_device = INVALID_DEVICE,fm_device = INVALID_DEVICE;
    Routing_table* temp = NULL;
    ALOGV("do_route_audio_rpc(%d, %d, %d)", device, ear_mute, mic_mute);

    if(device == SND_DEVICE_HANDSET) {
        new_rx_device = DEVICE_HANDSET_RX;
        new_tx_device = DEVICE_HANDSET_TX;
        ALOGV("In HANDSET");
    }
    else if(device == SND_DEVICE_SPEAKER) {
        new_rx_device = DEVICE_SPEAKER_RX;
        new_tx_device = DEVICE_SPEAKER_TX;
        ALOGV("In SPEAKER");
    }
    else if(device == SND_DEVICE_HEADSET) {
        new_rx_device = DEVICE_HEADSET_RX;
        new_tx_device = DEVICE_HEADSET_TX;
        ALOGV("In HEADSET");
    }
    else if(device == SND_DEVICE_NO_MIC_HEADSET) {
        new_rx_device = DEVICE_HEADSET_RX;
        new_tx_device = DEVICE_HANDSET_TX;
        ALOGV("In NO MIC HEADSET");
    }
    else if (device == SND_DEVICE_FM_HANDSET) {
        fm_device = DEVICE_FMRADIO_HANDSET_RX;
        ALOGV("In FM HANDSET");
    }
    else if(device == SND_DEVICE_FM_SPEAKER) {
        fm_device = DEVICE_FMRADIO_SPEAKER_RX;
        ALOGV("In FM SPEAKER");
    }
    else if(device == SND_DEVICE_FM_HEADSET) {
        fm_device = DEVICE_FMRADIO_HEADSET_RX;
        ALOGV("In FM HEADSET");
    }
    else if(device == SND_DEVICE_IN_S_SADC_OUT_HANDSET) {
        new_rx_device = DEVICE_HANDSET_RX;
        new_tx_device = DEVICE_DUALMIC_HANDSET_TX;
        ALOGV("In DUALMIC_HANDSET");
    }
    else if(device == SND_DEVICE_IN_S_SADC_OUT_SPEAKER_PHONE) {
        new_rx_device = DEVICE_SPEAKER_RX;
        new_tx_device = DEVICE_DUALMIC_SPEAKER_TX;
        ALOGV("In DUALMIC_SPEAKER");
    }
    else if(device == SND_DEVICE_TTY_FULL) {
        new_rx_device = DEVICE_TTY_HEADSET_MONO_RX;
        new_tx_device = DEVICE_TTY_HEADSET_MONO_TX;
        ALOGV("In TTY_FULL");
    }
    else if(device == SND_DEVICE_TTY_VCO) {
        new_rx_device = DEVICE_TTY_HEADSET_MONO_RX;
        new_tx_device = DEVICE_HANDSET_TX;
        ALOGV("In TTY_VCO");
    }
    else if(device == SND_DEVICE_TTY_HCO) {
        new_rx_device = DEVICE_HANDSET_RX;
        new_tx_device = DEVICE_TTY_HEADSET_MONO_TX;
        ALOGV("In TTY_HCO");
    }
    else if(device == SND_DEVICE_BT) {
        new_rx_device = DEVICE_BT_SCO_RX;
        new_tx_device = DEVICE_BT_SCO_TX;
        ALOGV("In BT_HCO");
    }
    else if(device == SND_DEVICE_HEADSET_AND_SPEAKER) {
        new_rx_device = DEVICE_SPEAKER_HEADSET_RX;
        new_tx_device = DEVICE_HEADSET_TX;
        ALOGV("In DEVICE_SPEAKER_HEADSET_RX and DEVICE_HEADSET_TX");
    }
    else if(device == SND_DEVICE_HEADPHONE_AND_SPEAKER) {
        new_rx_device = DEVICE_SPEAKER_HEADSET_RX;
        new_tx_device = DEVICE_HANDSET_TX;
        ALOGV("In DEVICE_SPEAKER_HEADSET_RX and DEVICE_HANDSET_TX");
    }
    else if (device == SND_DEVICE_HDMI) {
        new_rx_device = DEVICE_HDMI_STERO_RX;
        new_tx_device = cur_tx;
        ALOGV("In DEVICE_HDMI_STERO_RX and cur_tx");
    }else if(device == SND_DEVICE_FM_TX){
        new_rx_device = DEVICE_FMRADIO_STEREO_RX;
        new_tx_device = cur_tx;
        ALOGV("In DEVICE_FMRADIO_STEREO_RX and cur_tx");
    }
#ifdef SAMSUNG_AUDIO
    else if (device == SND_DEVICE_VOIP_HANDSET) {
        new_rx_device = DEVICE_HANDSET_VOIP_RX;
        new_tx_device = DEVICE_HANDSET_VOIP_TX;
        ALOGV("In VOIP HANDSET");
    }
    else if (device == SND_DEVICE_VOIP_SPEAKER) {
        new_rx_device = DEVICE_SPEAKER_VOIP_RX;
        new_tx_device = DEVICE_SPEAKER_VOIP_TX;
        ALOGV("In VOIP SPEAKER");
    }
    else if (device == SND_DEVICE_VOIP_HEADSET) {
        new_rx_device = DEVICE_HEADSET_VOIP_RX;
        new_tx_device = DEVICE_HEADSET_VOIP_TX;
        ALOGV("In VOIP HEADSET");
    }
    else if (device == SND_DEVICE_CALL_HANDSET) {
        new_rx_device = DEVICE_HANDSET_CALL_RX;
        new_tx_device = DEVICE_HANDSET_CALL_TX;
        ALOGV("In CALL HANDSET");
    }
    else if (device == SND_DEVICE_CALL_SPEAKER) {
        new_rx_device = DEVICE_SPEAKER_CALL_RX;
        new_tx_device = DEVICE_SPEAKER_CALL_TX;
        ALOGV("In CALL SPEAKER");
    }
    else if (device == SND_DEVICE_CALL_HEADSET) {
        new_rx_device = DEVICE_HEADSET_CALL_RX;
        new_tx_device = DEVICE_HEADSET_CALL_TX;
        ALOGV("In CALL HEADSET");
    }
#endif
#ifdef SEMC_AUDIO
    else if (device == SND_DEVICE_SPEAKER_DUAL_MIC_REAL_STEREO) {
        new_rx_device = cur_rx;
        new_tx_device = DEVICE_SPEAKER_DUAL_MIC_REAL_STEREO_TX;
        ALOGV("In dual mic real stereo endfire");
    }
#endif

    if(new_rx_device != INVALID_DEVICE)
        ALOGD("new_rx = %d", DEV_ID(new_rx_device));
    if(new_tx_device != INVALID_DEVICE)
        ALOGD("new_tx = %d", DEV_ID(new_tx_device));

    if (ear_mute == false && !isStreamOn(VOICE_CALL)) {
        ALOGV("Going to enable RX/TX device for voice stream");
            // Routing Voice
            if ( (new_rx_device != INVALID_DEVICE) && (new_tx_device != INVALID_DEVICE))
            {
                ALOGD("Starting voice on Rx %d and Tx %d device", DEV_ID(new_rx_device), DEV_ID(new_tx_device));
                msm_route_voice(DEV_ID(new_rx_device),DEV_ID(new_tx_device), 1);
            }
            else
            {
                return -1;
            }

            if(cur_rx == INVALID_DEVICE || new_rx_device == INVALID_DEVICE)
                return -1;

            if(cur_tx == INVALID_DEVICE || new_tx_device == INVALID_DEVICE)
                return -1;

           //Enable RX device
           if(new_rx_device != cur_rx) {
               enableDevice(cur_rx,0);
           }
           enableDevice(new_rx_device,1);

           //Enable TX device
           if(new_tx_device != cur_tx) {
               enableDevice(cur_tx,0);
           }
           enableDevice(new_tx_device,1);

            // start Voice call
            ALOGD("Starting voice call and UnMuting the call");
            msm_start_voice();
            msm_set_voice_tx_mute(0);
            cur_rx = new_rx_device;
            cur_tx = new_tx_device;
            addToTable(0,cur_rx,cur_tx,VOICE_CALL,true);
            updateDeviceInfo(new_rx_device,new_tx_device);
    }
    else if (ear_mute == true && isStreamOnAndActive(VOICE_CALL)) {
        ALOGV("Going to disable RX/TX device during end of voice call");
        temp = getNodeByStreamType(VOICE_CALL);
        if(temp == NULL)
            return 0;

        // Ending voice call
        ALOGD("Ending Voice call");
        msm_end_voice();
        deleteFromTable(VOICE_CALL);
        updateDeviceInfo(new_rx_device,new_tx_device);
        if(new_rx_device != INVALID_DEVICE && new_tx_device != INVALID_DEVICE) {
            cur_rx = new_rx_device;
            cur_tx = new_tx_device;
        }
    }
    else {
        updateDeviceInfo(new_rx_device,new_tx_device);
    }
    return NO_ERROR;
}

// always call with mutex held
status_t AudioHardware::doAudioRouteOrMute(uint32_t device)
{
// BT acoustics is not supported. This might be used by OEMs. Hence commenting
// the code and not removing it.
#if 0
    if (device == (uint32_t)SND_DEVICE_BT || device == (uint32_t)SND_DEVICE_CARKIT) {
        if (mBluetoothId) {
            device = mBluetoothId;
        } else if (!mBluetoothNrec) {
            device = SND_DEVICE_BT_EC_OFF;
        }
    }
#endif

    status_t ret = NO_ERROR;

    ALOGV("doAudioRouteOrMute() device %x, mMode %d, mMicMute %d", device, mMode, mMicMute);
    int earMute = (mMode != AUDIO_MODE_IN_CALL) && (mMode != AUDIO_MODE_IN_COMMUNICATION);
    ret = do_route_audio_rpc(device, earMute, mMicMute);

    if (isStreamOnAndActive(VOICE_CALL) && mMicMute == false)
        msm_set_voice_tx_mute(0);

    if (isInCall())
        setVoiceVolume(mVoiceVolume);

    return ret;
}

status_t AudioHardware::doRouting(AudioStreamInMSM72xx *input, uint32_t outputDevices)
{
    Mutex::Autolock lock(mLock);
    status_t ret = NO_ERROR;
    int audProcess = (ADRC_DISABLE | EQ_DISABLE | RX_IIR_DISABLE);
    int sndDevice = -1;

    if (!outputDevices)
        outputDevices = mOutput->devices();

    ALOGD("outputDevices = %x", outputDevices);

    if (input != NULL) {
        uint32_t inputDevice = input->devices();
        ALOGI("do input routing device %x\n", inputDevice);
        // ignore routing device information when we start a recording in voice
        // call
        // Recording will happen through currently active tx device
        if((inputDevice == AUDIO_DEVICE_IN_VOICE_CALL)
#ifdef QCOM_FM_ENABLED
           || (inputDevice == AUDIO_DEVICE_IN_FM_RX)
           || (inputDevice == AUDIO_DEVICE_IN_FM_RX_A2DP)
#endif
          )
            return NO_ERROR;
        if (inputDevice & AUDIO_DEVICE_BIT_IN) {
            if (inputDevice == AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET) {
                ALOGI("Routing audio to Bluetooth PCM\n");
                sndDevice = SND_DEVICE_BT;
#ifdef SEMC_AUDIO
            // Camcorder case
            } else if (inputDevice & 0x80000004) {
                ALOGI("Routing audio to dual mic real stereo\n");
                sndDevice = SND_DEVICE_SPEAKER_DUAL_MIC_REAL_STEREO;
#endif
            } else if (inputDevice == AUDIO_DEVICE_IN_WIRED_HEADSET) {
                if ((outputDevices & AUDIO_DEVICE_OUT_WIRED_HEADSET) &&
                    (outputDevices & AUDIO_DEVICE_OUT_SPEAKER)) {
                    ALOGI("Routing audio to Wired Headset and Speaker\n");
                    sndDevice = SND_DEVICE_HEADSET_AND_SPEAKER;
                    audProcess = (ADRC_ENABLE | EQ_ENABLE | RX_IIR_ENABLE | MBADRC_ENABLE);
                } else {
                    ALOGI("Routing audio to Wired Headset\n");
                    sndDevice = SND_DEVICE_HEADSET;
                }
            } else {
                if (outputDevices == AUDIO_DEVICE_OUT_WIRED_HEADPHONE) {
                    ALOGI("Routing audio to Speakerphone\n");
                    sndDevice = SND_DEVICE_NO_MIC_HEADSET;
                } else if (outputDevices & AUDIO_DEVICE_OUT_EARPIECE) {
                    ALOGI("Routing audio to Handset\n");
                    sndDevice = SND_DEVICE_HANDSET;
                } else {
                    ALOGI("Routing audio to Speakerphone\n");
                    sndDevice = SND_DEVICE_SPEAKER;
                }
            }
        }
    }

    if (sndDevice == -1) {
        if (outputDevices & (outputDevices - 1)) {
            if ((outputDevices & AUDIO_DEVICE_OUT_SPEAKER) == 0) {
                ALOGW("Hardware does not support requested route combination (%#X),"
                     " picking closest possible route...", outputDevices);
            }
        }
        if ((mTtyMode != TTY_OFF) && (mMode == AUDIO_MODE_IN_CALL) &&
                (outputDevices & AUDIO_DEVICE_OUT_WIRED_HEADSET)) {
            if (mTtyMode == TTY_FULL) {
                ALOGI("Routing audio to TTY FULL Mode\n");
                sndDevice = SND_DEVICE_TTY_FULL;
            } else if (mTtyMode == TTY_VCO) {
                ALOGI("Routing audio to TTY VCO Mode\n");
                sndDevice = SND_DEVICE_TTY_VCO;
            } else if (mTtyMode == TTY_HCO) {
                ALOGI("Routing audio to TTY HCO Mode\n");
                sndDevice = SND_DEVICE_TTY_HCO;
            }
        } else if (outputDevices &
                   (AUDIO_DEVICE_OUT_BLUETOOTH_SCO | AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET)) {
            ALOGI("Routing audio to Bluetooth PCM\n");
            sndDevice = SND_DEVICE_BT;
        } else if (outputDevices & AUDIO_DEVICE_OUT_BLUETOOTH_SCO_CARKIT) {
            ALOGI("Routing audio to Bluetooth PCM\n");
            sndDevice = SND_DEVICE_CARKIT;
        } else if (outputDevices & AUDIO_DEVICE_OUT_AUX_DIGITAL) {
            ALOGI("Routing audio to HDMI\n");
            sndDevice = SND_DEVICE_HDMI;
        } else if ((outputDevices & AUDIO_DEVICE_OUT_WIRED_HEADSET) &&
                   (outputDevices & AUDIO_DEVICE_OUT_SPEAKER)) {
            ALOGI("Routing audio to Wired Headset and Speaker\n");
            sndDevice = SND_DEVICE_HEADSET_AND_SPEAKER;
            audProcess = (ADRC_ENABLE | EQ_ENABLE | RX_IIR_ENABLE | MBADRC_ENABLE);
        }
#ifdef QCOM_FM_ENABLED
        else if ((outputDevices & AUDIO_DEVICE_OUT_FM_TX) &&
                   (outputDevices & AUDIO_DEVICE_OUT_SPEAKER)) {
            ALOGI("Routing audio to FM Tx and Speaker\n");
            sndDevice = SND_DEVICE_FM_TX_AND_SPEAKER;
            enableComboDevice(sndDevice,1);
            audProcess = (ADRC_ENABLE | EQ_ENABLE | RX_IIR_ENABLE | MBADRC_ENABLE);
        }
#endif
        else if (outputDevices & AUDIO_DEVICE_OUT_WIRED_HEADPHONE) {
            if (outputDevices & AUDIO_DEVICE_OUT_SPEAKER) {
                ALOGI("Routing audio to No microphone Wired Headset and Speaker (%d,%x)\n", mMode, outputDevices);
                sndDevice = SND_DEVICE_HEADPHONE_AND_SPEAKER;
                audProcess = (ADRC_ENABLE | EQ_ENABLE | RX_IIR_ENABLE | MBADRC_ENABLE);
            } else {
                ALOGI("Routing audio to No microphone Wired Headset (%d,%x)\n", mMode, outputDevices);
                sndDevice = SND_DEVICE_NO_MIC_HEADSET;
            }
        } else if (outputDevices & AUDIO_DEVICE_OUT_WIRED_HEADSET) {
            ALOGI("Routing audio to Wired Headset\n");
            sndDevice = SND_DEVICE_HEADSET;
            audProcess = (ADRC_ENABLE | EQ_ENABLE | RX_IIR_ENABLE | MBADRC_ENABLE);
        } else if (outputDevices & AUDIO_DEVICE_OUT_SPEAKER) {
            ALOGI("Routing audio to Speakerphone\n");
            sndDevice = SND_DEVICE_SPEAKER;
            audProcess = (ADRC_ENABLE | EQ_ENABLE | RX_IIR_ENABLE | MBADRC_ENABLE);
        } else if(outputDevices & AUDIO_DEVICE_OUT_EARPIECE){
            ALOGI("Routing audio to Handset\n");
            sndDevice = SND_DEVICE_HANDSET;
            audProcess = (ADRC_ENABLE | EQ_ENABLE | RX_IIR_ENABLE | MBADRC_ENABLE);
        }
#ifdef QCOM_FM_ENABLED
         else if(outputDevices & AUDIO_DEVICE_OUT_FM_TX){
            ALOGI("Routing audio to FM Tx Device\n");
            sndDevice = SND_DEVICE_FM_TX;
            audProcess = (ADRC_ENABLE | EQ_ENABLE | RX_IIR_ENABLE | MBADRC_ENABLE);
        }
#endif
    }

    if (mDualMicEnabled && mMode == AUDIO_MODE_IN_CALL) {
        if (sndDevice == SND_DEVICE_HANDSET) {
            ALOGI("Routing audio to handset with DualMike enabled\n");
            sndDevice = SND_DEVICE_IN_S_SADC_OUT_HANDSET;
        } else if (sndDevice == SND_DEVICE_SPEAKER) {
            ALOGI("Routing audio to speakerphone with DualMike enabled\n");
            sndDevice = SND_DEVICE_IN_S_SADC_OUT_SPEAKER_PHONE;
        }
    }

#ifdef SAMSUNG_AUDIO
    if (mMode == AudioSystem::MODE_IN_CALL) {
        if (sndDevice == SND_DEVICE_HANDSET) {
            ALOGI("Routing audio to Call Handset\n");
            sndDevice = SND_DEVICE_CALL_HANDSET;
        } else if (sndDevice == SND_DEVICE_SPEAKER) {
            ALOGI("Routing audio to Call Speaker\n");
            sndDevice = SND_DEVICE_CALL_SPEAKER;
        } else if (sndDevice == SND_DEVICE_HEADSET) {
            ALOGI("Routing audio to Call Headset\n");
            sndDevice = SND_DEVICE_CALL_HEADSET;
        }
    } else if (mMode == AudioSystem::MODE_IN_COMMUNICATION) {
        if (sndDevice == SND_DEVICE_HANDSET) {
            ALOGI("Routing audio to VOIP handset\n");
            sndDevice = SND_DEVICE_VOIP_HANDSET;
        } else if (sndDevice == SND_DEVICE_SPEAKER) {
            ALOGI("Routing audio to VOIP speaker\n");
            sndDevice = SND_DEVICE_VOIP_SPEAKER;
        } else if (sndDevice == SND_DEVICE_HEADSET) {
            ALOGI("Routing audio to VOIP headset\n");
            sndDevice = SND_DEVICE_VOIP_HEADSET;
        }
    }
#endif

    if ((CurrentComboDeviceData.DeviceId == INVALID_DEVICE) &&
        (sndDevice == SND_DEVICE_FM_TX_AND_SPEAKER )){
        /* speaker rx is already enabled change snd device to the fm tx
         * device and let the flow take the regular route to
         * updatedeviceinfo().
         */
        Mutex::Autolock lock_1(mComboDeviceLock);

        CurrentComboDeviceData.DeviceId = SND_DEVICE_FM_TX_AND_SPEAKER;
        sndDevice = DEVICE_FMRADIO_STEREO_RX;
    }
    else if(CurrentComboDeviceData.DeviceId != INVALID_DEVICE){
        /* time to disable the combo device */
        enableComboDevice(CurrentComboDeviceData.DeviceId,0);
        Mutex::Autolock lock_2(mComboDeviceLock);
        CurrentComboDeviceData.DeviceId = INVALID_DEVICE;
        CurrentComboDeviceData.StreamType = INVALID_STREAM;
    }

    if (sndDevice == SND_DEVICE_HEADSET)
    {
        char value[PROPERTY_VALUE_MAX];
        property_get("persist.service.audio.hs_intmic", value, "0");
        if (!strcmp(value, "1"))
            sndDevice = SND_DEVICE_NO_MIC_HEADSET;
    }

    if (sndDevice != -1 && sndDevice != mCurSndDevice) {
        ret = doAudioRouteOrMute(sndDevice);
        mCurSndDevice = sndDevice;
    }

    return ret;
}
status_t AudioHardware::enableComboDevice(uint32_t sndDevice, bool enableOrDisable)
{
    ALOGD("enableComboDevice %u",enableOrDisable);
    status_t status = NO_ERROR;
    Routing_table *LpaNode = getNodeByStreamType(LPA_DECODE);
    Routing_table *PcmNode = getNodeByStreamType(PCM_PLAY);


    if(SND_DEVICE_FM_TX_AND_SPEAKER == sndDevice){

        if(getNodeByStreamType(VOICE_CALL) || getNodeByStreamType(FM_RADIO) ||
           getNodeByStreamType(FM_A2DP)){
            ALOGE("voicecall/FM radio active bailing out");
            return NO_ERROR;
        }

        if(!LpaNode && !PcmNode) {
            ALOGE("No active playback session active bailing out ");
            return NO_ERROR;
        }

        Mutex::Autolock lock_1(mComboDeviceLock);

        Routing_table* temp = NULL;

        if (enableOrDisable == 1) {

            if(enableDevice(DEVICE_SPEAKER_RX, 1)) {
                ALOGE("enableDevice failed for device %d", DEVICE_SPEAKER_RX);
                return -1;
            }


            if(CurrentComboDeviceData.StreamType == INVALID_STREAM){
                if (PcmNode){
                    temp = PcmNode;
                    CurrentComboDeviceData.StreamType = PCM_PLAY;
                    ALOGD("PCM_PLAY session Active ");
                }else if(LpaNode){
                    temp = LpaNode;
                    CurrentComboDeviceData.StreamType = LPA_DECODE;
                    ALOGD("LPA_DECODE session Active ");
                } else {
                    ALOGE("no PLAYback session Active ");
                    return -1;
                }
            }else
                temp = getNodeByStreamType(CurrentComboDeviceData.StreamType);

            if(temp == NULL){
                ALOGE("null check:fatal error:temp cannot be null");
                return -1;
            }

            ALOGD("combo:msm_route_stream(%d,%d,1)",temp->dec_id,
                DEV_ID(DEVICE_SPEAKER_RX));
            if(msm_route_stream(PCM_PLAY, temp->dec_id, DEV_ID(DEVICE_SPEAKER_RX),
                1)) {
                ALOGE("msm_route_stream failed");
                return -1;
            }

        }else if(enableOrDisable == 0) {
            temp = getNodeByStreamType(CurrentComboDeviceData.StreamType);


            if(temp == NULL){
                ALOGE("null check:fatal error:temp cannot be null");
                return -1;
            }

            ALOGD("combo:de-route msm_route_stream(%d,%d,0)",temp->dec_id,
                DEV_ID(DEVICE_SPEAKER_RX));
            if(msm_route_stream(PCM_PLAY, temp->dec_id,
                DEV_ID(DEVICE_SPEAKER_RX), 0)) {
                ALOGE("msm_route_stream failed");
                return -1;
            }

            if(enableDevice(DEVICE_SPEAKER_RX, 0)) {
                ALOGE("enableDevice failed for device %d", DEVICE_SPEAKER_RX);
                return -1;
            }
        }

    }

    return status;
}

status_t AudioHardware::enableFM()
{
    ALOGD("enableFM");
    status_t status = NO_INIT;
    unsigned short session_id = INVALID_DEVICE;
    status = ::open(FM_DEVICE, O_RDWR);
    if (status < 0) {
           ALOGE("Cannot open FM_DEVICE errno: %d", errno);
           goto Error;
    }
    mFmFd = status;
    if(ioctl(mFmFd, AUDIO_GET_SESSION_ID, &session_id)) {
           ALOGE("AUDIO_GET_SESSION_ID failed*********");
           goto Error;
    }

    if(enableDevice(DEVICE_FMRADIO_STEREO_TX, 1)) {
           ALOGE("enableDevice failed for device %d", DEVICE_FMRADIO_STEREO_TX);
           goto Error;
    }
    if(msm_route_stream(PCM_PLAY, session_id, DEV_ID(DEVICE_FMRADIO_STEREO_TX), 1)) {
           ALOGE("msm_route_stream failed");
           goto Error;
    }
    addToTable(session_id,cur_rx,INVALID_DEVICE,FM_RADIO,true);

#ifdef QCOM_FM_ENABLED
    setFmVolume(mFmVolume);
#endif

    enableDevice(cur_rx, 1);
    msm_route_stream(PCM_PLAY,session_id,DEV_ID(cur_rx),1);

    status = ioctl(mFmFd, AUDIO_START, 0);
    if (status < 0) {
            ALOGE("Cannot do AUDIO_START");
            goto Error;
    }
    return NO_ERROR;
    Error:
    if (mFmFd >= 0) {
        ::close(mFmFd);
        mFmFd = -1;
    }
    return NO_ERROR;
}

status_t AudioHardware::disableFM()
{
    ALOGD("disableFM");
    Routing_table* temp = NULL;
    temp = getNodeByStreamType(FM_RADIO);
    if(temp == NULL)
        return 0;
    if (mFmFd >= 0) {
            ::close(mFmFd);
            mFmFd = -1;
    }
    if(msm_route_stream(PCM_PLAY, temp->dec_id, DEV_ID(DEVICE_FMRADIO_STEREO_TX), 0)) {
           ALOGE("msm_route_stream failed");
           return 0;
    }
    if(!getNodeByStreamType(FM_A2DP)){
        if(enableDevice(DEVICE_FMRADIO_STEREO_TX, 0)) {
            ALOGE("Disabling device failed for device %d", DEVICE_FMRADIO_STEREO_TX);
        }
    }
    deleteFromTable(FM_RADIO);
    updateDeviceInfo(cur_rx, cur_tx);
    return NO_ERROR;
}


status_t AudioHardware::checkMicMute()
{
    Mutex::Autolock lock(mLock);
    if (mMode != AUDIO_MODE_IN_CALL) {
        setMicMute_nosync(true);
    }

    return NO_ERROR;
}

status_t AudioHardware::dumpInternals(int fd, const Vector<String16>& args)
{
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;
    result.append("AudioHardware::dumpInternals\n");
    snprintf(buffer, SIZE, "\tmInit: %s\n", mInit? "true": "false");
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmMicMute: %s\n", mMicMute? "true": "false");
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmBluetoothNrec: %s\n", mBluetoothNrec? "true": "false");
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmBluetoothId: %d\n", mBluetoothId);
    result.append(buffer);
    ::write(fd, result.string(), result.size());
    return NO_ERROR;
}

status_t AudioHardware::dump(int fd, const Vector<String16>& args)
{
    dumpInternals(fd, args);
    for (size_t index = 0; index < mInputs.size(); index++) {
        mInputs[index]->dump(fd, args);
    }

    if (mOutput) {
        mOutput->dump(fd, args);
    }
    return NO_ERROR;
}

uint32_t AudioHardware::getInputSampleRate(uint32_t sampleRate)
{
    uint32_t i;
    uint32_t prevDelta;
    uint32_t delta;

    for (i = 0, prevDelta = 0xFFFFFFFF; i < sizeof(inputSamplingRates)/sizeof(uint32_t); i++, prevDelta = delta) {
        delta = abs(sampleRate - inputSamplingRates[i]);
        if (delta > prevDelta) break;
    }
    // i is always > 0 here
    return inputSamplingRates[i-1];
}


AudioHardware::AudioStreamOutMSM72xx::AudioStreamOutMSM72xx() :
    mHardware(0), mFd(-1), mStartCount(0), mRetryCount(0), mStandby(true), mDevices(0)
{
}

status_t AudioHardware::AudioStreamOutMSM72xx::set(
        AudioHardware* hw, uint32_t devices, int *pFormat, uint32_t *pChannels, uint32_t *pRate)
{
    int lFormat = pFormat ? *pFormat : 0;
    uint32_t lChannels = pChannels ? *pChannels : 0;
    uint32_t lRate = pRate ? *pRate : 0;

    mHardware = hw;

    // fix up defaults
    if (lFormat == 0) lFormat = format();
    if (lChannels == 0) lChannels = channels();
    if (lRate == 0) lRate = sampleRate();

    // check values
    if ((lFormat != format()) ||
        (lChannels != channels()) ||
        (lRate != sampleRate())) {
        if (pFormat) *pFormat = format();
        if (pChannels) *pChannels = channels();
        if (pRate) *pRate = sampleRate();
        ALOGE("%s: Setting up correct values", __func__);
        return NO_ERROR;
    }

    if (pFormat) *pFormat = lFormat;
    if (pChannels) *pChannels = lChannels;
    if (pRate) *pRate = lRate;

    mDevices = devices;

    return NO_ERROR;
}

AudioHardware::AudioStreamOutMSM72xx::~AudioStreamOutMSM72xx()
{
    if (mFd >= 0) close(mFd);
}

ssize_t AudioHardware::AudioStreamOutMSM72xx::write(const void* buffer, size_t bytes)
{
    // ALOGD("AudioStreamOutMSM72xx::write(%p, %u)", buffer, bytes);
    status_t status = NO_INIT;
    size_t count = bytes;
    const uint8_t* p = static_cast<const uint8_t*>(buffer);
    unsigned short dec_id = INVALID_DEVICE;

    if (mStandby) {

        // open driver
        ALOGV("open driver");
        status = ::open("/dev/msm_pcm_out", O_WRONLY/*O_RDWR*/);
        if (status < 0) {
            ALOGE("Cannot open /dev/msm_pcm_out errno: %d", errno);
            goto Error;
        }
        mFd = status;

        // configuration
        ALOGV("get config");
        struct msm_audio_config config;
        status = ioctl(mFd, AUDIO_GET_CONFIG, &config);
        if (status < 0) {
            ALOGE("Cannot read config");
            goto Error;
        }

        ALOGV("set config");
        config.channel_count = AudioSystem::popCount(channels());
        config.sample_rate = sampleRate();
        config.buffer_size = bufferSize();
        config.buffer_count = AUDIO_HW_NUM_OUT_BUF;
        config.type = CODEC_TYPE_PCM;

        status = ioctl(mFd, AUDIO_SET_CONFIG, &config);
        if (status < 0) {
            ALOGE("Cannot set config");
            goto Error;
        }

        ALOGV("buffer_size: %u", config.buffer_size);
        ALOGV("buffer_count: %u", config.buffer_count);
        ALOGV("channel_count: %u", config.channel_count);
        ALOGV("sample_rate: %u", config.sample_rate);

        // fill 2 buffers before AUDIO_START
        mStartCount = AUDIO_HW_NUM_OUT_BUF;
        mStandby = false;
    }

    while (count) {
        ssize_t written = ::write(mFd, p, count);
        if (written >= 0) {
            count -= written;
            p += written;
        } else {
            if (errno != EAGAIN) return written;
            mRetryCount++;
            ALOGW("EAGAIN - retry");
        }
    }

    // start audio after we fill 2 buffers
    if (mStartCount) {
        if (--mStartCount == 0) {
            if(ioctl(mFd, AUDIO_GET_SESSION_ID, &dec_id)) {
                ALOGE("AUDIO_GET_SESSION_ID failed*********");
                return 0;
            }
            ALOGV("dec_id = %d\n",dec_id);
            if(cur_rx == INVALID_DEVICE)
                return 0;

            Mutex::Autolock lock(mDeviceSwitchLock);

            ALOGV("cur_rx for pcm playback = %d",cur_rx);
            if(enableDevice(cur_rx, 1)) {
                ALOGE("enableDevice failed for device cur_rx %d", cur_rx);
                return 0;
            }

            ALOGV("msm_route_stream(PCM_PLAY,%d,%d,1)",dec_id,DEV_ID(cur_rx));
            if(msm_route_stream(PCM_PLAY, dec_id, DEV_ID(cur_rx), 1)) {
                ALOGE("msm_route_stream failed");
                return 0;
            }
            Mutex::Autolock lock_1(mComboDeviceLock);

            if(CurrentComboDeviceData.DeviceId == SND_DEVICE_FM_TX_AND_SPEAKER){
                Routing_table *LpaNode = getNodeByStreamType(LPA_DECODE);

                /* This de-routes the LPA being routed on to speaker, which is done in
                  * enablecombo()
                  */
                if(LpaNode == NULL){
                    ALOGE("null check:fatal error:LpaNode cannot be null");
                    return -1;
                }
                ALOGD("combo:de-route:msm_route_stream(%d,%d,0)",LpaNode ->dec_id,
                    DEV_ID(DEVICE_SPEAKER_RX));
                if(msm_route_stream(PCM_PLAY, LpaNode ->dec_id, DEV_ID(DEVICE_SPEAKER_RX),
                    0)) {
                    ALOGE("msm_route_stream failed");
                    return -1;
                }


                ALOGD("Routing PCM stream to speaker for combo device");
                ALOGD("combo:msm_route_stream(PCM_PLAY,session id:%d,dev id:%d,1)",dec_id,
                    DEV_ID(DEVICE_SPEAKER_RX));

                if(msm_route_stream(PCM_PLAY, dec_id, DEV_ID(DEVICE_SPEAKER_RX),
                    1)) {
                    ALOGE("msm_route_stream failed");
                    return -1;
                }
                CurrentComboDeviceData.StreamType = PCM_PLAY;
            }
            addToTable(dec_id,cur_rx,INVALID_DEVICE,PCM_PLAY,true);
            ioctl(mFd, AUDIO_START, 0);
        }
    }
    return bytes;

Error:
    if (mFd >= 0) {
        ::close(mFd);
        mFd = -1;
    }
    // Simulate audio output timing in case of error
    usleep(bytes * 1000000 / frameSize() / sampleRate());

    return status;
}

status_t AudioHardware::AudioStreamOutMSM72xx::standby()
{
    Routing_table* temp = NULL;
    ALOGV("AudioStreamOutMSM72xx::standby()");
    status_t status = NO_ERROR;

    temp = getNodeByStreamType(PCM_PLAY);

    if(temp == NULL)
        return NO_ERROR;

    ALOGV("Deroute pcm out stream");
    if(msm_route_stream(PCM_PLAY, temp->dec_id,DEV_ID(temp->dev_id), 0)) {
        ALOGE("could not set stream routing\n");
        deleteFromTable(PCM_PLAY);
        return -1;
    }
    deleteFromTable(PCM_PLAY);
    updateDeviceInfo(cur_rx, cur_tx);

    if (!mStandby && mFd >= 0) {
        ::close(mFd);
        mFd = -1;
    }

    mStandby = true;
    return status;
}

status_t AudioHardware::AudioStreamOutMSM72xx::dump(int fd, const Vector<String16>& args)
{
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;
    result.append("AudioStreamOutMSM72xx::dump\n");
    snprintf(buffer, SIZE, "\tsample rate: %d\n", sampleRate());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tbuffer size: %d\n", bufferSize());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tchannels: %d\n", channels());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tformat: %d\n", format());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmHardware: %p\n", mHardware);
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmFd: %d\n", mFd);
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmStartCount: %d\n", mStartCount);
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmRetryCount: %d\n", mRetryCount);
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmStandby: %s\n", mStandby? "true": "false");
    result.append(buffer);
    ::write(fd, result.string(), result.size());
    return NO_ERROR;
}

bool AudioHardware::AudioStreamOutMSM72xx::checkStandby()
{
    return mStandby;
}


status_t AudioHardware::AudioStreamOutMSM72xx::setParameters(const String8& keyValuePairs)
{
    AudioParameter param = AudioParameter(keyValuePairs);
    String8 key;
    status_t status = NO_ERROR;
    int device;
    ALOGV("AudioStreamOutMSM72xx::setParameters() %s", keyValuePairs.string());

#ifdef QCOM_FM_ENABLED
    float fm_volume;
    key = String8(AudioParameter::keyFmVolume);
    if (param.getFloat(key, fm_volume) == NO_ERROR) {
        mHardware->setFmVolume(fm_volume);
        param.remove(key);
    }

    key = String8(AudioParameter::keyHandleFm);
    if (param.getInt(key, device) == NO_ERROR) {
        if (device & AUDIO_DEVICE_OUT_FM) {
            mDevices |= device;
            mHardware->enableFM();
        } else {
            mHardware->disableFM();
            mDevices &= device;
        }
        param.remove(key);
    }
#endif

    key = String8(AudioParameter::keyRouting);
    if (param.getInt(key, device) == NO_ERROR) {
        mDevices = device;
        ALOGV("set output routing %x", mDevices);
        status = mHardware->doRouting(NULL, device);
        param.remove(key);
    }

    if (param.size()) {
        status = BAD_VALUE;
    }
    return status;
}

String8 AudioHardware::AudioStreamOutMSM72xx::getParameters(const String8& keys)
{
    AudioParameter param = AudioParameter(keys);
    String8 value;
    String8 key = String8(AudioParameter::keyRouting);

    if (param.get(key, value) == NO_ERROR) {
        ALOGV("get routing %x", mDevices);
        param.addInt(key, (int)mDevices);
    }

    ALOGV("AudioStreamOutMSM72xx::getParameters() %s", param.toString().string());
    return param.toString();
}

status_t AudioHardware::AudioStreamOutMSM72xx::getRenderPosition(uint32_t *dspFrames)
{
    //TODO: enable when supported by driver
    return INVALID_OPERATION;
}

AudioHardware::AudioStreamOutDirect::AudioStreamOutDirect() :
    mHardware(0), mFd(-1), mStartCount(0), mRetryCount(0), mStandby(true), mDevices(0),mChannels(AUDIO_CHANNEL_OUT_MONO),
    mSampleRate(AUDIO_HW_VOIP_SAMPLERATE_8K), mBufferSize(AUDIO_HW_VOIP_BUFFERSIZE_8K)
{
}

status_t AudioHardware::AudioStreamOutDirect::set(
        AudioHardware* hw, uint32_t devices, int *pFormat, uint32_t *pChannels, uint32_t *pRate)
{
    int lFormat = pFormat ? *pFormat : 0;
    uint32_t lChannels = pChannels ? *pChannels : 0;
    uint32_t lRate = pRate ? *pRate : 0;

    ALOGV("AudioStreamOutDirect::set lFormat = %d lChannels= %u lRate = %u\n", lFormat, lChannels, lRate );
    mHardware = hw;

    // fix up defaults
    if (lFormat == 0) lFormat = format();
    if (lChannels == 0) lChannels = channels();
    if (lRate == 0) lRate = sampleRate();

    // check values
    if ((lFormat != format()) ||
        (lChannels != channels()) ||
        (lRate != sampleRate())) {
        if (pFormat) *pFormat = format();
        if (pChannels) *pChannels = channels();
        if (pRate) *pRate = sampleRate();
        ALOGE("  AudioStreamOutDirect::set return bad values\n");
        return BAD_VALUE;
    }

    if (pFormat) *pFormat = lFormat;
    if (pChannels) *pChannels = lChannels;
    if (pRate) *pRate = lRate;

    mDevices = devices;
    mChannels = lChannels;
    mSampleRate = lRate;

    if(mSampleRate == AUDIO_HW_VOIP_SAMPLERATE_8K) {
        mBufferSize = AUDIO_HW_VOIP_BUFFERSIZE_8K;
    } else if(mSampleRate == AUDIO_HW_VOIP_SAMPLERATE_16K) {
        mBufferSize = AUDIO_HW_VOIP_BUFFERSIZE_16K;
    } else {
        ALOGE("  AudioStreamOutDirect::set return bad values\n");
        return BAD_VALUE;
    }

    mHardware->mVoipOutActive = true;

    if (mHardware->mVoipInActive)
        mHardware->setupDeviceforVoipCall(true);

    return NO_ERROR;
}

AudioHardware::AudioStreamOutDirect::~AudioStreamOutDirect()
{
    ALOGV("AudioStreamOutDirect destructor");
    mHardware->mVoipOutActive = false;
    standby();
}

ssize_t AudioHardware::AudioStreamOutDirect::write(const void* buffer, size_t bytes)
{
    status_t status = NO_INIT;
    size_t count = bytes;
    const uint8_t* p = static_cast<const uint8_t*>(buffer);
    unsigned short dec_id = INVALID_DEVICE;
    ALOGV("AudioStreamOutDirect::write(%p, %ld)", buffer, bytes);

    if (mStandby) {
        if(mHardware->mVoipFd >= 0) {
            mFd = mHardware->mVoipFd;

            mHardware->mVoipOutActive = true;
            if (mHardware->mVoipInActive)
                mHardware->setupDeviceforVoipCall(true);

            mStandby = false;
        } else {
            //Routing Voice
            if ( (cur_rx != INVALID_DEVICE) && (cur_tx != INVALID_DEVICE))
            {
                ALOGV("Starting voice on Rx %d and Tx %d device", DEV_ID(cur_rx), DEV_ID(cur_tx));
                msm_route_voice(DEV_ID(cur_rx),DEV_ID(cur_tx), 1);
            }
            else
            {
                return -1;
            }

            //Enable RX device
            if(cur_rx != INVALID_DEVICE && (enableDevice(cur_rx,1) == -1))
            {
                return -1;
            }

            //Enable TX device
            if(cur_tx != INVALID_DEVICE&&(enableDevice(cur_tx,1) == -1))
            {
                return -1;
            }

            // start Voice call
            ALOGD("Starting voice call and UnMuting the call");
            msm_start_voice();
            msm_set_voice_tx_mute(0);
            addToTable(0,cur_rx,cur_tx,VOICE_CALL,true);


            // open driver
            ALOGV("open mvs driver");
            status = ::open(MVS_DEVICE, /*O_WRONLY*/ O_RDWR);
            if (status < 0) {
                ALOGE("Cannot open %s errno: %d",MVS_DEVICE, errno);
                goto Error;
            }
            mFd = status;
            mHardware->mVoipFd = mFd;
            // configuration
            ALOGV("get mvs config");
            struct msm_audio_mvs_config mvs_config;
            status = ioctl(mFd, AUDIO_GET_MVS_CONFIG, &mvs_config);
            if (status < 0) {
                ALOGE("Cannot read mvs config");
                goto Error;
            }

            ALOGV("set mvs config");
            mvs_config.mvs_mode = MVS_MODE_PCM;
            status = ioctl(mFd, AUDIO_SET_MVS_CONFIG, &mvs_config);
            if (status < 0) {
                ALOGE("Cannot set mvs config");
                goto Error;
            }

            ALOGV("start mvs config");
            status = ioctl(mFd, AUDIO_START, 0);
            if (status < 0) {
                ALOGE("Cannot start mvs driver");
                goto Error;
            }

            mHardware->mVoipOutActive = true;
            if (mHardware->mVoipInActive)
                mHardware->setupDeviceforVoipCall(true);

            // fill 2 buffers before AUDIO_START
            mStartCount = AUDIO_HW_NUM_OUT_BUF;
            mStandby = false;
        }
    }
    struct q5v2_msm_audio_mvs_frame audio_mvs_frame;
    audio_mvs_frame.frame_type = 0;
    while (count) {
        audio_mvs_frame.len = mBufferSize;
        memcpy(&audio_mvs_frame.voc_pkt, p, mBufferSize);
        size_t written = ::write(mFd, &audio_mvs_frame, sizeof(audio_mvs_frame));
        ALOGV(" mvs bytes count = %d written : %d \n", count, written);
        if (written == 0) {
            count -= mBufferSize;
            p += mBufferSize;
        } else {
            if (errno != EAGAIN) return written;
            mRetryCount++;
            ALOGW("EAGAIN - retry");
        }
    }

    // start audio after we fill 2 buffers
    if (mStartCount) {
        if (--mStartCount == 0) {
            if(ioctl(mFd, AUDIO_GET_SESSION_ID, &dec_id)) {
                ALOGE("AUDIO_GET_SESSION_ID failed*********");
                return 0;
            }
            ALOGD("write(): dec_id = %d cur_rx = %d\n",dec_id,cur_rx);
            if(cur_rx == INVALID_DEVICE) {
                //return 0; //temporary fix until team upmerges code to froyo tip
                cur_rx = 0;
                cur_tx = 1;
            }
        }
    }
    bytes = sizeof(audio_mvs_frame.voc_pkt);
    return bytes;

Error:
ALOGE("  write Error \n");
  //  mHardware->mLock.unlock();
    if (mFd >= 0) {
        ::close(mFd);
        mFd = -1;
        mHardware->mVoipFd = -1;
    }
    // Simulate audio output timing in case of error
    usleep(bytes * 1000000 / frameSize() / sampleRate());

    return status;
}

status_t AudioHardware::AudioStreamOutDirect::standby()
{
    Routing_table* temp = NULL;
    ALOGD("AudioStreamOutDirect::standby()");
    Mutex::Autolock lock(mHardware->mVoipLock);
    status_t status = NO_ERROR;

    ALOGD("Voipin %d driver fd %d", mHardware->mVoipInActive, mHardware->mVoipFd);
    mHardware->mVoipOutActive = false;
    if (mHardware->mVoipFd >= 0 && !mHardware->mVoipInActive) {
        ALOGE("mute and disbale device  ***\n");
        //Mute and disable the device.
        int ret = msm_set_voice_rx_vol(0);
        if (ret <0)
            ALOGE("Error %d setting volume\n", ret);

        ret = msm_set_voice_tx_mute(1);
        if (ret < 0)
            ALOGE("Error %d setting mute\n", ret);

        ret = msm_end_voice();
        if (ret < 0)
            ALOGE("Error %d ending voice\n", ret);

        if (mHardware->mVoipFd >= 0) {
            ret = ioctl(mHardware->mVoipFd, AUDIO_STOP, NULL);
            ALOGE("MVS stop returned %d \n", ret);
            ::close(mFd);
            mFd = mHardware->mVoipFd = -1;
            mHardware->setupDeviceforVoipCall(false);
            ALOGD("MVS driver closed %d mFd %d", __LINE__, mHardware->mVoipFd);
        }
       //mHardware->checkMicMute();
    }
    else
        ALOGE("Not closing MVS driver");
    mStandby = true;
    return status;
}

status_t AudioHardware::AudioStreamOutDirect::dump(int fd, const Vector<String16>& args)
{
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;
    result.append("AudioStreamOutDirect::dump\n");
    snprintf(buffer, SIZE, "\tsample rate: %d\n", sampleRate());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tbuffer size: %d\n", bufferSize());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tchannels: %d\n", channels());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tformat: %d\n", format());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmHardware: %p\n", mHardware);
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmFd: %d\n", mFd);
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmStartCount: %d\n", mStartCount);
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmRetryCount: %d\n", mRetryCount);
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmStandby: %s\n", mStandby? "true": "false");
    result.append(buffer);
    ::write(fd, result.string(), result.size());
    return NO_ERROR;
}

bool AudioHardware::AudioStreamOutDirect::checkStandby()
{
    return mStandby;
}


status_t AudioHardware::AudioStreamOutDirect::setParameters(const String8& keyValuePairs)
{
    AudioParameter param = AudioParameter(keyValuePairs);
    String8 key = String8(AudioParameter::keyRouting);
    status_t status = NO_ERROR;
    int device;
    ALOGV("AudioStreamOutDirect::setParameters() %s", keyValuePairs.string());

    if (param.getInt(key, device) == NO_ERROR) {
        mDevices = device;
        ALOGV("set output routing %x", mDevices);
        status = mHardware->doRouting(NULL, device);
        param.remove(key);
    }

    if (param.size()) {
        status = BAD_VALUE;
    }
    return status;
}

String8 AudioHardware::AudioStreamOutDirect::getParameters(const String8& keys)
{
    AudioParameter param = AudioParameter(keys);
    String8 value;
    String8 key = String8(AudioParameter::keyRouting);

    if (param.get(key, value) == NO_ERROR) {
        ALOGV("get routing %x", mDevices);
        param.addInt(key, (int)mDevices);
    }

    ALOGV("AudioStreamOutDirect::getParameters() %s", param.toString().string());
    return param.toString();
}

status_t AudioHardware::AudioStreamOutDirect::getRenderPosition(uint32_t *dspFrames)
{
    //TODO: enable when supported by driver
    return INVALID_OPERATION;
}


// ----------------------------------------------------------------------------

AudioHardware::AudioStreamInMSM72xx::AudioStreamInMSM72xx() :
    mHardware(0), mFd(-1), mState(AUDIO_INPUT_CLOSED), mRetryCount(0),
    mFormat(AUDIO_HW_IN_FORMAT), mChannels(AUDIO_HW_IN_CHANNELS),
    mSampleRate(AUDIO_HW_IN_SAMPLERATE), mBufferSize(AUDIO_HW_IN_BUFFERSIZE),
    mAcoustics((AudioSystem::audio_in_acoustics)0), mDevices(0)
{
}

status_t AudioHardware::AudioStreamInMSM72xx::set(
        AudioHardware* hw, uint32_t devices, int *pFormat, uint32_t *pChannels, uint32_t *pRate,
        AudioSystem::audio_in_acoustics acoustic_flags)
{
    if ((pFormat == 0) ||
        ((*pFormat != AUDIO_HW_IN_FORMAT) &&
         (*pFormat != AUDIO_FORMAT_AMR_NB) &&
         (*pFormat != AUDIO_FORMAT_EVRC) &&
         (*pFormat != AUDIO_FORMAT_QCELP) &&
         (*pFormat != AUDIO_FORMAT_AAC)))
    {
        *pFormat = AUDIO_HW_IN_FORMAT;
        return BAD_VALUE;
    }

    if((*pFormat == AUDIO_FORMAT_AAC) && (*pChannels & (AUDIO_CHANNEL_IN_VOICE_DNLINK |  AUDIO_CHANNEL_IN_VOICE_UPLINK))) {
        ALOGE("voice call recording in AAC format does not support");
        return BAD_VALUE;
    }

    if (pRate == 0) {
        return BAD_VALUE;
    }
    uint32_t rate = hw->getInputSampleRate(*pRate);
    if (rate != *pRate) {
        *pRate = rate;
        return BAD_VALUE;
    }

    if (pChannels == 0 || (*pChannels & (AUDIO_CHANNEL_IN_MONO | AUDIO_CHANNEL_IN_STEREO)) == 0) {
        *pChannels = AUDIO_HW_IN_CHANNELS;
        return BAD_VALUE;
    }

    mHardware = hw;

    ALOGV("AudioStreamInMSM72xx::set(%d, %d, %u)", *pFormat, *pChannels, *pRate);
    if (mFd >= 0) {
        ALOGE("Audio record already open");
        return -EPERM;
    }
    status_t status =0;
    struct msm_voicerec_mode voc_rec_cfg;
#ifdef QCOM_FM_ENABLED
    if(devices == AUDIO_DEVICE_IN_FM_RX_A2DP) {
        status = ::open("/dev/msm_a2dp_in", O_RDONLY);
        if (status < 0) {
            ALOGE("Cannot open /dev/msm_a2dp_in errno: %d", errno);
            goto Error;
        }
        mFd = status;
        // configuration
        ALOGV("get config");
        struct msm_audio_config config;
        status = ioctl(mFd, AUDIO_GET_CONFIG, &config);
        if (status < 0) {
            ALOGE("Cannot read config");
            goto Error;
        }

        ALOGV("set config");
        config.channel_count = AudioSystem::popCount((*pChannels) & (AUDIO_CHANNEL_IN_STEREO | AUDIO_CHANNEL_IN_MONO));
        config.sample_rate = *pRate;
        config.buffer_size = bufferSize();
        config.buffer_count = 2;
        config.type = CODEC_TYPE_PCM;
        status = ioctl(mFd, AUDIO_SET_CONFIG, &config);
        if (status < 0) {
            ALOGE("Cannot set config");
            if (ioctl(mFd, AUDIO_GET_CONFIG, &config) == 0) {
                if (config.channel_count == 1) {
                    *pChannels = AUDIO_CHANNEL_IN_MONO;
                } else {
                    *pChannels = AUDIO_CHANNEL_IN_STEREO;
                }
                *pRate = config.sample_rate;
            }
            goto Error;
        }

        ALOGV("confirm config");
        status = ioctl(mFd, AUDIO_GET_CONFIG, &config);
        if (status < 0) {
            ALOGE("Cannot read config");
            goto Error;
        }
        ALOGV("buffer_size: %u", config.buffer_size);
        ALOGV("buffer_count: %u", config.buffer_count);
        ALOGV("channel_count: %u", config.channel_count);
        ALOGV("sample_rate: %u", config.sample_rate);

        mDevices = devices;
        mFormat = AUDIO_HW_IN_FORMAT;
        mChannels = *pChannels;
        mSampleRate = config.sample_rate;
        mBufferSize = config.buffer_size;
    }
    else
#endif
    if(*pFormat == AUDIO_HW_IN_FORMAT)
    {
        // open audio input device
        status = ::open("/dev/msm_pcm_in", O_RDONLY);
        if (status < 0) {
            ALOGE("Cannot open /dev/msm_pcm_in errno: %d", errno);
            goto Error;
        }
        mFd = status;

        // configuration
        ALOGV("get config");
        struct msm_audio_config config;
        status = ioctl(mFd, AUDIO_GET_CONFIG, &config);
        if (status < 0) {
            ALOGE("Cannot read config");
            goto Error;
        }

        ALOGV("set config");
        config.channel_count = AudioSystem::popCount((*pChannels) & (AUDIO_CHANNEL_IN_STEREO | AUDIO_CHANNEL_IN_MONO));
        config.sample_rate = *pRate;
        config.buffer_size = bufferSize();
        config.buffer_count = 2;
        config.type = CODEC_TYPE_PCM;
        if (build_id[17] == '1') {//build 4.1
           /*
             Configure pcm record buffer size based on the sampling rate:
             If sampling rate >= 44.1 Khz, use 512 samples/channel pcm recording and
             If sampling rate < 44.1 Khz, use 256 samples/channel pcm recording
           */
            if(*pRate>=44100)
                config.buffer_size = 1024 * config.channel_count;
            else
                config.buffer_size = 512 * config.channel_count;
        }
        status = ioctl(mFd, AUDIO_SET_CONFIG, &config);
        if (status < 0) {
            ALOGE("Cannot set config");
            if (ioctl(mFd, AUDIO_GET_CONFIG, &config) == 0) {
                if (config.channel_count == 1) {
                    *pChannels = AUDIO_CHANNEL_IN_MONO;
                } else {
                    *pChannels = AUDIO_CHANNEL_IN_STEREO;
                }
                *pRate = config.sample_rate;
            }
            goto Error;
        }

        ALOGV("confirm config");
        status = ioctl(mFd, AUDIO_GET_CONFIG, &config);
        if (status < 0) {
            ALOGE("Cannot read config");
            goto Error;
        }
        ALOGV("buffer_size: %u", config.buffer_size);
        ALOGV("buffer_count: %u", config.buffer_count);
        ALOGV("channel_count: %u", config.channel_count);
        ALOGV("sample_rate: %u", config.sample_rate);

        mDevices = devices;
        mFormat = AUDIO_HW_IN_FORMAT;
        mChannels = *pChannels;
        if (mDevices == AUDIO_DEVICE_IN_VOICE_CALL)
         {
            if ((mChannels & AUDIO_CHANNEL_IN_VOICE_DNLINK) &&
                (mChannels & AUDIO_CHANNEL_IN_VOICE_UPLINK)) {
                 ALOGI("Recording Source: Voice Call Both Uplink and Downlink");
                 voc_rec_cfg.rec_mode = VOC_REC_BOTH;
            } else if (mChannels & AUDIO_CHANNEL_IN_VOICE_DNLINK) {
                 ALOGI("Recording Source: Voice Call DownLink");
                 voc_rec_cfg.rec_mode = VOC_REC_DOWNLINK;
            } else if (mChannels & AUDIO_CHANNEL_IN_VOICE_UPLINK) {
                 ALOGI("Recording Source: Voice Call UpLink");
                 voc_rec_cfg.rec_mode = VOC_REC_UPLINK;
            }
            if (ioctl(mFd, AUDIO_SET_INCALL, &voc_rec_cfg))
            {
                ALOGE("Error: AUDIO_SET_INCALL failed\n");
                goto  Error;
            }
        }
        mSampleRate = config.sample_rate;
        mBufferSize = config.buffer_size;
    }
    else if (*pFormat == AUDIO_FORMAT_EVRC)
    {
          ALOGI("Recording format: EVRC");
          // open evrc input device
          status = ::open(EVRC_DEVICE_IN, O_RDONLY);
          if (status < 0) {
              ALOGE("Cannot open evrc device for read");
              goto Error;
          }
          mFd = status;
          mDevices = devices;
          mChannels = *pChannels;

          if (mDevices == AUDIO_DEVICE_IN_VOICE_CALL)
          {
              if ((mChannels & AUDIO_CHANNEL_IN_VOICE_DNLINK) &&
                     (mChannels & AUDIO_CHANNEL_IN_VOICE_UPLINK)) {
                  ALOGI("Recording Source: Voice Call Both Uplink and Downlink");
                  voc_rec_cfg.rec_mode = VOC_REC_BOTH;
              } else if (mChannels & AUDIO_CHANNEL_IN_VOICE_DNLINK) {
                  ALOGI("Recording Source: Voice Call DownLink");
                  voc_rec_cfg.rec_mode = VOC_REC_DOWNLINK;
              } else if (mChannels & AUDIO_CHANNEL_IN_VOICE_UPLINK) {
                  ALOGI("Recording Source: Voice Call UpLink");
                  voc_rec_cfg.rec_mode = VOC_REC_UPLINK;
              }

              if (ioctl(mFd, AUDIO_SET_INCALL, &voc_rec_cfg))
              {
                 ALOGE("Error: AUDIO_SET_INCALL failed\n");
                 goto  Error;
              }
          }

          /* Config param */
          struct msm_audio_stream_config config;
          if(ioctl(mFd, AUDIO_GET_STREAM_CONFIG, &config))
          {
            ALOGE(" Error getting buf config param AUDIO_GET_STREAM_CONFIG \n");
            goto  Error;
          }

          ALOGV("The Config buffer size is %d", config.buffer_size);
          ALOGV("The Config buffer count is %d", config.buffer_count);

          mSampleRate =8000;
          mFormat = *pFormat;
          mBufferSize = 230;
          struct msm_audio_evrc_enc_config evrc_enc_cfg;

          if (ioctl(mFd, AUDIO_GET_EVRC_ENC_CONFIG, &evrc_enc_cfg))
          {
            ALOGE("Error: AUDIO_GET_EVRC_ENC_CONFIG failed\n");
            goto  Error;
          }

          ALOGV("The Config cdma_rate is %d", evrc_enc_cfg.cdma_rate);
          ALOGV("The Config min_bit_rate is %d", evrc_enc_cfg.min_bit_rate);
          ALOGV("The Config max_bit_rate is %d", evrc_enc_cfg.max_bit_rate);

          evrc_enc_cfg.min_bit_rate = 4;
          evrc_enc_cfg.max_bit_rate = 4;

          if (ioctl(mFd, AUDIO_SET_EVRC_ENC_CONFIG, &evrc_enc_cfg))
          {
            ALOGE("Error: AUDIO_SET_EVRC_ENC_CONFIG failed\n");
            goto  Error;
          }
    }
    else if (*pFormat == AUDIO_FORMAT_QCELP)
    {
          ALOGI("Recording format: QCELP");
          // open qcelp input device
          status = ::open(QCELP_DEVICE_IN, O_RDONLY);
          if (status < 0) {
              ALOGE("Cannot open qcelp device for read");
              goto Error;
          }
          mFd = status;
          mDevices = devices;
          mChannels = *pChannels;

          if (mDevices == AUDIO_DEVICE_IN_VOICE_CALL)
          {
              if ((mChannels & AUDIO_CHANNEL_IN_VOICE_DNLINK) &&
                  (mChannels & AUDIO_CHANNEL_IN_VOICE_UPLINK)) {
                  ALOGI("Recording Source: Voice Call Both Uplink and Downlink");
                  voc_rec_cfg.rec_mode = VOC_REC_BOTH;
              } else if (mChannels & AUDIO_CHANNEL_IN_VOICE_DNLINK) {
                  ALOGI("Recording Source: Voice Call DownLink");
                  voc_rec_cfg.rec_mode = VOC_REC_DOWNLINK;
              } else if (mChannels & AUDIO_CHANNEL_IN_VOICE_UPLINK) {
                  ALOGI("Recording Source: Voice Call UpLink");
                  voc_rec_cfg.rec_mode = VOC_REC_UPLINK;
              }

              if (ioctl(mFd, AUDIO_SET_INCALL, &voc_rec_cfg))
              {
                 ALOGE("Error: AUDIO_SET_INCALL failed\n");
                 goto  Error;
              }
          }

          /* Config param */
          struct msm_audio_stream_config config;
          if(ioctl(mFd, AUDIO_GET_STREAM_CONFIG, &config))
          {
            ALOGE(" Error getting buf config param AUDIO_GET_STREAM_CONFIG \n");
            goto  Error;
          }

          ALOGV("The Config buffer size is %d", config.buffer_size);
          ALOGV("The Config buffer count is %d", config.buffer_count);

          mSampleRate =8000;
          mFormat = *pFormat;
          mBufferSize = 350;

          struct msm_audio_qcelp_enc_config qcelp_enc_cfg;

          if (ioctl(mFd, AUDIO_GET_QCELP_ENC_CONFIG, &qcelp_enc_cfg))
          {
            ALOGE("Error: AUDIO_GET_QCELP_ENC_CONFIG failed\n");
            goto  Error;
          }

          ALOGV("The Config cdma_rate is %d", qcelp_enc_cfg.cdma_rate);
          ALOGV("The Config min_bit_rate is %d", qcelp_enc_cfg.min_bit_rate);
          ALOGV("The Config max_bit_rate is %d", qcelp_enc_cfg.max_bit_rate);

          qcelp_enc_cfg.min_bit_rate = 4;
          qcelp_enc_cfg.max_bit_rate = 4;

          if (ioctl(mFd, AUDIO_SET_QCELP_ENC_CONFIG, &qcelp_enc_cfg))
          {
            ALOGE("Error: AUDIO_SET_QCELP_ENC_CONFIG failed\n");
            goto  Error;
          }
    }
    else if (*pFormat == AUDIO_FORMAT_AMR_NB)
    {
          ALOGI("Recording format: AMR_NB");
          // open amr_nb input device
          status = ::open(AMRNB_DEVICE_IN, O_RDONLY);
          if (status < 0) {
              ALOGE("Cannot open amr_nb device for read");
              goto Error;
          }
          mFd = status;
          mDevices = devices;
          mChannels = *pChannels;

          if (mDevices == AUDIO_DEVICE_IN_VOICE_CALL)
          {
              if ((mChannels & AUDIO_CHANNEL_IN_VOICE_DNLINK) &&
                     (mChannels & AUDIO_CHANNEL_IN_VOICE_UPLINK)) {
                  ALOGI("Recording Source: Voice Call Both Uplink and Downlink");
                  voc_rec_cfg.rec_mode = VOC_REC_BOTH;
              } else if (mChannels & AUDIO_CHANNEL_IN_VOICE_DNLINK) {
                  ALOGI("Recording Source: Voice Call DownLink");
                  voc_rec_cfg.rec_mode = VOC_REC_DOWNLINK;
              } else if (mChannels & AUDIO_CHANNEL_IN_VOICE_UPLINK) {
                  ALOGI("Recording Source: Voice Call UpLink");
                  voc_rec_cfg.rec_mode = VOC_REC_UPLINK;
              }

              if (ioctl(mFd, AUDIO_SET_INCALL, &voc_rec_cfg))
              {
                 ALOGE("Error: AUDIO_SET_INCALL failed\n");
                 goto  Error;
              }
          }

          /* Config param */
          struct msm_audio_stream_config config;
          if(ioctl(mFd, AUDIO_GET_STREAM_CONFIG, &config))
          {
            ALOGE(" Error getting buf config param AUDIO_GET_STREAM_CONFIG \n");
            goto  Error;
          }

          ALOGV("The Config buffer size is %d", config.buffer_size);
          ALOGV("The Config buffer count is %d", config.buffer_count);

          mSampleRate =8000;
          mFormat = *pFormat;
          mBufferSize = 320;
          struct msm_audio_amrnb_enc_config_v2 amr_nb_cfg;

          if (ioctl(mFd, AUDIO_GET_AMRNB_ENC_CONFIG_V2, &amr_nb_cfg))
          {
            ALOGE("Error: AUDIO_GET_AMRNB_ENC_CONFIG_V2 failed\n");
            goto  Error;
          }

          ALOGV("The Config band_mode is %d", amr_nb_cfg.band_mode);
          ALOGV("The Config dtx_enable is %d", amr_nb_cfg.dtx_enable);
          ALOGV("The Config frame_format is %d", amr_nb_cfg.frame_format);

          amr_nb_cfg.band_mode = 7; /* Bit Rate 12.2 kbps MR122 */
          amr_nb_cfg.dtx_enable= 0;
          amr_nb_cfg.frame_format = 0; /* IF1 */

          if (ioctl(mFd, AUDIO_SET_AMRNB_ENC_CONFIG_V2, &amr_nb_cfg))
          {
            ALOGE("Error: AUDIO_SET_AMRNB_ENC_CONFIG_V2 failed\n");
            goto  Error;
          }
    }
    else if (*pFormat == AUDIO_FORMAT_AAC)
    {
          ALOGI("Recording format: AAC");
          // open aac input device
          status = ::open(AAC_DEVICE_IN, O_RDWR);
          if (status < 0) {
              ALOGE("Cannot open aac device for read");
              goto Error;
          }
          mFd = status;

          struct msm_audio_stream_config config;
          if(ioctl(mFd, AUDIO_GET_STREAM_CONFIG, &config))
          {
            ALOGE(" Error getting buf config param AUDIO_GET_STREAM_CONFIG \n");
            goto  Error;
          }

          ALOGE("The Config buffer size is %d", config.buffer_size);
          ALOGE("The Config buffer count is %d", config.buffer_count);


          struct msm_audio_aac_enc_config aac_enc_cfg;
          if (ioctl(mFd, AUDIO_GET_AAC_ENC_CONFIG, &aac_enc_cfg))
          {
            ALOGE("Error: AUDIO_GET_AAC_ENC_CONFIG failed\n");
            goto  Error;
          }

          ALOGV("The Config channels is %d", aac_enc_cfg.channels);
          ALOGV("The Config sample_rate is %d", aac_enc_cfg.sample_rate);
          ALOGV("The Config bit_rate is %d", aac_enc_cfg.bit_rate);
          ALOGV("The Config stream_format is %d", aac_enc_cfg.stream_format);

          mDevices = devices;
          mChannels = *pChannels;
          aac_enc_cfg.sample_rate = mSampleRate = *pRate;
          mFormat = *pFormat;
          mBufferSize = 2048;
          if (*pChannels & (AUDIO_CHANNEL_IN_MONO))
              aac_enc_cfg.channels =  1;
          else if (*pChannels & (AUDIO_CHANNEL_IN_STEREO))
              aac_enc_cfg.channels =  2;
          aac_enc_cfg.bit_rate = 128000;

          ALOGV("Setting the Config channels is %d", aac_enc_cfg.channels);
          ALOGV("Setting the Config sample_rate is %d", aac_enc_cfg.sample_rate);
          ALOGV("Setting the Config bit_rate is %d", aac_enc_cfg.bit_rate);
          ALOGV("Setting the Config stream_format is %d", aac_enc_cfg.stream_format);

          if (ioctl(mFd, AUDIO_SET_AAC_ENC_CONFIG, &aac_enc_cfg))
          {
            ALOGE("Error: AUDIO_SET_AAC_ENC_CONFIG failed\n");
            goto  Error;
          }
    }
    //mHardware->setMicMute_nosync(false);
    mState = AUDIO_INPUT_OPENED;

    if (!acoustic)
        return NO_ERROR;

    audpre_index = calculate_audpre_table_index(mSampleRate);
    tx_iir_index = (audpre_index * 2) + (hw->checkOutputStandby() ? 0 : 1);
    ALOGD("audpre_index = %d, tx_iir_index = %d\n", audpre_index, tx_iir_index);

    /**
     * If audio-preprocessing failed, we should not block record.
     */
    int (*msm72xx_set_audpre_params)(int, int);
    msm72xx_set_audpre_params = (int (*)(int, int))::dlsym(acoustic, "msm72xx_set_audpre_params");
    status = msm72xx_set_audpre_params(audpre_index, tx_iir_index);
    if (status < 0)
        ALOGE("Cannot set audpre parameters");

    int (*msm72xx_enable_audpre)(int, int, int);
    msm72xx_enable_audpre = (int (*)(int, int, int))::dlsym(acoustic, "msm72xx_enable_audpre");
    mAcoustics = acoustic_flags;
    status = msm72xx_enable_audpre((int)acoustic_flags, audpre_index, tx_iir_index);
    if (status < 0)
        ALOGE("Cannot enable audpre");

    return NO_ERROR;

Error:
    if (mFd >= 0) {
        ::close(mFd);
        mFd = -1;
    }
    return status;
}

AudioHardware::AudioStreamInMSM72xx::~AudioStreamInMSM72xx()
{
    ALOGV("AudioStreamInMSM72xx destructor");
    standby();
}

ssize_t AudioHardware::AudioStreamInMSM72xx::read( void* buffer, ssize_t bytes)
{
    unsigned short dec_id = INVALID_DEVICE;
    ALOGV("AudioStreamInMSM72xx::read(%p, %ld)", buffer, bytes);
    if (!mHardware) return -1;

    size_t count = bytes;
    size_t  aac_framesize= bytes;
    uint8_t* p = static_cast<uint8_t*>(buffer);
    uint32_t* recogPtr = (uint32_t *)p;
    uint16_t* frameCountPtr;
    uint16_t* frameSizePtr;

    if (mState < AUDIO_INPUT_OPENED) {
        AudioHardware *hw = mHardware;
        hw->mLock.lock();
        status_t status = set(hw, mDevices, &mFormat, &mChannels, &mSampleRate, mAcoustics);
        if (status != NO_ERROR) {
            hw->mLock.unlock();
            return -1;
        }
#ifdef QCOM_FM_ENABLED
        if((mDevices == AUDIO_DEVICE_IN_FM_RX) || (mDevices == AUDIO_DEVICE_IN_FM_RX_A2DP) ){
            if(ioctl(mFd, AUDIO_GET_SESSION_ID, &dec_id)) {
                ALOGE("AUDIO_GET_SESSION_ID failed*********");
                hw->mLock.unlock();
                return -1;
            }

            if(enableDevice(DEVICE_FMRADIO_STEREO_TX, 1)) {
                ALOGE("enableDevice failed for device %d",DEVICE_FMRADIO_STEREO_TX);
                hw->mLock.unlock();
                return -1;
             }

            if(msm_route_stream(PCM_REC, dec_id, DEV_ID(DEVICE_FMRADIO_STEREO_TX), 1)) {
                ALOGE("msm_route_stream failed");
                hw->mLock.unlock();
                return -1;
            }
            mFirstread = false;
            if (mDevices == AUDIO_DEVICE_IN_FM_RX_A2DP) {
                addToTable(dec_id,cur_tx,INVALID_DEVICE,FM_A2DP,true);
                mFmRec = FM_A2DP_REC;
            }
            else {
                addToTable(dec_id,cur_tx,INVALID_DEVICE,FM_REC,true);
                mFmRec = FM_FILE_REC;
            }
            hw->mLock.unlock();
        }
        else
#endif
{
            hw->mLock.unlock();
            if(ioctl(mFd, AUDIO_GET_SESSION_ID, &dec_id)) {
                ALOGE("AUDIO_GET_SESSION_ID failed*********");
                return -1;
            }
            ALOGV("dec_id = %d,cur_tx= %d",dec_id,cur_tx);
            if(cur_tx == INVALID_DEVICE)
                cur_tx = DEVICE_HANDSET_TX;

            Mutex::Autolock lock(mDeviceSwitchLock);

            if(enableDevice(cur_tx, 1)) {
                ALOGE("enableDevice failed, device %d",cur_tx);
                return -1;
            }
            if(msm_route_stream(PCM_REC, dec_id, DEV_ID(cur_tx), 1)) {
                ALOGE("msm_route_stream failed");
                return -1;
            }
            addToTable(dec_id,cur_tx,INVALID_DEVICE,PCM_REC,true);
            mFirstread = false;
        }
    }


    if (mState < AUDIO_INPUT_STARTED) {
        // force routing to input device
        mHardware->clearCurDevice();
        mHardware->doRouting(this, 0);
        if (ioctl(mFd, AUDIO_START, 0)) {
            ALOGE("Error starting record");
            standby();
            return -1;
        }
        mState = AUDIO_INPUT_STARTED;
    }

    bytes = 0;
    if(mFormat == AUDIO_HW_IN_FORMAT)
    {
        while (count) {
            ssize_t bytesRead = ::read(mFd, buffer, count);
            if (bytesRead >= 0) {
                count -= bytesRead;
                p += bytesRead;
                bytes += bytesRead;
                if(!mFirstread)
                {
                   mFirstread = true;
                   break;
                }
            } else {
                if (errno != EAGAIN) return bytesRead;
                mRetryCount++;
                ALOGW("EAGAIN - retrying");
            }
        }
    }
    else if ((mFormat == AUDIO_FORMAT_EVRC) || (mFormat == AUDIO_FORMAT_QCELP) || (mFormat == AUDIO_FORMAT_AMR_NB))
    {
        uint8_t readBuf[36];
        uint8_t *dataPtr;
        while (count) {
            dataPtr = readBuf;
            ssize_t bytesRead = ::read(mFd, readBuf, 36);
            if (bytesRead >= 0) {
                if (mFormat == AUDIO_FORMAT_AMR_NB){
                   amr_transcode(dataPtr,p);
                   p += AMRNB_FRAME_SIZE;
                   count -= AMRNB_FRAME_SIZE;
                   bytes += AMRNB_FRAME_SIZE;
                   if(!mFirstread)
                   {
                      mFirstread = true;
                      break;
                   }
                }
                else {
                    dataPtr++;
                    if (mFormat == AUDIO_FORMAT_EVRC){
                       memcpy(p, dataPtr, EVRC_FRAME_SIZE);
                       p += EVRC_FRAME_SIZE;
                       count -= EVRC_FRAME_SIZE;
                       bytes += EVRC_FRAME_SIZE;
                       if(!mFirstread)
                       {
                          mFirstread = true;
                          break;
                       }
                    }
                    else if (mFormat == AUDIO_FORMAT_QCELP){
                       memcpy(p, dataPtr, QCELP_FRAME_SIZE);
                       p += QCELP_FRAME_SIZE;
                       count -= QCELP_FRAME_SIZE;
                       bytes += QCELP_FRAME_SIZE;
                       if(!mFirstread)
                       {
                          mFirstread = true;
                          break;
                       }
                    }
                }

            } else {
                if (errno != EAGAIN) return bytesRead;
                mRetryCount++;
                ALOGW("EAGAIN - retrying");
            }
        }
    }
    else if (mFormat == AUDIO_FORMAT_AAC)
    {
        *((uint32_t*)recogPtr) = 0x51434F4D ;// ('Q','C','O', 'M') Number to identify format as AAC by higher layers
        recogPtr++;
        frameCountPtr = (uint16_t*)recogPtr;
        *frameCountPtr = 0;
        p += 3*sizeof(uint16_t);
        count -= 3*sizeof(uint16_t);

        while (count > 0) {
            frameSizePtr = (uint16_t *)p;
            p += sizeof(uint16_t);
            if(!(count > 2)) break;
            count -= sizeof(uint16_t);

            ssize_t bytesRead = ::read(mFd, p, count);
            if (bytesRead > 0) {
                ALOGV("Number of Bytes read = %d", bytesRead);
                count -= bytesRead;
                p += bytesRead;
                bytes += bytesRead;
                ALOGV("Total Number of Bytes read = %d", bytes);

                *frameSizePtr =  bytesRead;
                (*frameCountPtr)++;
                if(!mFirstread)
                {
                   mFirstread = true;
                   break;
                }
                /*Typical frame size for AAC is around 250 bytes. So we have
                 * taken the minimum buffer size as twice of this size i.e.
                 * 512 to avoid short reads from driver */
                if(count < 512)
                {
                   ALOGI("buffer passed to driver %d, is less than the min 512 bytes", count);
                   break;
                }
            }
            else if(bytesRead == 0)
            {
             ALOGI("Bytes Read = %d ,Buffer no longer sufficient",bytesRead);
             break;
            } else {
                if (errno != EAGAIN) return bytesRead;
                mRetryCount++;
                ALOGW("EAGAIN - retrying");
            }
        }
    }

    if (mFormat == AUDIO_FORMAT_AAC)
         return aac_framesize;

        return bytes;
}

status_t AudioHardware::AudioStreamInMSM72xx::standby()
{
    bool isDriverClosed = false;
    ALOGD("AudioStreamInMSM72xx::standby()");
    Routing_table* temp = NULL;
    if (!mHardware) return -1;
    if (mState > AUDIO_INPUT_CLOSED) {
        if (mFd >= 0) {
            ::close(mFd);
            mFd = -1;
            ALOGV("driver closed");
            isDriverClosed = true;
        }
        //mHardware->checkMicMute();
        mState = AUDIO_INPUT_CLOSED;
    }
    if (mFmRec == FM_A2DP_REC) {
        //A2DP Recording
        temp = getNodeByStreamType(FM_A2DP);
        if(temp == NULL)
            return NO_ERROR;
        if(msm_route_stream(PCM_PLAY, temp->dec_id, DEV_ID(DEVICE_FMRADIO_STEREO_TX), 0)) {
           ALOGE("msm_route_stream failed");
           return 0;
        }
        deleteFromTable(FM_A2DP);
        if(enableDevice(DEVICE_FMRADIO_STEREO_TX, 0)) {
            ALOGE("Disabling device failed for device %d", DEVICE_FMRADIO_STEREO_TX);
        }
    }
    if (mFmRec == FM_FILE_REC) {
        //FM Recording
        temp = getNodeByStreamType(FM_REC);
        if(temp == NULL)
            return NO_ERROR;
        if(msm_route_stream(PCM_PLAY, temp->dec_id, DEV_ID(DEVICE_FMRADIO_STEREO_TX), 0)) {
           ALOGE("msm_route_stream failed");
           return 0;
        }
        deleteFromTable(FM_REC);
    }
    temp = getNodeByStreamType(PCM_REC);
    if(temp == NULL)
        return NO_ERROR;

    if(isDriverClosed){
        ALOGD("Deroute pcm in stream");
        if(msm_route_stream(PCM_REC, temp->dec_id,DEV_ID(temp->dev_id), 0)) {
            ALOGE("could not set stream routing\n");
            deleteFromTable(PCM_REC);
            return -1;
        }
        ALOGV("Disable device");
        deleteFromTable(PCM_REC);
        updateDeviceInfo(cur_rx, cur_tx);
    }//mRecordingSession condition.
    // restore output routing if necessary
    mHardware->clearCurDevice();
    mHardware->doRouting(this, 0);
    return NO_ERROR;
}

status_t AudioHardware::AudioStreamInMSM72xx::dump(int fd, const Vector<String16>& args)
{
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;
    result.append("AudioStreamInMSM72xx::dump\n");
    snprintf(buffer, SIZE, "\tsample rate: %d\n", sampleRate());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tbuffer size: %d\n", bufferSize());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tchannels: %d\n", channels());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tformat: %d\n", format());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmHardware: %p\n", mHardware);
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmFd count: %d\n", mFd);
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmState: %d\n", mState);
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmRetryCount: %d\n", mRetryCount);
    result.append(buffer);
    ::write(fd, result.string(), result.size());
    return NO_ERROR;
}

status_t AudioHardware::AudioStreamInMSM72xx::setParameters(const String8& keyValuePairs)
{
    AudioParameter param = AudioParameter(keyValuePairs);
    String8 key = String8(AudioParameter::keyRouting);
    status_t status = NO_ERROR;
    int device;
    ALOGV("AudioStreamInMSM72xx::setParameters() %s", keyValuePairs.string());

    if (param.getInt(key, device) == NO_ERROR) {
        ALOGV("set input routing %x", device);
        if (device & (device - 1)) {
            status = BAD_VALUE;
        } else {
            mDevices = device;
            status = mHardware->doRouting(this, device);
        }
        param.remove(key);
    }

    if (param.size()) {
        status = BAD_VALUE;
    }
    return status;
}

String8 AudioHardware::AudioStreamInMSM72xx::getParameters(const String8& keys)
{
    AudioParameter param = AudioParameter(keys);
    String8 value;
    String8 key = String8(AudioParameter::keyRouting);

    if (param.get(key, value) == NO_ERROR) {
        ALOGV("get routing %x", mDevices);
        param.addInt(key, (int)mDevices);
    }

    ALOGV("AudioStreamInMSM72xx::getParameters() %s", param.toString().string());
    return param.toString();
}

// getActiveInput_l() must be called with mLock held
AudioHardware::AudioStreamInMSM72xx *AudioHardware::getActiveInput_l()
{
    for (size_t i = 0; i < mInputs.size(); i++) {
        // return first input found not being in standby mode
        // as only one input can be in this state
        if (mInputs[i]->state() > AudioStreamInMSM72xx::AUDIO_INPUT_CLOSED) {
            return mInputs[i];
        }
    }

    return NULL;
}

AudioHardware::AudioSessionOutLPA::AudioSessionOutLPA( AudioHardware *hw,
                                         uint32_t   devices,
                                         int        format,
                                         uint32_t   channels,
                                         uint32_t   samplingRate,
                                         int        type,
                                         status_t   *status)
{
    Mutex::Autolock autoLock(mLock);
    mHardware = hw;
    ALOGE("AudioSessionOutLPA constructor");
    mFormat             = format;
    mSampleRate         = samplingRate;
    mChannels           = popcount(channels);
    mBufferSize         = LPA_BUFFER_SIZE; //TODO to check what value is correct
    *status             = BAD_VALUE;

    mPaused             = false;
    mIsDriverStarted    = false;
    mGenerateEOS        = true;
    mSeeking            = false;
    mReachedEOS         = false;
    mSkipWrite          = false;
    timeStarted = 0;
    timePlayed = 0;

    mInputBufferSize    = LPA_BUFFER_SIZE;
    mInputBufferCount   = BUFFER_COUNT;
    efd = -1;
    mEosEventReceived   =false;

    mEventThread        = NULL;
    mEventThreadAlive   = false;
    mKillEventThread    = false;
    mObserver           = NULL;
    if((format == AUDIO_FORMAT_PCM_16_BIT) && (mChannels == 0 || mChannels > 2)) {
        ALOGE("Invalid number of channels %d", channels);
        return;
    }

    mDevices = devices;

    *status = openAudioSessionDevice();

    if (*status == NO_ERROR)
        createEventThread();
}

AudioHardware::AudioSessionOutLPA::~AudioSessionOutLPA()
{
    ALOGV("AudioSessionOutLPA destructor");
    mSkipWrite = true;
    mWriteCv.signal();
    reset();

}

status_t AudioHardware::AudioSessionOutLPA::setParameters(const String8& keyValuePairs)
{
    AudioParameter param = AudioParameter(keyValuePairs);
    String8 key = String8(AudioParameter::keyRouting);
    status_t status = NO_ERROR;
    int device;
    ALOGV("AudioSessionOutLPA::setParameters() %s", keyValuePairs.string());

    if (param.getInt(key, device) == NO_ERROR) {
        mDevices = device;
        ALOGV("set output routing %x", mDevices);
        status = mHardware->doRouting(NULL, device);
        param.remove(key);
    }

    if (param.size()) {
        status = BAD_VALUE;
    }
    return status;
}
String8 AudioHardware::AudioSessionOutLPA::getParameters(const String8& keys)
{
    AudioParameter param = AudioParameter(keys);
    String8 value;
    String8 key = String8(AudioParameter::keyRouting);

    if (param.get(key, value) == NO_ERROR) {
        ALOGV("get routing %x", mDevices);
        param.addInt(key, (int)mDevices);
    }

    ALOGV("AudioSessionOutLPA::getParameters() %s", param.toString().string());
    return param.toString();
}

ssize_t AudioHardware::AudioSessionOutLPA::write(const void* buffer, size_t bytes)
{
    Mutex::Autolock autoLock(mLock);
    int err;
    ALOGV("write Empty Queue size() = %d, Filled Queue size() = %d ",
         mEmptyQueue.size(),mFilledQueue.size());

    if (mSkipWrite) {
        mSkipWrite = false;
        if (bytes < LPA_BUFFER_SIZE)
            bytes = 0;
        else
            return UNKNOWN_ERROR;
    }

    //2.) Dequeue the buffer from empty buffer queue. Copy the data to be
    //    written into the buffer. Then Enqueue the buffer to the filled
    //    buffer queue
    mEmptyQueueMutex.lock();
    List<BuffersAllocated>::iterator it = mEmptyQueue.begin();
    BuffersAllocated buf = *it;
    mEmptyQueue.erase(it);
    mEmptyQueueMutex.unlock();

    memset(buf.memBuf, 0, bytes);
    memcpy(buf.memBuf, buffer, bytes);
    buf.bytesToWrite = bytes;

    struct msm_audio_aio_buf aio_buf_local;
    if ( buf.bytesToWrite > 0) {
        memset(&aio_buf_local, 0, sizeof(msm_audio_aio_buf));
        aio_buf_local.buf_addr = buf.memBuf;
        aio_buf_local.buf_len = buf.bytesToWrite;
        aio_buf_local.data_len = buf.bytesToWrite;
        aio_buf_local.private_data = (void*) buf.memFd;

        if ( (buf.bytesToWrite % 2) != 0 ) {
            ALOGV("Increment for even bytes");
            aio_buf_local.data_len += 1;
        }
        if (timeStarted == 0)
            timeStarted = nanoseconds_to_microseconds(systemTime(SYSTEM_TIME_MONOTONIC));
    } else {
        /* Put the buffer back into requestQ */
        ALOGV("mEmptyQueueMutex locking: %d", __LINE__);
        mEmptyQueueMutex.lock();
        ALOGV("mEmptyQueueMutex locked: %d", __LINE__);
        mEmptyQueue.push_back(buf);
        ALOGV("mEmptyQueueMutex unlocking: %d", __LINE__);
        mEmptyQueueMutex.unlock();
        ALOGV("mEmptyQueueMutex unlocked: %d", __LINE__);
        //Post EOS in case the filled queue is empty and EOS is reached.
        mReachedEOS = true;
        mFilledQueueMutex.lock();
        if (mFilledQueue.empty() && !mEosEventReceived) {
            ALOGV("mEosEventReceived made true");
            mEosEventReceived = true;
            if (mObserver != NULL) {
                ALOGV("mObserver: posting EOS");
                mObserver->postEOS(0);
            }
        }
        mFilledQueueMutex.unlock();
        return NO_ERROR;
    }
    mFilledQueueMutex.lock();
    mFilledQueue.push_back(buf);
    mFilledQueueMutex.unlock();

    ALOGV("PCM write start");
    //3.) Write the buffer to the Driver
    if(mIsDriverStarted) {
       if (ioctl(afd, AUDIO_ASYNC_WRITE, &aio_buf_local) < 0 ) {
           ALOGE("error on async write\n");
       }
    }
    ALOGV("PCM write complete");

    if (bytes < LPA_BUFFER_SIZE) {
        ALOGV("Last buffer case");
        mReachedEOS = true;
        mLock.unlock();
        if (fsync(afd) != 0) {
            ALOGE("fsync failed.");
        }
        mLock.lock();
    }

    return NO_ERROR; //TODO Do wee need to send error
}


status_t AudioHardware::AudioSessionOutLPA::standby()
{
    ALOGD("AudioSessionOutLPA::standby()");
    status_t status = NO_ERROR;
    return status;
}


status_t AudioHardware::AudioSessionOutLPA::dump(int fd, const Vector<String16>& args)
{
    return NO_ERROR;
}
status_t AudioHardware::AudioSessionOutLPA::setVolume(float left, float right)
{
    int sessionId = 0;
    unsigned short decId;

    float v = (left + right) / 2;
    if (v < 0.0) {
        ALOGW("AudioSessionOutLPA::setVolume(%f) under 0.0, assuming 0.0\n", v);
        v = 0.0;
    } else if (v > 1.0) {
        ALOGW("AudioSessionOutLPA::setVolume(%f) over 1.0, assuming 1.0\n", v);
        v = 1.0;
    }

    if ( ioctl(afd, AUDIO_GET_SESSION_ID, &decId) == -1 ) {
        ALOGE("AUDIO_GET_SESSION_ID FAILED\n");
        return BAD_VALUE;
    } else {
        sessionId = (int)decId;
        ALOGE("AUDIO_GET_SESSION_ID success : decId = %d", decId);
    }

    // Ensure to convert the log volume back to linear for LPA
    float vol = v * 100;
    ALOGV("AudioSessionOutLPA::setVolume(%f)\n", v);
    ALOGV("Setting session volume to %f (available range is 0 to 100)\n", vol);

    if(msm_set_volume(sessionId, vol)) {
        ALOGE("msm_set_volume(%d %f) failed errno = %d",sessionId , vol,errno);
        return -1;
    }
    ALOGV("msm_set_volume(%f) succeeded",vol);
    return NO_ERROR;
}

status_t AudioHardware::AudioSessionOutLPA::openAudioSessionDevice( )
{
    status_t status = NO_ERROR;

    ALOGE("Opening LPA pcm_dec driver");
    afd = open("/dev/msm_pcm_lp_dec", O_WRONLY | O_NONBLOCK);
    if ( afd < 0 ) {
        ALOGE("pcm_lp_dec: cannot open pcm_dec device and the error is %d", errno);
        //initCheck = false;
        return UNKNOWN_ERROR;
    } else {
        //initCheck = true;
        ALOGV("pcm_lp_dec: pcm_lp_dec Driver opened");
    }

    start();
    bufferAlloc();

    return status;
}

void AudioHardware::AudioSessionOutLPA::bufferAlloc( )
{
    // Allocate ION buffers
    void *ion_buf; int32_t ion_fd;
    struct msm_audio_ion_info ion_info;
    //1. Open the ion_audio
    ionfd = open("/dev/ion", O_RDONLY | O_SYNC);
    if (ionfd < 0) {
        ALOGE("/dev/ion open failed \n");
        return;
    }
    for (int i = 0; i < mInputBufferCount; i++) {
        ion_buf = memBufferAlloc(mInputBufferSize, &ion_fd);
        memset(&ion_info, 0, sizeof(msm_audio_ion_info));
        ALOGE("Registering ION with fd %d and address as %p", ion_fd, ion_buf);
        ion_info.fd = ion_fd;
        ion_info.vaddr = ion_buf;
        if ( ioctl(afd, AUDIO_REGISTER_ION, &ion_info) < 0 ) {
            ALOGE("Registration of ION with the Driver failed with fd %d and memory %x",
                 ion_info.fd, (unsigned int)ion_info.vaddr);
        }
    }
}


void* AudioHardware::AudioSessionOutLPA::memBufferAlloc(int nSize, int32_t *ion_fd)
{
    void  *ion_buf = NULL;
    void  *local_buf = NULL;
    struct ion_fd_data fd_data;
    struct ion_allocation_data alloc_data;

    alloc_data.len =   nSize;
    alloc_data.align = 0x1000;
    alloc_data.heap_mask = ION_HEAP(ION_AUDIO_HEAP_ID);
    alloc_data.flags = 0;
    int rc = ioctl(ionfd, ION_IOC_ALLOC, &alloc_data);
    if (rc) {
        ALOGE("ION_IOC_ALLOC ioctl failed\n");
        return ion_buf;
    }
    fd_data.handle = alloc_data.handle;

    rc = ioctl(ionfd, ION_IOC_SHARE, &fd_data);
    if (rc) {
        ALOGE("ION_IOC_SHARE ioctl failed\n");
        rc = ioctl(ionfd, ION_IOC_FREE, &(alloc_data.handle));
        if (rc) {
            ALOGE("ION_IOC_FREE ioctl failed\n");
        }
        return ion_buf;
    }

    // 2. MMAP to get the virtual address
    ion_buf = mmap(NULL, nSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd_data.fd, 0);
    if(MAP_FAILED == ion_buf) {
        ALOGE("mmap() failed \n");
        close(fd_data.fd);
        rc = ioctl(ionfd, ION_IOC_FREE, &(alloc_data.handle));
        if (rc) {
            ALOGE("ION_IOC_FREE ioctl failed\n");
        }
        return ion_buf;
    }

    local_buf = malloc(nSize);
    if (NULL == local_buf) {
        // unmap the corresponding ION buffer and close the fd
        munmap(ion_buf, mInputBufferSize);
        close(fd_data.fd);
        rc = ioctl(ionfd, ION_IOC_FREE, &(alloc_data.handle));
        if (rc) {
            ALOGE("ION_IOC_FREE ioctl failed\n");
        }
        return NULL;
    }

    // 3. Store this information for internal mapping / maintanence
    BuffersAllocated buf(local_buf, ion_buf, nSize, fd_data.fd, alloc_data.handle);
    mEmptyQueue.push_back(buf);
    mBufPool.push_back(buf);

    // 4. Send the mem fd information
    *ion_fd = fd_data.fd;
    ALOGV("IONBufferAlloc calling with required size %d", nSize);
    ALOGV("ION allocated is %d, fd_data.fd %d and buffer is %x", *ion_fd, fd_data.fd, (unsigned int)ion_buf);

    // 5. Return the virtual address
    return ion_buf;
}

void AudioHardware::AudioSessionOutLPA::bufferDeAlloc()
{
    // De-Allocate ION buffers
    int rc = 0;
    //Remove all the buffers from empty queue
    mEmptyQueueMutex.lock();
    while (!mEmptyQueue.empty())  {
        List<BuffersAllocated>::iterator it = mEmptyQueue.begin();
        BuffersAllocated &ionBuffer = *it;
        struct msm_audio_ion_info ion_info;
        ion_info.vaddr = (*it).memBuf;
        ion_info.fd = (*it).memFd;
        if (ioctl(afd, AUDIO_DEREGISTER_ION, &ion_info) < 0) {
            ALOGE("ION deregister failed");
        }
        ALOGV("Ion Unmapping the address %p, size %d, fd %d from empty",ionBuffer.memBuf,ionBuffer.bytesToWrite,ionBuffer.memFd);
        munmap(ionBuffer.memBuf, mInputBufferSize);
        ALOGV("closing the ion shared fd");
        close(ionBuffer.memFd);
        rc = ioctl(ionfd, ION_IOC_FREE, &ionBuffer.ion_handle);
        if (rc) {
            ALOGE("ION_IOC_FREE ioctl failed\n");
        }
        // free the local buffer corresponding to ion buffer
        free(ionBuffer.localBuf);
        ALOGD("Removing from empty Q");
        mEmptyQueue.erase(it);
    }
    mEmptyQueueMutex.unlock();

    //Remove all the buffers from Filled queue
    mFilledQueueMutex.lock();
    while(!mFilledQueue.empty()){
        List<BuffersAllocated>::iterator it = mFilledQueue.begin();
        BuffersAllocated &ionBuffer = *it;
        struct msm_audio_ion_info ion_info;
        ion_info.vaddr = (*it).memBuf;
        ion_info.fd = (*it).memFd;
        if (ioctl(afd, AUDIO_DEREGISTER_ION, &ion_info) < 0) {
            ALOGE("ION deregister failed");
        }
        ALOGV("Ion Unmapping the address %p, size %d, fd %d from Request",ionBuffer.memBuf,ionBuffer.bytesToWrite,ionBuffer.memFd);
        munmap(ionBuffer.memBuf, mInputBufferSize);
        ALOGV("closing the ion shared fd");
        close(ionBuffer.memFd);
        rc = ioctl(ionfd, ION_IOC_FREE, &ionBuffer.ion_handle);
        if (rc) {
            ALOGE("ION_IOC_FREE ioctl failed\n");
        }
        // free the local buffer corresponding to ion buffer
        free(ionBuffer.localBuf);
        ALOGD("Removing from Filled Q");
        mFilledQueue.erase(it);
    }
    mFilledQueueMutex.unlock();
    while (!mBufPool.empty()) {
        List<BuffersAllocated>::iterator it = mBufPool.begin();
        ALOGE("Removing input buffer from Buffer Pool ");
        mBufPool.erase(it);
    }
    if (ionfd >= 0) {
        close(ionfd);
        ionfd = -1;
    }
}

uint32_t AudioHardware::AudioSessionOutLPA::latency() const
{
    return 54; //latency equal to regular hpcm session
}

void AudioHardware::AudioSessionOutLPA::requestAndWaitForEventThreadExit()
{
    if (!mEventThreadAlive)
        return;
    mKillEventThread = true;
    if (ioctl(afd, AUDIO_ABORT_GET_EVENT, 0) < 0) {
        ALOGE("Audio Abort event failed");
    }
    pthread_join(mEventThread,NULL);
}

void * AudioHardware::AudioSessionOutLPA::eventThreadWrapper(void *me)
{
    static_cast<AudioSessionOutLPA *>(me)->eventThreadEntry();
    return NULL;
}

void  AudioHardware::AudioSessionOutLPA::eventThreadEntry()
{
    struct msm_audio_event cur_pcmdec_event;
    mEventThreadAlive = true;
    int rc = 0;
    //2.) Set the priority for the event thread
    pid_t tid  = gettid();
    androidSetThreadPriority(tid, ANDROID_PRIORITY_AUDIO);
    prctl(PR_SET_NAME, (unsigned long)"HAL Audio EventThread", 0, 0, 0);
    ALOGV("event thread created ");
    if (mKillEventThread) {
        mEventThreadAlive = false;
        ALOGV("Event Thread is dying.");
        return;
    }
    while (1) {
        //Wait for an event to occur
        rc = ioctl(afd, AUDIO_GET_EVENT, &cur_pcmdec_event);
        ALOGE("pcm dec Event Thread rc = %d and errno is %d",rc, errno);

        if ( (rc < 0) && ((errno == ENODEV) || (errno == EBADF)) ) {
            ALOGV("AUDIO__GET_EVENT called. Exit the thread");
            break;
        }

        switch ( cur_pcmdec_event.event_type ) {
        case AUDIO_EVENT_WRITE_DONE:
            {
                Mutex::Autolock autoLock(mLock);
                ALOGE("WRITE_DONE: addr %p len %d and fd is %d\n",
                     cur_pcmdec_event.event_payload.aio_buf.buf_addr,
                     cur_pcmdec_event.event_payload.aio_buf.data_len,
                     (int32_t) cur_pcmdec_event.event_payload.aio_buf.private_data);
                mFilledQueueMutex.lock();
                BuffersAllocated buf = *(mFilledQueue.begin());
                for (List<BuffersAllocated>::iterator it = mFilledQueue.begin();
                    it != mFilledQueue.end(); ++it) {
                    if (it->memBuf == cur_pcmdec_event.event_payload.aio_buf.buf_addr) {
                        buf = *it;
                        mFilledQueue.erase(it);
                        // Post buffer to Empty Q
                        ALOGV("mEmptyQueueMutex locking: %d", __LINE__);
                        mEmptyQueueMutex.lock();
                        ALOGV("mEmptyQueueMutex locked: %d", __LINE__);
                        mEmptyQueue.push_back(buf);
                        ALOGV("mEmptyQueueMutex unlocking: %d", __LINE__);
                        mEmptyQueueMutex.unlock();
                        ALOGV("mEmptyQueueMutex unlocked: %d", __LINE__);
                        if (mFilledQueue.empty() && mReachedEOS && mGenerateEOS) {
                            ALOGV("Posting the EOS to the observer player %p", mObserver);
                            mEosEventReceived = true;
                            if (mObserver != NULL) {
                                ALOGV("mObserver: posting EOS");
                                mObserver->postEOS(0);
                            }
                        }
                        break;
                    }
                }
                mFilledQueueMutex.unlock();
                 mWriteCv.signal();
            }
            break;
        case AUDIO_EVENT_SUSPEND:
            {
                struct msm_audio_stats stats;
                int nBytesConsumed = 0;

                ALOGV("AUDIO_EVENT_SUSPEND received\n");
                if (!mPaused) {
                    ALOGV("Not in paused, no need to honor SUSPEND event");
                    break;
                }
                // 1. Get the Byte count that is consumed
                if ( ioctl(afd, AUDIO_GET_STATS, &stats)  < 0 ) {
                    ALOGE("AUDIO_GET_STATUS failed");
                } else {
                    ALOGV("Number of bytes consumed by DSP is %u", stats.byte_count);
                    nBytesConsumed = stats.byte_count;
                    }
                    // Reset eosflag to resume playback where we actually paused
                    mReachedEOS = false;
                    // 3. Call AUDIO_STOP on the Driver.
                    ALOGV("Received AUDIO_EVENT_SUSPEND and calling AUDIO_STOP");
                    if ( ioctl(afd, AUDIO_STOP, 0) < 0 ) {
                         ALOGE("AUDIO_STOP failed");
                    }
                    mIsDriverStarted = false;
                    break;
            }
            break;
        case AUDIO_EVENT_RESUME:
            {
                ALOGV("AUDIO_EVENT_RESUME received\n");
            }
            break;
        default:
            ALOGE("Received Invalid Event from driver\n");
            break;
        }
    }
    mEventThreadAlive = false;
    ALOGV("Event Thread is dying.");
}


void AudioHardware::AudioSessionOutLPA::createEventThread()
{
    ALOGV("Creating Event Thread");
    mKillEventThread = false;
    mEventThreadAlive = true;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_create(&mEventThread, &attr, eventThreadWrapper, this);
    ALOGV("Event Thread created");
}

status_t AudioHardware::AudioSessionOutLPA::start( )
{

    ALOGV("LPA playback start");
    if (mPaused && mIsDriverStarted) {
        mPaused = false;
        if (ioctl(afd, AUDIO_PAUSE, 0) < 0) {
            ALOGE("Resume:: LPA driver resume failed");
            return UNKNOWN_ERROR;
        }
    } else {
    int sessionId = 0;
        mPaused = false;
    if ( afd >= 0 ) {
        struct msm_audio_config config;
        if ( ioctl(afd, AUDIO_GET_CONFIG, &config) < 0 ) {
            ALOGE("could not get config");
            close(afd);
            afd = -1;
            return BAD_VALUE;
        }

        config.sample_rate = mSampleRate;
        config.channel_count = mChannels;
        ALOGV("sample_rate=%d and channel count=%d \n", mSampleRate, mChannels);
        if ( ioctl(afd, AUDIO_SET_CONFIG, &config) < 0 ) {
            ALOGE("could not set config");
            close(afd);
            afd = -1;
            return BAD_VALUE;
        }
    }

    unsigned short decId;
    if ( ioctl(afd, AUDIO_GET_SESSION_ID, &decId) == -1 ) {
        ALOGE("AUDIO_GET_SESSION_ID FAILED\n");
        return BAD_VALUE;
    } else {
        sessionId = (int)decId;
        ALOGV("AUDIO_GET_SESSION_ID success : decId = %d", decId);
    }

    Mutex::Autolock lock(mDeviceSwitchLock);
    if(getNodeByStreamType(LPA_DECODE) != NULL) {
        ALOGE("Not allowed, There is alread an LPA Node existing");
        return -1;
    }
    ALOGE("AudioSessionOutMSM7x30::set() Adding LPA_DECODE Node to Table");
    addToTable(sessionId,cur_rx,INVALID_DEVICE,LPA_DECODE,true);
    ALOGE("enableDevice(cur_rx = %d, dev_id = %d)",cur_rx,DEV_ID(cur_rx));
    if (enableDevice(cur_rx, 1)) {
        ALOGE("enableDevice failed for device cur_rx %d", cur_rx);
        return -1;
    }

    ALOGE("msm_route_stream(PCM_PLAY,%d,%d,0)",sessionId,DEV_ID(cur_rx));
    if(msm_route_stream(PCM_PLAY,sessionId,DEV_ID(cur_rx),1)) {
        ALOGE("msm_route_stream(PCM_PLAY,%d,%d,1) failed",sessionId,DEV_ID(cur_rx));
        return -1;
    }

    Mutex::Autolock lock_1(mComboDeviceLock);
    if(CurrentComboDeviceData.DeviceId == SND_DEVICE_FM_TX_AND_SPEAKER){
        ALOGD("Routing LPA steam to speaker for combo device");
        ALOGD("combo:msm_route_stream(LPA_DECODE,session id:%d,dev id:%d,1)",sessionId,
            DEV_ID(DEVICE_SPEAKER_RX));
            /* music session is already routed to speaker in
             * enableComboDevice(), but at this point it can
             * be said that it was done with incorrect session id,
             * so re-routing with correct session id here.
             */
        if(msm_route_stream(PCM_PLAY, sessionId, DEV_ID(DEVICE_SPEAKER_RX),
           1)) {
            ALOGE("msm_route_stream failed");
            return -1;
        }
        CurrentComboDeviceData.StreamType = LPA_DECODE;
    }

    //Start the Driver
    if (ioctl(afd, AUDIO_START,0) < 0) {
        ALOGE("Driver start failed!");
        return BAD_VALUE;
    }
    mIsDriverStarted = true;
    if (timeStarted == 0)
        timeStarted = nanoseconds_to_microseconds(systemTime(SYSTEM_TIME_MONOTONIC));// Needed
    }
    return NO_ERROR;
}

status_t AudioHardware::AudioSessionOutLPA::pause()
{
    ALOGV("LPA playback pause");
    if (ioctl(afd, AUDIO_PAUSE, 1) < 0) {
    ALOGE("Audio Pause failed");
    }
    mPaused = true;
    timePlayed += (nanoseconds_to_microseconds(systemTime(SYSTEM_TIME_MONOTONIC)) - timeStarted);//needed
    return NO_ERROR;
}

status_t AudioHardware::AudioSessionOutLPA::drain()
{
    ALOGV("LPA playback EOS");
    return NO_ERROR;
}

status_t AudioHardware::AudioSessionOutLPA::flush()
{
    Mutex::Autolock autoLock(mLock);
    ALOGV("LPA playback flush ");
    int err;

    // 2.) Add all the available buffers to Empty Queue (Maintain order)
    mFilledQueueMutex.lock();
    mEmptyQueueMutex.lock();
    // 1.) Clear the Empty and Filled buffer queue
    mEmptyQueue.clear();
    mFilledQueue.clear();
    // 2.) Add all the available buffers to Empty Queue (Maintain order)
    List<BuffersAllocated>::iterator it = mBufPool.begin();
    for (; it!=mBufPool.end(); ++it) {
       memset(it->memBuf, 0x0, (*it).memBufsize);
       mEmptyQueue.push_back(*it);
    }
    mEmptyQueueMutex.unlock();
    mFilledQueueMutex.unlock();
    ALOGV("Transferred all the buffers from Filled queue to "
          "Empty queue to handle seek");
    ALOGV("mPaused %d mEosEventReceived %d", mPaused, mEosEventReceived);
    mReachedEOS = false;
    if (!mPaused) {
        if (!mEosEventReceived) {
            if (ioctl(afd, AUDIO_PAUSE, 1) < 0) {
                ALOGE("Audio Pause failed");
                return UNKNOWN_ERROR;
            }
            mSkipWrite = true;
            if (ioctl(afd, AUDIO_FLUSH, 0) < 0) {
                ALOGE("Audio Flush failed");
                return UNKNOWN_ERROR;
            }
        }
    } else {
        timeStarted = 0;
        mSkipWrite = true;
        if (ioctl(afd, AUDIO_FLUSH, 0) < 0) {
            ALOGE("Audio Flush failed");
            return UNKNOWN_ERROR;
        }
        if (ioctl(afd, AUDIO_PAUSE, 1) < 0) {
            ALOGE("Audio Pause failed");
            return UNKNOWN_ERROR;
        }
    }
    mEosEventReceived = false;
    //4.) Skip the current write from the decoder and signal to the Write get
    //   the next set of data from the decoder
    mWriteCv.signal();
    return NO_ERROR;
}
status_t AudioHardware::AudioSessionOutLPA::stop()
{
    Mutex::Autolock autoLock(mLock);
    ALOGV("AudioSessionOutLPA- stop");
    // close all the existing PCM devices
    mSkipWrite = true;
    mWriteCv.signal();
    return NO_ERROR;
}

status_t AudioHardware::AudioSessionOutLPA::setObserver(void *observer)
{
    ALOGV("Registering the callback \n");
    mObserver = reinterpret_cast<AudioEventObserver *>(observer);
    return NO_ERROR;
}

status_t  AudioHardware::AudioSessionOutLPA::getNextWriteTimestamp(int64_t *timestamp)
{

    *timestamp = nanoseconds_to_microseconds(systemTime(SYSTEM_TIME_MONOTONIC)) - timeStarted + timePlayed;//needed
    ALOGV("Timestamp returned = %lld\n", *timestamp);
    return NO_ERROR;
}

void AudioHardware::AudioSessionOutLPA::reset()
{
    Routing_table* temp = NULL;
    ALOGD("AudioSessionOutLPA::reset()");
    status_t status = NO_ERROR;

    temp = getNodeByStreamType(LPA_DECODE);

    if(temp == NULL)
        return ;

    ALOGD("Deroute lpa playback stream");
    if(msm_route_stream(PCM_PLAY, temp->dec_id,DEV_ID(temp->dev_id), 0)) {
        ALOGE("could not set stream routing\n");
        deleteFromTable(LPA_DECODE);
        return ;
    }
    deleteFromTable(LPA_DECODE);
    updateDeviceInfo(cur_rx, cur_tx);
    mGenerateEOS = false;

    ioctl(afd,AUDIO_STOP,0);
    mIsDriverStarted = false;
    requestAndWaitForEventThreadExit();
    bufferDeAlloc();
    ::close(afd);
}
status_t AudioHardware::AudioSessionOutLPA::getRenderPosition(uint32_t *dspFrames)
{
    //TODO: enable when supported by driver
    return INVALID_OPERATION;
}

status_t AudioHardware::setupDeviceforVoipCall(bool value)
{

    int mode = (value ? AUDIO_MODE_IN_COMMUNICATION : AUDIO_MODE_NORMAL);
    if (setMode(mode) != NO_ERROR) {
        ALOGV("setMode fails");
        return UNKNOWN_ERROR;
    }

    if (setMicMute(!value) != NO_ERROR) {
        ALOGV("MicMute fails");
        return UNKNOWN_ERROR;
    }

    ALOGD("Device setup sucess for VOIP call");

    return NO_ERROR;
}


status_t AudioHardware::AudioSessionOutLPA::getBufferInfo(buf_info **buf) {

    buf_info *tempbuf = (buf_info *)malloc(sizeof(buf_info) + mInputBufferCount*sizeof(int *));
    ALOGV("Get buffer info");
    tempbuf->bufsize = LPA_BUFFER_SIZE;
    tempbuf->nBufs = mInputBufferCount;
    tempbuf->buffers = (int **)((char*)tempbuf + sizeof(buf_info));
    List<BuffersAllocated>::iterator it = mBufPool.begin();
    for (int i = 0; i < mInputBufferCount; i++) {
        tempbuf->buffers[i] = (int *)it->memBuf;
        it++;
    }
    *buf = tempbuf;
    return NO_ERROR;
}

status_t AudioHardware::AudioSessionOutLPA::isBufferAvailable(int *isAvail) {

    Mutex::Autolock autoLock(mLock);
    ALOGV("isBufferAvailable Empty Queue size() = %d, Filled Queue size() = %d ",
          mEmptyQueue.size(),mFilledQueue.size());
    *isAvail = false;
    // 1.) Wait till a empty buffer is available in the Empty buffer queue
    mEmptyQueueMutex.lock();
    if (mEmptyQueue.empty()) {
        ALOGV("Write: waiting on mWriteCv");
        mLock.unlock();
        mWriteCv.wait(mEmptyQueueMutex);
        mEmptyQueueMutex.unlock();
        mLock.lock();
        if (mSkipWrite) {
            ALOGV("Write: Flushing the previous write buffer");
            mSkipWrite = false;
            return NO_ERROR;
        }
        ALOGV("isBufferAvailable: received a signal to wake up");
    }else {
        ALOGV("Buffer available in empty queue");
        mEmptyQueueMutex.unlock();
        }

    *isAvail = true;
    return NO_ERROR;
}

AudioHardware::AudioStreamInVoip::AudioStreamInVoip() :
    mHardware(0), mFd(-1), mState(AUDIO_INPUT_CLOSED), mRetryCount(0),
    mFormat(AUDIO_HW_IN_FORMAT), mChannels(AUDIO_HW_IN_CHANNELS),
    mSampleRate(AUDIO_HW_VOIP_SAMPLERATE_8K), mBufferSize(AUDIO_HW_VOIP_BUFFERSIZE_8K),
    mAcoustics((AudioSystem::audio_in_acoustics)0), mDevices(0)
{
}

status_t AudioHardware::AudioStreamInVoip::set(
        AudioHardware* hw, uint32_t devices, int *pFormat, uint32_t *pChannels, uint32_t *pRate,
        AudioSystem::audio_in_acoustics acoustic_flags)
{
    ALOGV(" AudioHardware::AudioStreamInVoip::set devices = %u format = %d pChannels = %u Rate = %u \n",
         devices, *pFormat, *pChannels, *pRate);
    ALOGV("AudioStreamInVoip cur_rx %d, cur_tx %d" ,cur_rx,cur_tx);
    if (pFormat == 0 ||*pFormat != AUDIO_HW_IN_FORMAT)
    {
        *pFormat = AUDIO_HW_IN_FORMAT;
        return BAD_VALUE;
    }

    if (pRate == 0) {
        return BAD_VALUE;
    }

    uint32_t rate = hw->getInputSampleRate(*pRate);
    if (rate != *pRate) {
        *pRate = rate;
        ALOGE(" sample rate does not match\n");
        return BAD_VALUE;
    }

    if (pChannels == 0 || (*pChannels & (AUDIO_CHANNEL_IN_MONO)) == 0) {
        *pChannels = AUDIO_HW_IN_CHANNELS;
        ALOGE(" Channle count does not match\n");
        return BAD_VALUE;
    }

    mHardware = hw;

    ALOGV("AudioStreamInVoip::set(%d, %d, %u)", *pFormat, *pChannels, *pRate);
    if (mFd >= 0) {
        ALOGE("Audio record already open");
        return -EPERM;
    }

    status_t status = NO_INIT;
    // open driver
    ALOGV("Check if driver is open");
    if(mHardware->mVoipFd >= 0) {
        mFd = mHardware->mVoipFd;
    }
    else {

       ALOGV("Going to enable RX/TX device for voice stream");
        // Routing Voice
        if ( (cur_rx != INVALID_DEVICE) && (cur_tx != INVALID_DEVICE))
        {
            ALOGV("Starting voice on Rx %d and Tx %d device", DEV_ID(cur_rx), DEV_ID(cur_tx));
            msm_route_voice(DEV_ID(cur_rx),DEV_ID(cur_tx), 1);
        }
        else
        {
            return -1;
        }

        if(cur_rx != INVALID_DEVICE && (enableDevice(cur_rx,1) == -1))
        {
            ALOGI(" Enable device for cur_rx failed \n");
            return -1;
        }

        if(cur_tx != INVALID_DEVICE&&(enableDevice(cur_tx,1) == -1))
        {
           ALOGE(" Enable device for cur_tx failed \n");
           return -1;
        }

        // start Voice call
        ALOGD("Starting voice call and UnMuting the call");
        msm_start_voice();
        msm_set_voice_tx_mute(0);
        addToTable(0,cur_rx,cur_tx,VOICE_CALL,true);

        ALOGE("open mvs driver");
        status = ::open(MVS_DEVICE, /*O_WRONLY*/ O_RDWR);
        if (status < 0) {
            ALOGE("Cannot open %s errno: %d",MVS_DEVICE, errno);
            goto Error;
        }

        mFd = status;
        ALOGE("VOPIstreamin : Save the fd \n");

        // configuration
        ALOGV("get mvs config");
        struct msm_audio_mvs_config mvs_config;
        status = ioctl(mFd, AUDIO_GET_MVS_CONFIG, &mvs_config);
        if (status < 0) {
            ALOGE("Cannot read mvs config");
            goto Error;
        }

        ALOGV("set mvs config");
        mvs_config.mvs_mode = MVS_MODE_PCM;
        status = ioctl(mFd, AUDIO_SET_MVS_CONFIG, &mvs_config);
        if (status < 0) {
            ALOGE("Cannot set mvs config");
           goto Error;
        }

        ALOGV("start mvs");
        status = ioctl(mFd, AUDIO_START, 0);
         if (status < 0) {
            ALOGE("Cannot start mvs driver");
            goto Error;
        }
    }
    mFormat =  *pFormat;
    mChannels = *pChannels;
    mSampleRate = *pRate;
    if(mSampleRate == AUDIO_HW_VOIP_SAMPLERATE_8K) {
        mBufferSize = AUDIO_HW_VOIP_BUFFERSIZE_8K;
    } else if(mSampleRate == AUDIO_HW_VOIP_SAMPLERATE_16K) {
        mBufferSize = AUDIO_HW_VOIP_BUFFERSIZE_16K;
    } else {
        ALOGE("AudioStreamInVoip::set return bad values\n");
        return BAD_VALUE;
    }

    ALOGV(" AudioHardware::AudioStreamInVoip::set after configuring devices = %u format = %d pChannels = %u Rate = %u \n",
         devices, mFormat, mChannels, mSampleRate);

    ALOGV(" Set state  AUDIO_INPUT_OPENED\n");
    mState = AUDIO_INPUT_OPENED;

    ALOGV(" Set mVoipFd now\n");
    mHardware->mVoipFd = mFd;
    mHardware->mVoipInActive = true;

    if (mHardware->mVoipOutActive)
        mHardware->setupDeviceforVoipCall(true);

    if (!acoustic)
        return NO_ERROR;

     return NO_ERROR;

Error:
    if (mFd >= 0) {
        ::close(mFd);
        mFd = -1;
        mHardware->mVoipFd = -1;
    }
    ALOGE("Error : ret status \n");
    return status;
}

AudioHardware::AudioStreamInVoip::~AudioStreamInVoip()
{
    ALOGV("AudioStreamInVoip destructor");
    mHardware->mVoipInActive = false;
    standby();
}

ssize_t AudioHardware::AudioStreamInVoip::read( void* buffer, ssize_t bytes)
{
    unsigned short dec_id = INVALID_DEVICE;
    ALOGV("AudioStreamInVoip::read(%p, %ld)", buffer, bytes);
    if (!mHardware) return -1;

    size_t count = bytes;
    size_t totalBytesRead = 0;

    if (mState < AUDIO_INPUT_OPENED) {
       ALOGE(" reopen the device \n");
        AudioHardware *hw = mHardware;
        hw->mLock.lock();
        status_t status = set(hw, mDevices, &mFormat, &mChannels, &mSampleRate, mAcoustics);
        if (status != NO_ERROR) {
            hw->mLock.unlock();
            return -1;
        }
        hw->mLock.unlock();
        mState = AUDIO_INPUT_STARTED;
        bytes = 0;
    }else
      ALOGV("AudioStreamInVoip::read : device is already open \n");


    if(mFormat == AUDIO_HW_IN_FORMAT)
    {
        if(count < mBufferSize) {
            ALOGE("read:: read size requested is less than min input buffer size");
            return 0;
        }

        struct q5v2_msm_audio_mvs_frame audio_mvs_frame;
        audio_mvs_frame.frame_type = 0;
        while (count >= mBufferSize) {
            audio_mvs_frame.len = mBufferSize;
            ALOGV("Calling read count = %u mBufferSize = %u\n",count, mBufferSize );
            int bytesRead = ::read(mFd, &audio_mvs_frame, sizeof(audio_mvs_frame));
            ALOGV("read_bytes = %d mvs bytes \n", bytesRead);
            if (bytesRead > 0) {
                memcpy(buffer+totalBytesRead,&audio_mvs_frame.voc_pkt, mBufferSize);
                count -= mBufferSize;
                totalBytesRead += mBufferSize;
                if(!mFirstread){
                   mFirstread = true;
                   break;
                }
            } else {
                ALOGE("retry read count = %d buffersize = %d\n", count, mBufferSize);
                if (errno != EAGAIN) return bytesRead;
                mRetryCount++;
                ALOGW("EAGAIN - retrying");
            }
        }
    }
    return totalBytesRead;
}

status_t AudioHardware::AudioStreamInVoip::standby()
{
    Routing_table* temp = NULL;
    ALOGD("AudioStreamInVoip::standby");
    Mutex::Autolock lock(mHardware->mVoipLock);
    if (!mHardware) return -1;
    ALOGE("VoipOut %d driver fd %d", mHardware->mVoipOutActive, mHardware->mVoipFd);
    mHardware->mVoipInActive = false;
    if (mState > AUDIO_INPUT_CLOSED && !mHardware->mVoipOutActive) {
         ALOGE(" closing mvs driver\n");
         //Mute and disable the device.
         int ret = msm_set_voice_rx_vol(0);
         if (ret <0)
            ALOGE("Error %d setting volume\n", ret);
         ret = msm_set_voice_tx_mute(1);
         if (ret < 0)
            ALOGE("Error %d setting mute\n", ret);
         ret = msm_end_voice();
         if (ret < 0)
            ALOGE("Error %d ending voice\n", ret);
         if (ret < 0)
            ALOGE("Error %d closing mixer\n", ret);
         if (mHardware->mVoipFd >= 0) {
            ret = ioctl(mHardware->mVoipFd, AUDIO_STOP, NULL);
            ALOGD("MVS stop returned %d %d %d\n", ret, __LINE__, mHardware->mVoipFd);
            ::close(mFd);
            mFd = mHardware->mVoipFd = -1;
            mHardware->setupDeviceforVoipCall(false);
            ALOGD("MVS driver closed %d mFd %d", __LINE__, mHardware->mVoipFd);
        }
        mState = AUDIO_INPUT_CLOSED;
    }
    else
        ALOGE("Not closing MVS driver");
    return NO_ERROR;
}

status_t AudioHardware::AudioStreamInVoip::dump(int fd, const Vector<String16>& args)
{
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;
    result.append("AudioStreamInVoip::dump\n");
    snprintf(buffer, SIZE, "\tsample rate: %d\n", sampleRate());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tbuffer size: %d\n", bufferSize());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tchannels: %d\n", channels());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tformat: %d\n", format());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmHardware: %p\n", mHardware);
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmFd count: %d\n", mFd);
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmState: %d\n", mState);
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmRetryCount: %d\n", mRetryCount);
    result.append(buffer);
    ::write(fd, result.string(), result.size());
    return NO_ERROR;
}

status_t AudioHardware::AudioStreamInVoip::setParameters(const String8& keyValuePairs)
{
    AudioParameter param = AudioParameter(keyValuePairs);
    String8 key = String8(AudioParameter::keyRouting);
    status_t status = NO_ERROR;
    int device;
    ALOGV("AudioStreamInVoip::setParameters() %s", keyValuePairs.string());

    if (param.getInt(key, device) == NO_ERROR) {
        ALOGV("set input routing %x", device);
        if (device & (device - 1)) {
            ALOGV(" return BAD_VALUE ");
            status = BAD_VALUE;
        } else {
            mDevices = device;
            status = mHardware->doRouting(this, device);
        }
        param.remove(key);
    }

    if (param.size()) {
        status = BAD_VALUE;
    }

    return status;
}

String8 AudioHardware::AudioStreamInVoip::getParameters(const String8& keys)
{
    AudioParameter param = AudioParameter(keys);
    String8 value;
    String8 key = String8(AudioParameter::keyRouting);

    if (param.get(key, value) == NO_ERROR) {
        ALOGV("get routing %x", mDevices);
       param.addInt(key, (int)mDevices);
    }

    ALOGV("AudioStreamInVoip::getParameters() %s", param.toString().string());
    return param.toString();
}

// getActiveInput_l() must be called with mLock held
AudioHardware::AudioStreamInVoip*AudioHardware::getActiveVoipInput_l()
{
    for (size_t i = 0; i < mVoipInputs.size(); i++) {
        // return first input found not being in standby mode
        // as only one input can be in this state
        if (mVoipInputs[i]->state() > AudioStreamInVoip::AUDIO_INPUT_CLOSED) {
            return mVoipInputs[i];
        }
    }

    return NULL;
}


// ----------------------------------------------------------------------------

extern "C" AudioHardwareInterface* createAudioHardware(void) {
    return new AudioHardware();
}

/*===========================================================================

FUNCTION amrsup_frame_len

DESCRIPTION
  This function will determine number of bytes of AMR vocoder frame length
based on the frame type and frame rate.

DEPENDENCIES
  None.

RETURN VALUE
  number of bytes of AMR frame

SIDE EFFECTS
  None.

===========================================================================*/
int amrsup_frame_len_bits(
  amrsup_frame_type frame_type,
  amrsup_mode_type amr_mode
)
{
  int frame_len=0;


  switch (frame_type)
  {
    case AMRSUP_SPEECH_GOOD :
    case AMRSUP_SPEECH_DEGRADED :
    case AMRSUP_ONSET :
    case AMRSUP_SPEECH_BAD :
      if (amr_mode >= AMRSUP_MODE_MAX)
      {
        frame_len = 0;
      }
      else
      {
        frame_len = amrsup_122_framing.len_a
                    + amrsup_122_framing.len_b
                    + amrsup_122_framing.len_c;
      }
      break;

    case AMRSUP_SID_FIRST :
    case AMRSUP_SID_UPDATE :
    case AMRSUP_SID_BAD :
      frame_len = AMR_CLASS_A_BITS_SID;
      break;

    case AMRSUP_NO_DATA :
    case AMRSUP_SPEECH_LOST :
    default :
      frame_len = 0;
  }

  return frame_len;
}

/*===========================================================================

FUNCTION amrsup_frame_len

DESCRIPTION
  This function will determine number of bytes of AMR vocoder frame length
based on the frame type and frame rate.

DEPENDENCIES
  None.

RETURN VALUE
  number of bytes of AMR frame

SIDE EFFECTS
  None.

===========================================================================*/
int amrsup_frame_len(
  amrsup_frame_type frame_type,
  amrsup_mode_type amr_mode
)
{
  int frame_len = amrsup_frame_len_bits(frame_type, amr_mode);

  frame_len = (frame_len + 7) / 8;
  return frame_len;
}

/*===========================================================================

FUNCTION amrsup_tx_order

DESCRIPTION
  Use a bit ordering table to order bits from their original sequence.

DEPENDENCIES
  None.

RETURN VALUE
  None.

SIDE EFFECTS
  None.

===========================================================================*/
void amrsup_tx_order(
  unsigned char *dst_frame,
  int         *dst_bit_index,
  unsigned char *src,
  int         num_bits,
  const unsigned short *order
)
{
  unsigned long dst_mask = 0x00000080 >> ((*dst_bit_index) & 0x7);
  unsigned char *dst = &dst_frame[((unsigned int) *dst_bit_index) >> 3];
  unsigned long src_bit, src_mask;

  /* Prepare to process all bits
  */
  *dst_bit_index += num_bits;
  num_bits++;

  while(--num_bits) {
    /* Get the location of the bit in the input buffer */
    src_bit  = (unsigned long ) *order++;
    src_mask = 0x00000080 >> (src_bit & 0x7);

    /* Set the value of the output bit equal to the input bit */
    if (src[src_bit >> 3] & src_mask) {
      *dst |= (unsigned char ) dst_mask;
    }

    /* Set the destination bit mask and increment pointer if necessary */
    dst_mask >>= 1;
    if (dst_mask == 0) {
      dst_mask = 0x00000080;
      dst++;
    }
  }
} /* amrsup_tx_order */

/*===========================================================================

FUNCTION amrsup_if1_framing

DESCRIPTION
  Performs the transmit side framing function.  Generates AMR IF1 ordered data
  from the vocoder packet and frame type.

DEPENDENCIES
  None.

RETURN VALUE
  number of bytes of encoded frame.
  if1_frame : IF1-encoded frame.
  if1_frame_info : holds frame information of IF1-encoded frame.

SIDE EFFECTS
  None.

===========================================================================*/
static int amrsup_if1_framing(
  unsigned char              *vocoder_packet,
  amrsup_frame_type          frame_type,
  amrsup_mode_type           amr_mode,
  unsigned char              *if1_frame,
  amrsup_if1_frame_info_type *if1_frame_info
)
{
  amrsup_frame_order_type *ordering_table;
  int frame_len = 0;
  int i;

  if(amr_mode >= AMRSUP_MODE_MAX)
  {
    ALOGE("Invalid AMR_Mode : %d",amr_mode);
    return 0;
  }

  /* Initialize IF1 frame data and info */
  if1_frame_info->fqi = true;

  if1_frame_info->amr_type = AMRSUP_CODEC_AMR_NB;

  memset(if1_frame, 0,
           amrsup_frame_len(AMRSUP_SPEECH_GOOD, AMRSUP_MODE_1220));


  switch (frame_type)
  {
    case AMRSUP_SID_BAD:
      if1_frame_info->fqi = false;
      /* fall thru */

    case AMRSUP_SID_FIRST:
    case AMRSUP_SID_UPDATE:
      /* Set frame type index */
      if1_frame_info->frame_type_index
      = AMRSUP_FRAME_TYPE_INDEX_AMR_SID;


      /* ===== Encoding SID frame ===== */
      /* copy the sid frame to class_a data */
      for (i=0; i<5; i++)
      {
        if1_frame[i] = vocoder_packet[i];
      }

      /* Set the SID type : SID_FIRST: Bit 35 = 0, SID_UPDATE : Bit 35 = 1 */
      if (frame_type == AMRSUP_SID_FIRST)
      {
        if1_frame[4] &= ~0x10;
      }

      if (frame_type == AMRSUP_SID_UPDATE)
      {
        if1_frame[4] |= 0x10;
      }
      else
      {
      /* Set the mode (Bit 36 - 38 = amr_mode with bits swapped)
      */
      if1_frame[4] |= (((unsigned char)amr_mode << 3) & 0x08)
        | (((unsigned char)amr_mode << 1) & 0x04) | (((unsigned char)amr_mode >> 1) & 0x02);

      frame_len = AMR_CLASS_A_BITS_SID;
      }

      break;


    case AMRSUP_SPEECH_BAD:
      if1_frame_info->fqi = false;
      /* fall thru */

    case AMRSUP_SPEECH_GOOD:
      /* Set frame type index */

        if1_frame_info->frame_type_index
        = (amrsup_frame_type_index_type)(amr_mode);

      /* ===== Encoding Speech frame ===== */
      /* Clear num bits in frame */
      frame_len = 0;

      /* Select ordering table */
      ordering_table =
      (amrsup_frame_order_type*)&amrsup_122_framing;

      amrsup_tx_order(
        if1_frame,
        &frame_len,
        vocoder_packet,
        ordering_table->len_a,
        ordering_table->class_a
      );

      amrsup_tx_order(
        if1_frame,
        &frame_len,
        vocoder_packet,
        ordering_table->len_b,
        ordering_table->class_b
      );

      amrsup_tx_order(
        if1_frame,
        &frame_len,
        vocoder_packet,
        ordering_table->len_c,
        ordering_table->class_c
      );


      /* frame_len already updated with correct number of bits */
      break;



    default:
      ALOGE("Unsupported frame type %d", frame_type);
      /* fall thru */

    case AMRSUP_NO_DATA:
      /* Set frame type index */
      if1_frame_info->frame_type_index = AMRSUP_FRAME_TYPE_INDEX_NO_DATA;

      frame_len = 0;

      break;
  }  /* end switch */


  /* convert bit length to byte length */
  frame_len = (frame_len + 7) / 8;

  return frame_len;
}

static void amr_transcode(unsigned char *src, unsigned char *dst)
{
   amrsup_frame_type frame_type_in = (amrsup_frame_type) *(src++);
   amrsup_mode_type frame_rate_in = (amrsup_mode_type) *(src++);
   amrsup_if1_frame_info_type frame_info_out;
   unsigned char frameheader;

   amrsup_if1_framing(src, frame_type_in, frame_rate_in, dst+1, &frame_info_out);
   frameheader = (frame_info_out.frame_type_index << 3) + (frame_info_out.fqi << 2);
   *dst = frameheader;

   return;
}

}; // namespace android
