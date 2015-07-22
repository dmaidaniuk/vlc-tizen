#ifndef __vlc_H__
#define __vlc_H__

#include <app.h>
#include <Elementary.h>
#include <system_settings.h>
#include <efl_extension.h>
#include <dlog.h>

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "vlc"

#if !defined(PACKAGE)
#define PACKAGE "org.videolan.vlc"
#endif

#endif /* __vlc_H__ */
