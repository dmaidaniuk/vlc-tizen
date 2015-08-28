#ifndef VIDEO_VIEW_H_
#define VIDEO_VIEW_H_

typedef struct directory_data {
	Evas_Object *parent;
	char *file_path;
} directory_data_s;

Evas_Object*
create_directory_view(char* path, Evas_Object *parent);

#endif /* VIDEO_VIEW_H_ */
