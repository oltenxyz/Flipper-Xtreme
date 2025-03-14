#include "subghz_remote_app_i.h"
#include <lib/toolbox/path.h>
#include <flipper_format/flipper_format_i.h>

#include <lib/subghz/protocols/protocol_items.h>

#include "applications/main/subghz/helpers/subghz_txrx_i.h"

// #include <lib/subghz/protocols/keeloq.h>
// #include <lib/subghz/protocols/star_line.h>

#include <lib/subghz/blocks/custom_btn.h>

#define TAG "SubGhzRemote"

// XXX Using TxRx
// [x] use TxRx preset subrem_sub_preset_load & subrem_tx_start_sub
// [x] subrem_sub_preset_load & drop subrem_set_preset_data
// [x] subrem_tx_start_sub
// [x] subrem_tx_stop_sub

static const char* map_file_labels[SubRemSubKeyNameMaxCount][2] = {
    [SubRemSubKeyNameUp] = {"UP", "ULABEL"},
    [SubRemSubKeyNameDown] = {"DOWN", "DLABEL"},
    [SubRemSubKeyNameLeft] = {"LEFT", "LLABEL"},
    [SubRemSubKeyNameRight] = {"RIGHT", "RLABEL"},
    [SubRemSubKeyNameOk] = {"OK", "OKLABEL"},
};

static void subrem_map_preset_reset(SubRemMapPreset* map_preset) {
    furi_assert(map_preset);

    for(uint8_t i = 0; i < SubRemSubKeyNameMaxCount; i++) {
        subrem_sub_file_preset_reset(map_preset->subs_preset[i]);
    }
}

static SubRemLoadMapState subrem_map_preset_check(
    SubRemMapPreset* map_preset,
    SubGhzTxRx* txrx,
    FlipperFormat* fff_data_file) {
    furi_assert(map_preset);
    furi_assert(txrx);

    bool all_loaded = true;
    SubRemLoadMapState ret = SubRemLoadMapStateErrorBrokenFile;

    SubRemLoadSubState sub_preset_loaded;
    SubRemSubFilePreset* sub_preset;

    for(uint8_t i = 0; i < SubRemSubKeyNameMaxCount; i++) {
        sub_preset = map_preset->subs_preset[i];

        sub_preset_loaded = SubRemLoadSubStateErrorNoFile;

        if(furi_string_empty(sub_preset->file_path)) {
            // FURI_LOG_I(TAG, "Empty file path");
        } else if(!flipper_format_file_open_existing(
                      fff_data_file, furi_string_get_cstr(sub_preset->file_path))) {
            FURI_LOG_W(TAG, "Error open file %s", furi_string_get_cstr(sub_preset->file_path));
        } else {
            sub_preset_loaded = subrem_sub_preset_load(sub_preset, txrx, fff_data_file);
        }

        // TODO:
        // Load file state logic
        // Label depending on the state
        // Move to remote scene

        if(sub_preset_loaded != SubRemLoadSubStateOK) {
            furi_string_set_str(sub_preset->label, "N/A");
            all_loaded = false;
        } else {
            ret = SubRemLoadMapStateNotAllOK;
        }

        if(ret != SubRemLoadMapStateErrorBrokenFile && all_loaded) {
            ret = SubRemLoadMapStateOK;
        }

        flipper_format_file_close(fff_data_file);
    }

    return ret;
}

static bool subrem_map_preset_load(SubRemMapPreset* map_preset, FlipperFormat* fff_data_file) {
    furi_assert(map_preset);
    bool ret = false;
    SubRemSubFilePreset* sub_preset;
    for(uint8_t i = 0; i < SubRemSubKeyNameMaxCount; i++) {
        sub_preset = map_preset->subs_preset[i];
        if(!flipper_format_read_string(
               fff_data_file, map_file_labels[i][0], sub_preset->file_path)) {
#if FURI_DEBUG
            FURI_LOG_W(TAG, "No file patch for %s", map_file_labels[i][0]);
#endif
            sub_preset->type = SubGhzProtocolTypeUnknown;
        } else if(!flipper_format_rewind(fff_data_file)) {
            // Rewind error
        } else if(!flipper_format_read_string(
                      fff_data_file, map_file_labels[i][1], sub_preset->label)) {
#if FURI_DEBUG
            FURI_LOG_W(TAG, "No Label for %s", map_file_labels[i][0]);
#endif
            // TODO move to remote scene
            path_extract_filename(sub_preset->file_path, sub_preset->label, true);
            ret = true;
        } else {
            ret = true;
        }
        if(ret) {
            // Preload seccesful
            FURI_LOG_I(
                TAG,
                "%-5s: %s %s",
                map_file_labels[i][0],
                furi_string_get_cstr(sub_preset->label),
                furi_string_get_cstr(sub_preset->file_path));
            sub_preset->load_state = SubRemLoadSubStatePreloaded;
        }

        flipper_format_rewind(fff_data_file);
    }
    return ret;
}

SubRemLoadMapState subrem_map_file_load(SubGhzRemoteApp* app, const char* file_path) {
    furi_assert(app);
    furi_assert(file_path);
#if FURI_DEBUG
    FURI_LOG_I(TAG, "Load Map File Start");
#endif
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);
    SubRemLoadMapState ret = SubRemLoadMapStateErrorOpenError;
#if FURI_DEBUG
    FURI_LOG_I(TAG, "Open Map File..");
