#ifndef _CENGINE_UI_DIALOG_H_
#define _CENGINE_UI_DIALOG_H_

#include "cengine/types/string.h"

struct _Dialog;

typedef enum DialogType {

    DIALOG_TYPE_FILE_SELECTOR,

} DialogType;

typedef struct DialogFile {

    String *custom_command;
    String *selected_filename;

} DialogFile;

// sets a custom command to be executed to open a file dialog instead of the default
extern void ui_dialog_file_set_custom_command (struct _Dialog *dialog, const char *command);

// gets the selected filename from the file dialog
extern String *ui_dialog_file_get_selected (struct _Dialog *dialog);

struct _Dialog {

    DialogType type;
    void *type_data;

};

typedef struct _Dialog Dialog;

extern void ui_dialog_delete (void *dialog_ptr);

// creates a new dialog of the specific type
extern Dialog *ui_dialog_create (DialogType type);

#endif