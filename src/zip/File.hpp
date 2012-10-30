////////////////////////////////////////////////////////////
//
// Zip File
// Copyright (C) 2012 Chase Warrington (staff@spacechase0.com)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

#ifndef ZIP_FILE_HPP
#define ZIP_FILE_HPP

#include <sstream> // How can I get rid of this?
#include <string>

#include "zip/Entry.hpp"
#include "zip/EntryBase.hpp"

namespace zip
{
	class File : public priv::EntryBase
	{
		public:
			bool loadFromFile( const std::string& filename );
			bool loadFromMemory( const std::string& contents );
			
			bool saveToFile( const std::string& filename );
			void saveToMemory( std::string& contents );
			
			void addFile( const std::string& path, const std::string& contents );
			void addDirectory( const std::string& path );
		
		private:
			std::string getParent( const std::string& path );
			std::string getName( const std::string& path );
			Entry* createEntryAt( const std::string& path );
	};
}

#endif // ZIP_FILE_HPP
