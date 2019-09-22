#include <stdlib.h>
#include <string.h>

#include "cengine/types/string.h"

#include "cengine/ui/dialog.h"

#pragma region File Dialog

static DialogFile *ui_dialog_file_new (void) {

    DialogFile *df = (DialogFile *) malloc (sizeof (DialogFile));
    if (df) {
        df->custom_command = NULL;
        df->selected_filename = NULL;
    }

    return df;

}

static void ui_dialog_file_delete (void *df_ptr) {

    if (df_ptr) {
        DialogFile *df = (DialogFile *) df_ptr;
        str_delete (df->custom_command);
        str_delete (df->selected_filename);
        free (df);
    }

}

// sets a custom command to be executed to open a file dialog instead of the default
void ui_dialog_file_set_custom_command (Dialog *dialog, const char *command) {

    if (dialog) {
        DialogFile *df = (DialogFile *) dialog->type_data;
        str_delete (df->custom_command);
        df->custom_command = command ? str_new (command) : NULL;
    }

}

// gets the selected filename from the file dialog
String *ui_dialog_file_get_selected (Dialog *dialog) {

    return (dialog ? ((DialogFile *) dialog->type_data)->selected_filename : NULL);

}

#pragma endregion

#pragma region Dialog

static Dialog *ui_dialog_new (void) {

    Dialog *dialog = (Dialog *) malloc (sizeof (Dialog));
    if (dialog) {
        memset (dialog, 0, sizeof (Dialog));
        dialog->type_data = NULL;
    }

    return dialog;

}

void ui_dialog_delete (void *dialog_ptr) {

    if (dialog_ptr) {
        Dialog *dialog = (Dialog *) dialog_ptr;

        switch (dialog->type) {
            case DIALOG_TYPE_FILE_SELECTOR: ui_dialog_file_delete (dialog->type_data); break;
            default: break;
        }

        free (dialog);
    }

}

// creates a new dialog of the specific type
Dialog *ui_dialog_create (DialogType type) {

    Dialog *dialog = ui_dialog_new ();
    if (dialog) {
        dialog->type = type;
        switch (dialog->type) {
            case DIALOG_TYPE_FILE_SELECTOR: dialog->type_data = ui_dialog_file_new (); break;
            default: break;
        }
    }

    return dialog;

}

#pragma endregion