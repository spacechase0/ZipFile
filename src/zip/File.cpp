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

#include "zip/File.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <util/Convert.h>
#include <util/Endian.h>
#include <util/Tokenize.h>
#include <zlib.h>

#if false
	#include <iostream>
	
	#define print(a) std::cout << a
#else
	#define print(a)
#endif

// TO DO: Organize this mess!
// Also: Ignoring Zip64 :P

namespace
{
	template< typename T >
	T read( std::stringstream& ss )
	{
		T t;
		ss.read( reinterpret_cast< char* >( &t ), sizeof( T ) );
	
		#ifdef SFML_ENDIAN_BIG
		if ( sizeof( T ) > 1 )
		{
			t = util::swapBytes( t );
		}
		#endif
		
		return t;
	}
	
	std::string readStr( std::stringstream& ss, std::size_t len )
	{
		std::string str;
		for ( std::size_t i = 0; i < len; ++i )
		{
			char c = ss.get();
			str += c;
		}
		
		return str;
	}
	
	template< typename T >
	std::string readStr( std::stringstream& ss )
	{
		T len = read< T >( ss );
		return readStr( ss, len );
	}
	
	template< sf::Uint16 N >
	struct Signature
	{
		static const sf::Uint16 SIGNATURE;
	};
	template< sf::Uint16 N >
	const sf::Uint16 Signature< N >::SIGNATURE = N;
	
	// 4.3.7
	struct LocalFileHeaderBase
	{
		sf::Uint16 minVersion;
		sf::Uint16 flags;
		sf::Uint16 compressType;
		
		sf::Uint16 lastModTime;
		sf::Uint16 lastModDate;
		
		sf::Uint32 crc32;
		
		sf::Uint32 sizeCompressed;
		sf::Uint32 sizeNormal;
		
		std::string filename; // sf::Uint16 filenameLength
		std::string extra; // sf::Uint16 extraLength
	};
	
	struct LocalFileHeader : LocalFileHeaderBase, Signature< 0x0304 >
	{
	};
	
	// 4.3.9
	struct DataDescriptor : Signature< 0x0708 >
	{
		sf::Uint32 crc32; // ?
		
		sf::Uint32 sizeCompressed;
		sf::Uint32 sizeNormal;
	};
	
	// 4.3.11
	struct ArchiveExtraData : Signature< 0x0608 >
	{
		std::string extra; // sf::Uint32 extraLength
	};
	
	// 4.3.12
	struct CentralDirectoryStructure : LocalFileHeaderBase, Signature< 0x0102 >
	{
		sf::Uint16 versionMade;
		
		// LocalFileHeaderBase stuff here, except filename/extra
		
		std::string comment; // sf::Uint16 commentLength here, comment at the bottom
		
		sf::Uint16 diskNumStart;
		sf::Uint16 inAttr;
		sf::Uint32 exAttr;
		sf::Uint32 localHeaderOffset;
		
		// LocalFileHeaderBase filename/extra here
	};
	
	// 4.3.13
	struct DigitalSignature : Signature< 0x0505 >
	{
		std::string data; // sf::Uint16 size
	};
	
	// 4.3.16
	struct EndCentralDirectoryStructure : Signature< 0x0506 >
	{
		sf::Uint16 diskNum;
		sf::Uint16 diskNumStartCentralDirectory; // What is this? :P
		
		sf::Uint16 entryCountDisk;
		sf::Uint16 entryCountCentral;
		
		sf::Uint32 centralDirSize;
		sf::Uint32 centralDirOffset;
		
		std::string comment; // sf::Uint16 commentLength
	};
	
	namespace Flags
	{
		// 4.4.4
		enum General : sf::Uint16
		{
			Encrypted =               1 << 0,
			CompressionFlag1 =        1 << 1,
			CompressionFlag2 =        1 << 2,
			DataDescriptorPostponed = 1 << 3,
			ReservedForDeflation =    1 << 4,
			UsesPatchedData =         1 << 5, // ?
			StrongEncryption =        1 << 6,
			LanguageEncoded =         1 << 11,
			ReservedByPkware12 =      1 << 12,
			SomeValuesMasked =        1 << 13, // For strong encryption
			ReservedByPkware14 =      1 << 14,
			ReservedByPkware15 =      1 << 15,
		};
	}
	
