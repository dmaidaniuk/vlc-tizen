
#include "common.h"

#include <Elementary.h>

Evas_Object*
create_image(Evas_Object *parent, const char *image_path)
{
    /* Add and set images for buttons */
    char path[strlen(ICON_DIR)+strlen(image_path)+2];
    sprintf(path, ICON_DIR"/%s", image_path);
    Evas_Object *ic = elm_image_add(parent);

    /* */
    elm_image_resizable_set(ic, EINA_TRUE, EINA_TRUE);
    elm_image_file_set(ic, path, NULL);

    return ic;
}
