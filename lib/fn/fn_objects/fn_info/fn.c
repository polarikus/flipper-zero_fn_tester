//
// Created by Игорь Данилов on 23.02.2023.
//
#include "fn_i.h"

#define FFD_1_STR "1"
#define FFD_1_05_STR "1.05"
#define FFD_1_1_STR "1.1"
#define FFD_1_2_STR "1.2"

typedef enum {
    FNWarnReplaceNeeded = 0x01,
    FNWarnResourceEnd = 0x02,
    FNWarnMemoryEnd = 0x04,
    FNWarn_OFD_WaitTimeExceeded = 0x08,
    FNWarnFailureAccordingToFormatLogicalControlData = 0x10,
    FNWarnSetupKKTRequired = 0x32,
    FNWarn_OFD_Canceled = 0x40,
} FNWarnFlags;

bool fn_is_warn_flags_not_null(const FNInfo* fn){
    return (bool)(fn->fn_warn_flags);
}

void fn_get_warn_flags_str(const FNInfo* fn, FuriString* string)
{
    furi_string_cat_printf(string, "[ ");
    if(fn->fn_warn_flags & FNWarnReplaceNeeded) furi_string_cat_printf(string,
                                                                       "Urgent replacement of FN (before the expiration date 3 days)\n");
    if(fn->fn_warn_flags & FNWarnResourceEnd) furi_string_cat_printf(string,
                                                                     "FN resource exhaustion (before the expiration of 30 days)\n");
    if(fn->fn_warn_flags & FNWarnMemoryEnd) furi_string_cat_printf(string,
                                                                   "FN memory overflow (FN archive is 99%% full)\n");
    if(fn->fn_warn_flags & FNWarn_OFD_WaitTimeExceeded) furi_string_cat_printf(string,
                                                                               "OFD response timeout exceeded\n");
    if(fn->fn_warn_flags & FNWarnFailureAccordingToFormatLogicalControlData) furi_string_cat_printf(string,
                                                                                                    "Refusal according to the format-logical control data (the attribute is transmitted in the Confirmation from the OFD)\n");
    if(fn->fn_warn_flags & FNWarnSetupKKTRequired) furi_string_cat_printf(string,
                                                                          "KKT setting required (the attribute is transmitted in the Confirmation from the OFD)\n");
    if(fn->fn_warn_flags & FNWarn_OFD_Canceled) furi_string_cat_printf(string,
                                                                       "OFD canceled (sign is transmitted in Confirmation from OFD)\n");
    furi_string_cat_printf(string, " ]");
}

uint32_t fn_get_last_document_number(const FNInfo* fn){
    return (fn->last_doc_number);
}

char* fn_get_fn_state(const FNInfo* fn)
{
    switch(fn->fn_state) {
    case FNStage1: return "Ready registration (1)";
    case FNStage2: return "Registered (2)";
    case FNStage3: return "The closed FN archive is awaiting transfer to the OFD (3)";
    case FNStage4: return "FN archive is closed (4)";
    default: return "Unknown";
    }
}

FNState fn_get_fn_state_enum(const FNInfo* fn)
{
    return (fn->fn_state);
}


void fn_get_last_document_datetime(const FNInfo* fn, FuriString* string){
    furi_string_cat_printf(string, "%02d.%02d.20%02d %02d:%02d",
    fn->date_time.date, fn->date_time.mouth, fn->date_time.year,
                       fn->date_time.hour, fn->date_time.minute);
};

void fn_info_copy(FNInfo* dest, const FNInfo* src)
{
    memcpy(dest, src, sizeof(FNInfo));
}

bool fn_is_mgm(const FNInfo* fn)
{
    return !(fn->fw_version.fw_mode);
}

bool fn_is_session_open(const FNInfo* fn)
{
    return (fn->is_session_open);
}

const char* fn_get_fw_version(const FNInfo* fn)
{
    return fn->fw_version.fw_version;
}

const char* fn_get_sn(const FNInfo* fn)
{
    FURI_LOG_D("FN", "SN: %s", fn->serial_number);
    return fn->serial_number;
}

const char* fn_get_ffd_version_string(const FNInfo* fn)
{
    switch(fn->ffd_version) {
    case FFD_NONE:
        return "not registered";
    case FFD_1:
        return FFD_1_STR;
    case FFD_1_05:
        return FFD_1_05_STR;
    case FFD_1_1:
        return FFD_1_1_STR;
    case FFD_1_2:
        return FFD_1_2_STR;
    default:
        return "unknown";
    }
}

const char* fn_get_max_ffd_version_string(const FNInfo* fn)
{
    switch(fn->max_ffd_version) {
    case FFD_1:
        return FFD_1_STR;
    case FFD_1_05:
        return FFD_1_05_STR;
    case FFD_1_1:
        return FFD_1_1_STR;
    case FFD_1_2:
        return FFD_1_2_STR;
    default:
        return "unknown";
    }
}

FN_FFD fn_get_ffd_enum(const FNInfo* fn){
    return fn->ffd_version;
}
FN_FFD fn_get_max_ffd_enum(const FNInfo* fn){
    return fn->max_ffd_version;
}