	namespace Compression
	{
		// 4.4.5
		enum CompressionMethod : sf::Uint16
		{
			None = 0,
			Shrunk = 1,
			Reduced1 = 2,
			Reduced2 = 3,
			Reduced3 = 4,
			Reduced4 = 5,
			Imploded = 6,
			ReservedForTokenizing = 7,
			Deflated = 8,
			EnhancedDeflate64 = 9,
			PkwareImploding = 10,
			ReservedByPkware11 = 11,
			BZip2 = 12,
			ReservedByPkware13 = 13,
			Lzma = 14,
			ReservedByPkware15 = 15,
			ReservedByPkware16 = 16,
			ReservedByPkware17 = 17,
			IbmTerse = 18,
			IbmLz77 = 19,
			WavPack = 97,
			Ppmd = 98,
		};
	}
	
	// 4.4.7
	const sf::Uint32 magicNumberCrc32 = 0xe320bbde;
	
	std::string makeSig( sf::Uint16 sig )
	{
		// Not sure why
		#ifdef SFML_ENDIAN_LITTLE
			sig = util::swapBytes( sig );
		#endif
		
		std::string str1 = "PK";
		std::string str2( reinterpret_cast< const char* >( &sig ), 2 );
		return str1 + str2;
	}
	
	std::size_t findEndCentralDir( const std::string& contents )
	{
		std::string sig = makeSig( EndCentralDirectoryStructure::SIGNATURE );
		
		for ( std::size_t i = contents.length(); i >= 4; --i )
		{
			std::string str = contents.substr( i - 3, 4 );
			if ( str == sig )
			{
				return i - 3;
			}
		}
		
		return std::string::npos;
	}
	
	EndCentralDirectoryStructure readEndCentralDir( std::stringstream& ss )
	{
		sf::Uint32 sig = read< sf::Uint32 >( ss );
		
		EndCentralDirectoryStructure ecd;
		
		ecd.diskNum = read< sf::Uint16 >( ss );
		ecd.diskNumStartCentralDirectory = read< sf::Uint16 >( ss );
		
		ecd.entryCountDisk = read< sf::Uint16 >( ss );
		ecd.entryCountCentral = read< sf::Uint16 >( ss );
		
		if ( ecd.entryCountDisk != ecd.entryCountCentral )
		{
			throw std::runtime_error( "Don't know what to do/say." );
		}
		
		ecd.centralDirSize = read< sf::Uint32 >( ss );
		ecd.centralDirOffset = read< sf::Uint32 >( ss );
		
		ecd.comment = readStr< sf::Uint16 >( ss );
		
		return ecd;
	}
	
	void readLocalFileHeaderBase( std::stringstream& ss, LocalFileHeaderBase& lfh )
	{
		lfh.minVersion = read< sf::Uint16 >( ss );
		lfh.flags = read< sf::Uint16 >( ss );
		lfh.compressType = read< sf::Uint16 >( ss );
		
		lfh.lastModTime = read< sf::Uint16 >( ss );
		lfh.lastModDate = read< sf::Uint16 >( ss );
		
		lfh.crc32 = read< sf::Uint32 >( ss );
		
		lfh.sizeCompressed = read< sf::Uint32 >( ss );
		lfh.sizeNormal = read< sf::Uint32 >( ss );
	}
	
	CentralDirectoryStructure readCentralDir( std::stringstream& ss )
	{
		sf::Uint32 sig = read< sf::Uint32 >( ss );
		
		CentralDirectoryStructure cd;
		
		cd.versionMade = read< sf::Uint16 >( ss );
		
		readLocalFileHeaderBase( ss, cd );
		
		sf::Uint16 filenameLength = read< sf::Uint16 >( ss );
		sf::Uint16 extraLength = read< sf::Uint16 >( ss );
		sf::Uint16 commentLength = read< sf::Uint16 >( ss );
		
		cd.diskNumStart = read< sf::Uint16 >( ss );
		cd.inAttr = read< sf::Uint16 >( ss );
		cd.exAttr = read< sf::Uint32 >( ss );
		cd.localHeaderOffset = read< sf::Uint32 >( ss );
		
		cd.filename = readStr( ss, filenameLength );
		cd.extra = readStr( ss, extraLength );
		cd.comment = readStr( ss, commentLength );
		
		return cd;
	}
	
