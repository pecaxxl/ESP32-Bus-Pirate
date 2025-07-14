#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
// TinyUSB Configuration for ESP32-S3 + Cardputer
// -----------------------------------------------------------------------------

// ----------------------------
// USB Root Settings
// ----------------------------

#define CFG_TUSB_MCU               OPT_MCU_ESP32S3
#define CFG_TUSB_RHPORT0_MODE      OPT_MODE_DEVICE
#define CFG_TUSB_OS                OPT_OS_NONE

// ----------------------------
// USB Buffer Size
// ----------------------------
#define CFG_TUD_ENDPOINT0_SIZE     64

// ----------------------------
// Class Driver Enable Flags
// ----------------------------

#ifndef CFG_TUD_CDC
#define CFG_TUD_CDC                1
#endif

#ifndef CFG_TUD_MSC
#define CFG_TUD_MSC                1
#endif

#ifndef CFG_TUD_HID
#define CFG_TUD_HID                1
#endif

#ifndef CFG_TUD_AUDIO
#define CFG_TUD_AUDIO              1
#endif

// -----------------------------------------------------------------------------
// AUDIO Configuration (UAC)
// -----------------------------------------------------------------------------

// Taille totale du descripteur audio (mic + speaker)
// Peut être ajusté si besoin (macros dispos dans `audio.h`)
#define TUD_AUDIO_MIC_ONE_CH_DESC_LEN      0x4D
#define TUD_AUDIO_SPEAKER_ONE_CH_DESC_LEN  0x4D

#define CFG_TUD_AUDIO_FUNC_1_DESC_LEN  (TUD_AUDIO_MIC_ONE_CH_DESC_LEN + TUD_AUDIO_SPEAKER_ONE_CH_DESC_LEN)


// Nombre d'interfaces audio (2 = micro + speaker)
#define CFG_TUD_AUDIO_FUNC_1_N_AS_INT      2

// Buffer contrôle (requêtes SET_CUR, etc.)
#define CFG_TUD_AUDIO_FUNC_1_CTRL_BUF_SZ   64

// Taille maximale des paquets audio IN/OUT
#define CFG_TUD_AUDIO_EP_SZ_IN             192
#define CFG_TUD_AUDIO_EP_SZ_OUT            192

#ifdef __cplusplus
}
#endif
