//
// Created by Игорь Данилов on 21.08.2023.
//
#include "test_index.h"
#include <furi.h>
#include <cli/cli.h>
#include <toolbox/args.h>

static void fn_test_cli(Cli* cli, FuriString* args, void* context) {
    UNUSED(cli);
    UNUSED(args);
    UNUSED(context);
    printf("Test\r\n");
}

void fn_register_tests() {
#ifdef FURI_DEBUG
    Cli* cli = furi_record_open(RECORD_CLI);
    cli_add_command(cli, "fn_test", CliCommandFlagParallelSafe, fn_test_cli, NULL);
    furi_record_close(RECORD_CLI);
#else
    UNUSED(nfc_cli);
#endif
}

void fn_unregister_tests() {
#ifdef FURI_DEBUG
    Cli* cli = furi_record_open(RECORD_CLI);
    cli_delete_command(cli, "fn_test");
    furi_record_close(RECORD_CLI);
#else
    UNUSED(nfc_cli);
#endif
}