	LocalFileHeader readLocalFileHeader( std::stringstream& ss )
	{
		sf::Uint32 sig = read< sf::Uint32 >( ss );
		
		LocalFileHeader lf;
		
		readLocalFileHeaderBase( ss, lf );
		
		sf::Uint16 filenameLength = read< sf::Uint16 >( ss );
		sf::Uint16 extraLength = read< sf::Uint16 >( ss );
		
		lf.filename = readStr( ss, filenameLength );
		lf.extra = readStr( ss, extraLength );
		
		return lf;
	}
}

namespace zip
{
	bool File::loadFromFile( const std::string& filename )
	{
		std::fstream file( filename.c_str(), std::fstream::in | std::fstream::binary );
		if ( !file )
		{
			return false;
		}
		
		std::string contents;
		while ( file )
		{
			char c = file.get();
			if ( file )
			{
				contents += c;
			}
		}
		
		return loadFromMemory( contents );
	}
	
	bool File::loadFromMemory( const std::string& contents )
	{
		std::stringstream ss( contents, std::stringstream::in | std::stringstream::binary );
		
		try
		{
			#define streamCheck( a ) if ( !ss ) { throw std::runtime_error( "Stream error (maybe too short?) at " + std::string( a ) ); }
			
			std::size_t endCentralDirPos = findEndCentralDir( contents );
			if ( endCentralDirPos == std::string::npos )
			{
				return false;
			}
			ss.seekg( endCentralDirPos );
			streamCheck( "ecd pos" );
			
			EndCentralDirectoryStructure ecd = readEndCentralDir( ss );
			ss.seekg( ecd.centralDirOffset );
			streamCheck( "ecd" );
			
			std::vector< CentralDirectoryStructure > cds;
			cds.reserve( ecd.entryCountDisk );
			for ( std::size_t i = 0; i < ecd.entryCountDisk; ++i )
			{
				CentralDirectoryStructure cd = readCentralDir( ss );
				cds.push_back( cd );
			}
			streamCheck( "cds" );
			
			std::vector< std::pair< LocalFileHeader, std::string > > lfs;
			lfs.reserve( cds.size() );
			for ( std::size_t i = 0; i < cds.size(); ++i )
			{
				lfs.push_back( std::make_pair( LocalFileHeader(), std::string() ) );
				auto& pair = lfs.back();
				
				ss.seekg( cds[ i ].localHeaderOffset );
				pair.first = readLocalFileHeader( ss );
				if ( pair.first.flags & Flags::DataDescriptorPostponed and pair.first.compressType != Compression::Deflated )
				{
					throw std::runtime_error( "Post-data-descriptors not supported on non-deflate targets." );
				}
				else if ( pair.first.compressType != Compression::None and pair.first.compressType != Compression::Deflated )
				{
					throw std::runtime_error( "Unsupported compression type " + util::toString( pair.first.compressType ) + " at " + util::toString( ss.tellg() ) );//+ " for file " + pair.first.filename + "." );
				}
				
				if ( pair.first.compressType == Compression::Deflated )
				{
					readAndInflate( ss, pair.second );
					
					if ( pair.first.flags & Flags::DataDescriptorPostponed )
					{
						pair.first.crc32 = read< sf::Uint32 >( ss );
						if ( std::string( reinterpret_cast< char* >( &crc32 ), 4 ) == makeSig( DataDescriptor::SIGNATURE ) )
						{
							pair.first.crc32 = read< sf::Uint32 >( ss );
						}
						
						pair.first.sizeCompressed = read< sf::Uint32 >( ss );
						pair.first.sizeNormal = read< sf::Uint32 >( ss );
					}
				}
				else
				{
					pair.second = readStr( ss, pair.first.sizeCompressed );
				}
				
				if ( ss )
				{
					if ( pair.first.filename[ pair.first.filename.length() - 1 ] == '/' )
					{
						addDirectory( pair.first.filename.substr( 0, pair.first.filename.length() - 1 ) );
					}
					else
					{
						addFile( pair.first.filename, pair.second );
					}
				}
			}
			streamCheck( "lfs" );
		}
		catch ( std::exception& exception )
		{
			print( "Error loading zip file, exception: " << exception.what() << std::endl );
			return false;
		}
		
		return true;
	}
	
