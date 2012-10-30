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

#include "zip/EntryBase.hpp"

#include <util/Tokenize.h>

#include "zip/Entry.hpp"

namespace zip
{
	namespace priv
	{
		EntryBase::Iterator EntryBase::begin()
		{
			return children.begin();
		}
		
		EntryBase::Iterator EntryBase::end()
		{
			return children.end();
		}
		
		EntryBase::ConstIterator EntryBase::begin() const
		{
			return children.begin();
		}
		
		EntryBase::ConstIterator EntryBase::end() const
		{
			return children.end();
		}
		
		// http://stackoverflow.com/questions/856542/elegant-solution-to-duplicate-const-and-non-const-getters
		// This was more important when most of getEntryIterator() was in the const getEntry() :P
		Entry* EntryBase::getEntry( const std::string& path )
		{
			return const_cast< Entry* >( static_cast< const EntryBase* >( this )->getEntry( path ) );
		}
		
		const Entry* EntryBase::getEntry( const std::string& path ) const
		{
			ConstIterator it = getEntryIterator( path );
			return ( ( it == end() ) ? NULL : it->get() );
		}
		
		EntryBase::ConstIterator EntryBase::getEntryIterator( const std::string& path ) const
		{
			std::vector< std::string > tokens = util::tokenize( path, "/" );
			for ( auto it = tokens.begin(); it != tokens.end(); ++it )
			{
				if ( it->empty() )
				{
					tokens.erase( it );
					it = tokens.begin() - 1;
					continue;
				}
			}
			
			std::size_t currToken = 0;
			auto currEnd = end();
			for ( auto it = begin(); it != currEnd; ++it )
			{
				Entry* entry = it->get();
				if ( entry->getName() == tokens[ currToken ] )
				{
					if ( currToken >= tokens.size() - 1 )
					{
						return it;
					}
					else if ( entry->isDirectory() and entry->children.size() >= 1 )
					{
						it = entry->begin() - 1;
						currEnd = entry->end();
						++currToken;
						continue;
					}
					else
					{
						return end();
					}
				}
			}
			
			return end();
		}
	}
}
