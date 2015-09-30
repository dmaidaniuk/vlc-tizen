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

#include "IMediaLibrary.h"
#include "IFile.h"
#include "ILogger.h"
#include "IMovie.h"
#include "IShowEpisode.h"
#include "IVideoTrack.h"

#include "media_library.hpp"
#include "../system_storage.h"

class TizenLogger : public ILogger
{
    virtual void Error( const std::string& msg ) override
    {
        dlog_print( DLOG_ERROR, "medialibrary", msg.c_str() );
    }

    virtual void Warning( const std::string& msg ) override
    {
        dlog_print( DLOG_WARN, "medialibrary", msg.c_str() );
    }

    virtual void Info( const std::string& msg ) override
    {
        dlog_print( DLOG_INFO, "medialibrary", msg.c_str() );
    }
};

struct media_library : public IMediaLibraryCb
{
public:
    media_library();

    // IMediaLibraryCb
    virtual void onMetadataUpdated( FilePtr file ) override;

    virtual void onDiscoveryStarted( const std::string& entryPoint ) override;
    virtual void onFileAdded( FilePtr file ) override;
    virtual void onDiscoveryCompleted( const std::string& entryPoint ) override;

private:
    void onFileUpdated( FilePtr file );

public:
    std::unique_ptr<IMediaLibrary> ml;
    std::unique_ptr<TizenLogger> logger;
    media_added_cb mediaAddedCb;
};

media_library::media_library()
    : ml( MediaLibraryFactory::create() )
    , mediaAddedCb( nullptr )
{
    if ( ml == nullptr )
        throw std::runtime_error( "Failed to initialize MediaLibrary" );
}

void
media_library::onFileUpdated( FilePtr file )
{
    if ( file->isReady() == false )
        return;
    auto type = MEDIA_ITEM_TYPE_UNKNOWN;
    switch ( file->type() )
    {
    case IFile::Type::VideoType:
        type = MEDIA_ITEM_TYPE_VIDEO;
        break;
    case IFile::Type::AudioType:
        type = MEDIA_ITEM_TYPE_AUDIO;
        break;
    default:
        LOGW( "Unknown file type: %d", file->type() );
        return;
    }
    auto mi = media_item_create( file->mrl().c_str(), type );
    if ( mi == nullptr )
    {
        //FIXME: What should we do? This won't be run again until the next time
        //we restore the media library. Also, do we care? This is likely E_NOMEM, so we
        //might have bigger problems than a missing file...
        LOGE( "Failed to create media_item for file %s", file->mrl().c_str() );
        return;
    }
    mi->i_duration = file->duration();
    if ( file->type() == IFile::Type::VideoType )
    {
        auto vtracks = file->videoTracks();
        if ( vtracks.size() != 0 )
        {
            if ( vtracks.size() > 1 )
                LOGW( "Ignoring file [%s] extra video tracks for file description", file->mrl().c_str() );
            auto vtrack = vtracks[0];
            mi->i_w = vtrack->width();
            mi->i_h = vtrack->height();
        }
        else
        {
            // Assume a media library problem and just let it go.
            LOGW( "Adding video file [%s] with no video tracks detected.", file->mrl().c_str() );
        }
        // If this is a video, let's check if we can fetch it's movie/tvshow equivalent:
        auto movie = file->movie();
        if ( movie != nullptr )
        {
            mi->psz_title = strdup( movie->title().c_str() );
        }
        else
        {
            auto episode = file->showEpisode();
            if ( episode != nullptr )
            {
                mi->psz_title = strdup( episode->name().c_str() );
            }
            else
            {
                LOGW( "FIXME: Not a movie, not an episode, but we have no 'clip' type in MediaLibrary" );
            }
        }
    }
    else if ( file->type() == IFile::Type::AudioType )
    {
        // So far, nothing to do here.
    }

    mediaAddedCb( mi );
}

void
media_library::onMetadataUpdated( FilePtr file )
{
    onFileUpdated( file );
}

void
media_library::onFileAdded( FilePtr file )
{
    onFileUpdated( file );
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
media_library_start( media_library* p_media_library, media_added_cb cb )
{
    auto appDataCStr = std::unique_ptr<char, void(*)(void*)>( system_storage_appdata_get(), &free );
    std::string appData( appDataCStr.get() );
    if ( appDataCStr == nullptr )
    {
        LOGE( "Failed to fetch application data directory" );
        return false;
    }
    p_media_library->mediaAddedCb = cb;
    p_media_library->logger.reset( new TizenLogger );
    p_media_library->ml->setLogger( p_media_library->logger.get() );
    return p_media_library->ml->initialize( appData + "vlc.db", appData + "/snapshots", p_media_library );
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