	void File::addFile( const std::string& path, const std::string& contents )
	{
		Entry* entry = getEntry( path );
		if ( entry == NULL )
		{
			entry = createEntryAt( path );
		}
		
		entry->children.clear();
		entry->file = this;
		entry->parent = getEntry( getParent( path ) );
		entry->name = getName( path );
		entry->contents = contents;
		entry->dir = false;
	}
	
	void File::addDirectory( const std::string& path )
	{
		Entry* entry = getEntry( path );
		if ( entry == NULL )
		{
			entry = createEntryAt( path );
		}
		
		//entry->children.clear();
		entry->file = this;
		entry->parent = getEntry( getParent( path ) );
		entry->name = getName( path );
		entry->dir = true;
	}
	
	std::string File::getParent( const std::string& path )
	{
		std::vector< std::string > tokens = util::tokenize( path , "/" );
		if ( tokens.size() == 1 )
		{
			return path;
		}
		else if ( tokens.size() == 2 )
		{
			return tokens[ 0 ];
		}
		
		std::string parent;
		for ( auto it = tokens.begin(); it != tokens.end() - 1; ++it )
		{
			parent += ( * it );
			if ( it != tokens.end() - 2 )
			{
				parent += '/';
			}
		}
		
		return parent;
	}
	
	std::string File::getName( const std::string& path )
	{
		std::vector< std::string > tokens = util::tokenize( path , "/" );
		return tokens[ tokens.size() - 1 ];
	}
	
	Entry* File::createEntryAt( const std::string& path )
	{
		priv::EntryBase* entry = this;
		if ( getParent( path ) != path )
		{
			std::string parent = getParent( path );
			Entry* e = getEntry( parent );
			if ( e == NULL )
			{
				e = createEntryAt( parent );
				e->file = this;
				e->parent = getEntry( getParent( parent ) );
				e->name = getName( parent );
				e->dir = true;
			}
			entry = e;
		}
		
		entry->children.emplace_back( new Entry() );
		return entry->children.back().get();
	}
	
	void File::readAndInflate( std::stringstream& ss, std::string& contents )
	{
		std::size_t beforePos = ss.tellg();
		
		z_stream stream;
		stream.zalloc = Z_NULL;
		stream.zfree = Z_NULL;
		stream.opaque = Z_NULL;
		stream.next_in = Z_NULL;
		stream.avail_in = 0;
		
		int ret = inflateInit2( &stream, -15 ); // -15 == use raw deflate data, not header/check values. Took me a while to find that :P
		if ( ret != Z_OK )
		{
			throw std::runtime_error( "Failed to init zlib inflate." );
		}
		
		class InflateEnder
		{
			public:
				InflateEnder( z_stream& theStream )
				   : stream( theStream )
				{
				}
				
				~InflateEnder()
				{
					inflateEnd( &stream );
				}
			
			private:
				z_stream& stream;
		} ie( stream );
		
		constexpr std::size_t BUFFER_SIZE = 16;
		unsigned char bufferIn[ BUFFER_SIZE ];
		unsigned char bufferOut[ BUFFER_SIZE ];
		do
		{
			ss.read( reinterpret_cast< char* >( bufferIn ), BUFFER_SIZE );
			stream.avail_in = ss.gcount();
			if ( ss.bad() )
			{
				throw std::runtime_error( "Some error with reading the stream?" );
			}
			
			if ( stream.avail_in == 0 )
			{
				break;
			}
			
			stream.next_in = bufferIn;
			
			do
			{
				stream.avail_out = BUFFER_SIZE;
				stream.next_out = bufferOut;
				ret = inflate( &stream, Z_NO_FLUSH );
				
				switch ( ret )
				{
					case Z_NEED_DICT:
						//ret = Z_DATA_ERROR;
					case Z_DATA_ERROR:
					case Z_MEM_ERROR:
					case Z_STREAM_ERROR:
						throw std::runtime_error( "Some error with inflate: " + util::toString( ret ) );
				}
				
				unsigned int have = BUFFER_SIZE - stream.avail_out;
				contents += std::string( reinterpret_cast< char* >( bufferOut ), have );
			}
			while ( stream.avail_out == 0 );
		}
		while ( ret != Z_STREAM_END );
		
		ss.seekg( beforePos + stream.total_in );
	}
}
