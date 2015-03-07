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
