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

#include "IMediaLibrary.h"

#include <app_common.h>

#include "media_library.hpp"
#include "application.h"

#include "common.h"

class media_library : public IMediaLibraryCb
{
public:
	media_library();

	bool initialize();
	void discover( const std::string& location );
	// IMediaLibraryCb
	virtual void onMetadataUpdated( FilePtr file ) override;

    virtual void onDiscoveryStarted( const std::string& entryPoint ) override;
    virtual void onFileAdded( FilePtr file ) override;
    virtual void onDiscoveryCompleted( const std::string& entryPoint ) override;

private:
	std::unique_ptr<IMediaLibrary> m_ml;
};

media_library::media_library()
	: m_ml( MediaLibraryFactory::create() )
{
	if ( m_ml == nullptr )
		throw std::runtime_error( "Failed to initialize MediaLibrary" );
}

bool
media_library::initialize()
{
	auto appDataCStr = std::unique_ptr<char, void(*)(void*)>( app_get_data_path(), &free );
	std::string appData( appDataCStr.get() );
	if ( appDataCStr == nullptr )
	{
		LOGE( "Failed to fetch application data directory" );
		return false;
	}
	return m_ml->initialize( appData + "vlc.db", appData + "/snapshots", this );
}

void
media_library::onMetadataUpdated( FilePtr )
{

}

void
media_library::onDiscoveryStarted( const std::string& entryPoint )
{
	LOGI( "Starting [%s] discovery", entryPoint.c_str() );
}

void
media_library::onFileAdded( FilePtr )
{
	//FIXME
}

void
media_library::onDiscoveryCompleted( const std::string& entryPoint )
{
	LOGI("Completed [%s] discovery", entryPoint.c_str() );
}

void
media_library::discover( const std::string& location )
{
	m_ml->discover( location );
}

media_library *
CreateMediaLibrary(application *p_app)
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
StartMediaLibrary( media_library* p_media_library )
{
	return p_media_library->initialize();
}

void
DeleteMediaLibrary(media_library* p_media_library)
{
	delete p_media_library;
}

void
Discover( media_library* p_ml, const char* psz_location )
{
	p_ml->discover( psz_location );
}
