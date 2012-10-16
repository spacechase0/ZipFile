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

#include <iostream>
#include <string>

#include "zip/File.hpp"

namespace
{
	class ConsolePauser
	{
		public:
			~ConsolePauser()
			{
				std::cout << std::endl << "(Press enter to continue...)";
				
				std::string str;
				std::getline( std::cin, str );
			}
	};
	
	std::string operator * ( const std::string& str, int num )
	{
		std::string str_;
		for ( int i = 0; i < num; ++i )
		{
			str_ += str;
		}
		return str_;
	}

	void printZipFileStructure( zip::File::ConstIterator it, zip::File::ConstIterator end, const std::string prefix = "\t" )
	{
		for ( ; it != end; ++it )
		{
			zip::Entry* entry = it->get();
			std::cout << prefix << entry->getName();
			
			if ( entry->isDirectory() )
			{
				std::cout << '/' << std::endl;
				printZipFileStructure( entry->begin(), entry->end(), "\t" + prefix );
			}
			else
			{
				std::cout << std::endl;
			}
		}
	}
}

int main_zip( int argc, char* argv[] )
{
	std::unique_ptr< ConsolePauser > cp;
	if ( argc < 3 or argv[ 2 ] != std::string( "--nopause" ) )
	{
		cp.reset( new ConsolePauser() );
	}
	
	#ifndef DEBUG
	if ( argc < 2 )
	{
		std::cout << "Usage: zipfile <filename> [--nopause]" << std::endl;
	}
	#endif
	
	zip::File file;
	if ( !file.loadFromFile( ( argc > 1 ) ? argv[ 1 ] : "win_other.zip" ) )
	{
		std::cout << "Failed to load zip file." << std::endl;
		return 1;
	}
	
	std::cout << "File structure: " << std::endl;
	printZipFileStructure( file.begin(), file.end() );
	
	while ( true )
	{
		std::cout << std::endl << "Choose an entry to browse the contents of: ";
		
		std::string line;
		std::getline( std::cin, line );
		
		if ( line == "" )
		{
			break;
		}
		else if ( line == "/" )
		{
			std::cout << "File structure: " << std::endl;
			printZipFileStructure( file.begin(), file.end() );
		}
		else
		{
			zip::Entry* entry = file.getEntry( line );
			if ( entry == NULL )
			{
				std::cout << "No such entry." << std::endl;
			}
			else if ( entry->isDirectory() )
			{
				std::cout << "That is a directory." << std::endl;
			}
			else
			{
				std::cout << entry->getContents() << std::endl;
			}
		}
	}
	
	return 0;
}