#endif
    subrem_map_preset_reset(app->map_preset);

    if(!flipper_format_file_open_existing(fff_data_file, file_path)) {
        FURI_LOG_E(TAG, "Could not open MAP file %s", file_path);
        ret = SubRemLoadMapStateErrorOpenError;
    } else {
        if(!subrem_map_preset_load(app->map_preset, fff_data_file)) {
            FURI_LOG_E(TAG, "Could no Sub file path in MAP file");
            // ret = // error for popup
        } else if(!flipper_format_file_close(fff_data_file)) {
            ret = SubRemLoadMapStateErrorOpenError;
        } else {
            ret = subrem_map_preset_check(app->map_preset, app->txrx, fff_data_file);
        }
    }

    if(ret == SubRemLoadMapStateOK) {
        FURI_LOG_I(TAG, "Load Map File Seccesful");
    } else if(ret == SubRemLoadMapStateNotAllOK) {
        FURI_LOG_I(TAG, "Load Map File Seccesful [Not all files]");
    } else {
        FURI_LOG_E(TAG, "Broken Map File");
    }

    flipper_format_file_close(fff_data_file);
    flipper_format_free(fff_data_file);

    furi_record_close(RECORD_STORAGE);
    return ret;
}

bool subrem_save_protocol_to_file(FlipperFormat* flipper_format, const char* sub_file_name) {
    furi_assert(flipper_format);
    furi_assert(sub_file_name);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    Stream* flipper_format_stream = flipper_format_get_raw_stream(flipper_format);

    bool saved = false;
    uint32_t repeat = 200;
    FuriString* file_dir = furi_string_alloc();

    path_extract_dirname(sub_file_name, file_dir);
    do {
        // removing additional fields
        flipper_format_delete_key(flipper_format, "Repeat");
        // flipper_format_delete_key(flipper_format, "Manufacture");

        if(!storage_simply_remove(storage, sub_file_name)) {
            break;
        }

        //ToDo check Write
        stream_seek(flipper_format_stream, 0, StreamOffsetFromStart);
        stream_save_to_file(flipper_format_stream, storage, sub_file_name, FSOM_CREATE_ALWAYS);

        if(!flipper_format_insert_or_update_uint32(flipper_format, "Repeat", &repeat, 1)) {
            FURI_LOG_E(TAG, "Unable Repeat");
            break;
        }

        saved = true;
    } while(0);

    furi_string_free(file_dir);
    furi_record_close(RECORD_STORAGE);
    return saved;
}

void subrem_save_active_sub(void* context) {
    furi_assert(context);
    SubGhzRemoteApp* app = context;

    SubRemSubFilePreset* sub_preset = app->map_preset->subs_preset[app->chusen_sub];
    subrem_save_protocol_to_file(
        sub_preset->fff_data, furi_string_get_cstr(sub_preset->file_path));
}

bool subrem_tx_start_sub(SubGhzRemoteApp* app, SubRemSubFilePreset* sub_preset) {
    furi_assert(app);
    furi_assert(sub_preset);
    bool ret = false;

    subrem_tx_stop_sub(app, true);

    if(sub_preset->type == SubGhzProtocolTypeUnknown) {
        ret = false;
    } else {
        FURI_LOG_I(TAG, "Send %s", furi_string_get_cstr(sub_preset->label));

        subghz_txrx_load_decoder_by_name_protocol(
            app->txrx, furi_string_get_cstr(sub_preset->protocaol_name));

        subghz_txrx_set_preset(
            app->txrx,
            furi_string_get_cstr(sub_preset->freq_preset.name),
            sub_preset->freq_preset.frequency,
            NULL,
            0);

        subghz_custom_btn_set(SUBGHZ_CUSTOM_BTN_OK);
        keeloq_reset_original_btn();
        subghz_custom_btns_reset();

        if(subghz_txrx_tx_start(app->txrx, sub_preset->fff_data) == SubGhzTxRxStartTxStateOk) {
            ret = true;
        }
    }

    app->tx_running = ret;

    return ret;
}

bool subrem_tx_stop_sub(SubGhzRemoteApp* app, bool forced) {
    furi_assert(app);
    SubRemSubFilePreset* sub_preset = app->map_preset->subs_preset[app->chusen_sub];

    if(forced || (sub_preset->type != SubGhzProtocolTypeRAW)) {
        // XXX drop app->tx_running
        if(app->tx_running) {
            subghz_txrx_stop(app->txrx);

            if(sub_preset->type == SubGhzProtocolTypeDynamic) {
                keeloq_reset_mfname();
                keeloq_reset_kl_type();
                keeloq_reset_original_btn();
                subghz_custom_btns_reset();
                star_line_reset_mfname();
                star_line_reset_kl_type();
            }

            app->tx_running = false;
            return true;
        }
    }

    return false;
}

SubRemLoadMapState subrem_load_from_file(SubGhzRemoteApp* app) {
    furi_assert(app);

    FuriString* file_path = furi_string_alloc();
    SubRemLoadMapState ret = SubRemLoadMapStateBack;

    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(&browser_options, SUBREM_APP_EXTENSION, &I_sub1_10px);
    browser_options.base_path = SUBREM_APP_FOLDER;

    // Input events and views are managed by file_select
    if(!dialog_file_browser_show(app->dialogs, app->file_path, app->file_path, &browser_options)) {
    } else {
        ret = subrem_map_file_load(app, furi_string_get_cstr(app->file_path));
    }

    furi_string_free(file_path);

    return ret;
}
