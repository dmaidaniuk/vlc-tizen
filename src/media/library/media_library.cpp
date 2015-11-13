/*****************************************************************************
 * Copyright © 2015 VideoLAN, VideoLabs SAS
 *****************************************************************************
 *
 * Authors: Hugo Beauzée-Luyssen <hugo@beauzee.fr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/
/*
 * By committing to this project, you allow VideoLAN and VideoLabs to relicense
 * the code to a different OSI approved license, in case it is required for
 * compatibility with the Store
 *****************************************************************************/

#include "common.h"

#include <Ecore.h>

#include "IMediaLibrary.h"
#include "IVideoTrack.h"
#include "IArtist.h"
#include "media_library_private.hpp"
#include "system_storage.h"


media_library::media_library()
    : ml( MediaLibraryFactory::create() )
{
    if ( ml == nullptr )
        throw std::runtime_error( "Failed to initialize MediaLibrary" );
}

void
media_library::onFileAdded( MediaPtr file )
{
    sendFileUpdate( file, true );
}

void
media_library::onFileUpdated( MediaPtr file )
{
    sendFileUpdate( file, false );
}

void
media_library::sendFileUpdate( MediaPtr file, bool added )
{
    auto item = fileToMediaItem( file );
    auto ctx = new FileUpdateCallbackCtx{this, item, added};
    ecore_main_loop_thread_safe_call_async([](void* data) {
        std::unique_ptr<FileUpdateCallbackCtx> ctx( reinterpret_cast<FileUpdateCallbackCtx*>(data) );
        for ( auto& p : ctx->ml->m_onItemUpdatedCb )
        {
            if ( p.first( p.second, reinterpret_cast<library_item*>(ctx->item.get()), ctx->added ) == true )
                break;
        }
    }, ctx);
}

void
media_library::onDiscoveryStarted( const std::string& entryPoint )
{
    LOGI( "Starting [%s] discovery", entryPoint.c_str() );
}

void
media_library::onDiscoveryCompleted( const std::string& entryPoint )
{
    LOGI("Completed [%s] discovery", entryPoint.c_str() );
}

void
media_library::onReloadStarted()
{
    LOGI("Reloading media library...");
}

void
media_library::onReloadCompleted()
{
    LOGI("Media library reload completed.");
    for (auto &p : m_onChangeCb)
    {
        ecore_main_loop_thread_safe_call_async( p.first, p.second );
    }
}

void
media_library::onParsingStatsUpdated( uint32_t nbParsed, uint32_t nbToParse )
{
    LOGI("Parser progress: %d/%d", nbParsed, nbToParse);
}

void
media_library::registerOnChange(media_library_file_list_changed_cb cb, void* cbUserData)
{
    m_onChangeCb.emplace_back(cb, cbUserData);
}

void
media_library::unregisterOnChange(media_library_file_list_changed_cb cb, void* cbUserData)
{
    auto ite = end(m_onChangeCb);
    for (auto it = begin(m_onChangeCb); it != ite; ++it)
    {
        if ((*it).first == cb && (*it).second == cb)
        {
            m_onChangeCb.erase(it);
            return;
        }
    }
}

void
media_library::registerOnItemUpdated(media_library_item_updated_cb cb, void* userData)
{
    m_onItemUpdatedCb.emplace_back( cb, userData );
}

void
media_library::unregisterOnItemUpdated(media_library_item_updated_cb cb, void* userData)
{
    auto ite = end(m_onItemUpdatedCb);
    for (auto it = begin(m_onItemUpdatedCb); it != ite; ++it)
    {
        if ((*it).first == cb && (*it).second == cb)
        {
            m_onItemUpdatedCb.erase(it);
            return;
        }
    }
}

media_library *
media_library_create(application *p_app)
{
    try
    {
        return new media_library;
    }
    catch (std::exception& ex)
    {
        LOGE( "%s", ex.what() );
        return nullptr;
    }
}

bool
media_library_start(media_library* p_media_library)
{
    auto appDataCStr = std::unique_ptr<char, void(*)(void*)>( system_storage_appdata_get(), &free );
    std::string appData( appDataCStr.get() );
    if ( appDataCStr == nullptr )
    {
        LOGE( "Failed to fetch application data directory" );
        return false;
    }
    // Always ensure the folder exists
    errno = 0;
    auto res = mkdir( appData.c_str(), 0700 );
    if ( res != 0 && errno != EEXIST)
    {
        LOGE("Failed to create data directory: %s", strerror(errno));
        return false;
    }
    auto snapshotPath = appData + "/snapshots";
    res = mkdir( snapshotPath.c_str(), 0700 );
    if ( res != 0 && errno != EEXIST)
    {
        LOGE("Failed to create snapshot directory: %s", strerror(errno));
        return false;
    }
    p_media_library->logger.reset( new TizenLogger );
    p_media_library->ml->setVerbosity( LogLevel::Info );
    p_media_library->ml->setLogger( p_media_library->logger.get() );
    return p_media_library->ml->initialize( appData + "vlc.db", snapshotPath, p_media_library );
}

void
media_library_delete(media_library* p_media_library)
{
    delete p_media_library;
}

