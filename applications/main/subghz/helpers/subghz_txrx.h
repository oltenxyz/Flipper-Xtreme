#pragma once

#include "subghz_types.h"

#include <lib/subghz/subghz_worker.h>
#include <lib/subghz/subghz_setting.h>
#include <lib/subghz/receiver.h>
#include <lib/subghz/transmitter.h>
#include <lib/subghz/protocols/raw.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SubGhzTxRx SubGhzTxRx;

typedef void (*SubGhzTxRxNeedSaveCallback)(void* context);

typedef enum {
    SubGhzTxRxStartTxStateOk,
    SubGhzTxRxStartTxStateErrorOnlyRx,
    SubGhzTxRxStartTxStateErrorParserOthers,
} SubGhzTxRxStartTxState;

/**
 * Allocate SubGhzTxRx
 * 
 * @return SubGhzTxRx* pointer to SubGhzTxRx
 */
SubGhzTxRx* subghz_txrx_alloc();

/**
 * Free SubGhzTxRx
 * 
 * @param instance Pointer to a SubGhzTxRx
 */
void subghz_txrx_free(SubGhzTxRx* instance);

/**
 * Check if the database is loaded
 * 
 * @param instance Pointer to a SubGhzTxRx
 * @return bool True if the database is loaded
 */
bool subghz_txrx_is_database_loaded(SubGhzTxRx* instance);

/**
 * Set preset 
 * 
 * @param instance Pointer to a SubGhzTxRx
 * @param preset_name Name of preset
 * @param frequency Frequency in Hz
 * @param preset_data Data of preset
 * @param preset_data_size Size of preset data
 */
void subghz_txrx_set_preset(
    SubGhzTxRx* instance,
    const char* preset_name,
    uint32_t frequency,
    uint8_t* preset_data,
    size_t preset_data_size);

/**
 * Get name of preset
 * 
 * @param instance Pointer to a SubGhzTxRx
 * @param preset String of preset 
 * @return const char*  Name of preset
 */
const char* subghz_txrx_get_preset_name(SubGhzTxRx* instance, const char* preset);

/**
 * Get of preset
 * 
 * @param instance Pointer to a SubGhzTxRx
 * @return SubGhzRadioPreset Preset
 */
SubGhzRadioPreset subghz_txrx_get_preset(SubGhzTxRx* instance);

/**
 * Get string frequency and modulation
 * 
 * @param instance Pointer to a SubGhzTxRx
 * @param frequency Pointer to a string frequency
 * @param modulation Pointer to a string modulation
 */
void subghz_txrx_get_frequency_and_modulation(
    SubGhzTxRx* instance,
    FuriString* frequency,
    FuriString* modulation,
    bool long_name);

/**
 * Start TX CC1101
 * 
 * @param instance Pointer to a SubGhzTxRx
 * @param flipper_format Pointer to a FlipperFormat
 * @return SubGhzTxRxStartTxState 
 */
SubGhzTxRxStartTxState subghz_txrx_tx_start(SubGhzTxRx* instance, FlipperFormat* flipper_format);

/**
 * Start RX CC1101
 * 
 * @param instance Pointer to a SubGhzTxRx
 */
void subghz_txrx_rx_start(SubGhzTxRx* instance);

/**
 * Stop TX/RX CC1101
 * 
 * @param instance Pointer to a SubGhzTxRx
 */
void subghz_txrx_stop(SubGhzTxRx* instance);

/**
 * Set sleep mode CC1101
 * 
 * @param instance Pointer to a SubGhzTxRx
 */
void subghz_txrx_sleep(SubGhzTxRx* instance);

/**
 * Update frequency CC1101 in automatic mode (hopper)
 * 
 * @param instance Pointer to a SubGhzTxRx
 */
void subghz_txrx_hopper_update(SubGhzTxRx* instance);

/**
 * Get state hopper
 * 
 * @param instance Pointer to a SubGhzTxRx
 * @return SubGhzHopperState 
 */
SubGhzHopperState subghz_txrx_hopper_get_state(SubGhzTxRx* instance);

/**
 * Set state hopper
 * 
 * @param instance Pointer to a SubGhzTxRx
 * @param state State hopper
 */
void subghz_txrx_hopper_set_state(SubGhzTxRx* instance, SubGhzHopperState state);

/**
 * Unpause hopper
 * 
 * @param instance Pointer to a SubGhzTxRx
 */
void subghz_txrx_hopper_unpause(SubGhzTxRx* instance);

/**
 * Set pause hopper
 * 
 * @param instance Pointer to a SubGhzTxRx
 */
