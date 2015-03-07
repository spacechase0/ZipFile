#include "zip/EntryBase.hpp"

#include <util/String.hpp>

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