void
media_library_discover( media_library* p_ml, const char* psz_location )
{
    p_ml->ml->discover( psz_location );
}

template <typename SourceFunc, typename ConvertorFunc>
struct ml_callback_context
{
    ml_callback_context( media_library_list_cb c, void* p_user_data, SourceFunc s, ConvertorFunc conv )
        : cb(c), list(nullptr), p_data(p_user_data)
          , source(s), convertor(conv){}
    media_library_list_cb cb;
    Eina_List* list;
    void* p_data;
    SourceFunc source;
    ConvertorFunc convertor;
};

template <typename SourceFunc, typename ConvertorFunc>
void
intermediate_list_callback( void* p_data )
{
    auto ctx = reinterpret_cast<ml_callback_context<SourceFunc, ConvertorFunc>*>( p_data );
    ctx->cb( ctx->list, ctx->p_data );
    delete ctx;
}

template <typename SourceFunc, typename ConvertorFunc>
static void media_library_common_getter(media_library_list_cb cb, void* p_user_data, SourceFunc source, ConvertorFunc conv)
{
    auto ctx = new ml_callback_context<SourceFunc, ConvertorFunc>( cb, p_user_data, source, conv );

    ecore_thread_run( [](void* data, Ecore_Thread* ) {
        auto ctx = reinterpret_cast<ml_callback_context<SourceFunc, ConvertorFunc>*>( data );
        auto items = ctx->source();
        Eina_List *list = nullptr;
        for ( auto& f : items )
        {
            auto elem = ctx->convertor( f );
            if ( elem == nullptr )
                continue;
            list = eina_list_append( list, elem );
        }
        ctx->list = list;
        ecore_main_loop_thread_safe_call_async( intermediate_list_callback<SourceFunc, ConvertorFunc>, ctx );
    }, nullptr, nullptr, ctx );
}

void
media_library_get_audio_files( media_library* p_ml, media_library_list_cb cb, void* p_user_data )
{
    media_library_common_getter(cb, p_user_data,
            std::bind(&IMediaLibrary::audioFiles, p_ml->ml),
            fileToMediaItem);
}

void
media_library_get_video_files( media_library* p_ml, media_library_list_cb cb, void* p_user_data )
{
    media_library_common_getter(cb, p_user_data,
            std::bind(&IMediaLibrary::videoFiles, p_ml->ml),
            fileToMediaItem);
}

void
media_library_get_albums(media_library* p_ml, media_library_list_cb cb, void* p_user_data)
{
    media_library_common_getter(cb, p_user_data,
            std::bind(&IMediaLibrary::albums, p_ml->ml),
            albumToAlbumItem);
}

void
media_library_get_artists( media_library* p_ml, media_library_list_cb cb, void* p_user_data )
{
    auto ml = p_ml->ml;
    media_library_common_getter(cb, p_user_data,
            std::bind(&IMediaLibrary::artists, p_ml->ml),
                    artistToArtistItem);
}

void
media_library_get_artist_albums( media_library* p_ml, unsigned int i_artist_id, media_library_list_cb cb, void* p_user_data )
{
    ArtistPtr artist = p_ml->ml->artist( i_artist_id );
    if (artist == nullptr)
    {
        LOGE("Can't find artist %d", i_artist_id);
        return;
    }
    media_library_common_getter(cb, p_user_data,
                std::bind(&IArtist::albums, artist),
                &albumToAlbumItem);
}

void
media_library_get_album_songs(media_library* p_ml, unsigned int i_album_id, media_library_list_cb cb, void* p_user_data)
{
    auto album = p_ml->ml->album(i_album_id);
    if (album == nullptr)
    {
        LOGE("Can't find album #%d", i_album_id);
        return;
    }
    media_library_common_getter(cb, p_user_data,
            std::bind(&IAlbum::tracks, album),
            fileToMediaItem);
}

void
media_library_get_artist_songs(media_library* p_ml, unsigned int i_artist_id, media_library_list_cb cb, void* p_user_data)
{
    ArtistPtr artist = p_ml->ml->artist(i_artist_id);
    if (artist == nullptr)
    {
        LOGE("Can't find artist %u", i_artist_id);
        return;
    }
    media_library_common_getter(cb, p_user_data,
                std::bind(&IArtist::media, artist),
                &fileToMediaItem);
}

void
media_library_register_on_change(media_library* ml, media_library_file_list_changed_cb cb, void* p_data)
{
    ml->registerOnChange(cb, p_data);
}

void
media_library_unregister_on_change(media_library* ml, media_library_file_list_changed_cb cb, void* p_data)
{
    ml->unregisterOnChange(cb, p_data);
}

void
media_library_register_item_updated(media_library* ml, media_library_item_updated_cb cb, void* p_data )
{
    ml->registerOnItemUpdated(cb, p_data);
}

void
media_library_unregister_item_updated(media_library* ml, media_library_item_updated_cb cb, void* p_data )
{
    ml->unregisterOnItemUpdated(cb, p_data);
}

void
media_library_reload(media_library* ml)
{
    ml->ml->reload();
}

bool
media_library_is_various_artist(const artist_item* p_item)
{
    return p_item->i_id == medialibrary::VariousArtistID;
}