void subghz_txrx_hopper_pause(SubGhzTxRx* instance);

/**
 * Speaker on
 * 
 * @param instance Pointer to a SubGhzTxRx 
 */
void subghz_txrx_speaker_on(SubGhzTxRx* instance);

/**
 * Speaker off
 * 
 * @param instance Pointer to a SubGhzTxRx 
 */
void subghz_txrx_speaker_off(SubGhzTxRx* instance);

/**
 * Speaker mute
 * 
 * @param instance Pointer to a SubGhzTxRx 
 */
void subghz_txrx_speaker_mute(SubGhzTxRx* instance);

/**
 * Speaker unmute
 * 
 * @param instance Pointer to a SubGhzTxRx 
 */
void subghz_txrx_speaker_unmute(SubGhzTxRx* instance);

/**
 * Set state speaker
 * 
 * @param instance Pointer to a SubGhzTxRx 
 * @param state State speaker
 */
void subghz_txrx_speaker_set_state(SubGhzTxRx* instance, SubGhzSpeakerState state);

/**
 * Get state speaker
 * 
 * @param instance Pointer to a SubGhzTxRx 
 * @return SubGhzSpeakerState 
 */
SubGhzSpeakerState subghz_txrx_speaker_get_state(SubGhzTxRx* instance);

/**
 * load decoder by name protocol
 * 
 * @param instance Pointer to a SubGhzTxRx
 * @param name_protocol Name protocol
 * @return bool True if the decoder is loaded 
 */
bool subghz_txrx_load_decoder_by_name_protocol(SubGhzTxRx* instance, const char* name_protocol);

/**
 * Get decoder
 * 
 * @param instance Pointer to a SubGhzTxRx
 * @return SubGhzProtocolDecoderBase* Pointer to a SubGhzProtocolDecoderBase
 */
SubGhzProtocolDecoderBase* subghz_txrx_get_decoder(SubGhzTxRx* instance);

/**
 * Set callback for save data
 * 
 * @param instance Pointer to a SubGhzTxRx
 * @param callback Callback for save data
 * @param context Context for callback
 */
void subghz_txrx_set_need_save_callback(
    SubGhzTxRx* instance,
    SubGhzTxRxNeedSaveCallback callback,
    void* context);

/**
 * Get pointer to a load data key
 * 
 * @param instance Pointer to a SubGhzTxRx
 * @return FlipperFormat* 
 */
FlipperFormat* subghz_txrx_get_fff_data(SubGhzTxRx* instance);

/**
 * Get pointer to a SugGhzSetting
 * 
 * @param instance Pointer to a SubGhzTxRx
 * @return SubGhzSetting* 
 */
SubGhzSetting* subghz_txrx_get_setting(SubGhzTxRx* instance);

/**
 * Is it possible to save this protocol
 * 
 * @param instance Pointer to a SubGhzTxRx 
 * @return bool True if it is possible to save this protocol
 */
bool subghz_txrx_protocol_is_serializable(SubGhzTxRx* instance);

/**
 * Is it possible to send this protocol
 * 
 * @param instance Pointer to a SubGhzTxRx 
 * @return bool True if it is possible to send this protocol
 */
bool subghz_txrx_protocol_is_transmittable(SubGhzTxRx* instance, bool check_type);

/**
 * Set filter, what types of decoder to use 
 * 
 * @param instance Pointer to a SubGhzTxRx
 * @param filter Filter
 */
void subghz_txrx_receiver_set_filter(SubGhzTxRx* instance, SubGhzProtocolFlag filter);

/**
 * Set callback for receive data
 * 
 * @param instance Pointer to a SubGhzTxRx
 * @param callback Callback for receive data
 * @param context Context for callback
 */
void subghz_txrx_set_rx_calback(
    SubGhzTxRx* instance,
    SubGhzReceiverCallback callback,
    void* context);

/**
 * Set callback for Raw decoder, end of data transfer  
 * 
 * @param instance Pointer to a SubGhzTxRx
 * @param callback Callback for Raw decoder, end of data transfer 
 * @param context Context for callback
 */
void subghz_txrx_set_raw_file_encoder_worker_callback_end(
    SubGhzTxRx* instance,
    SubGhzProtocolEncoderRAWCallbackEnd callback,
    void* context);

void subghz_txrx_set_debug_pin_state(SubGhzTxRx* instance, bool state);
bool subghz_txrx_get_debug_pin_state(SubGhzTxRx* instance);

SubGhzReceiver* subghz_txrx_get_receiver(SubGhzTxRx* instance); // TODO use only in DecodeRaw

#ifdef __cplusplus
}
#endif
