#ifndef ZIP_ENTRYBASE_HPP
#define ZIP_ENTRYBASE_HPP

#include <memory>
#include <vector>

namespace zip
{
	class Entry;
	class File;
	
	namespace priv
	{
		class EntryBase
		{
			public:
				typedef std::vector< std::unique_ptr< Entry > >::iterator Iterator;
				typedef std::vector< std::unique_ptr< Entry > >::const_iterator ConstIterator;
				
				Iterator begin();
				Iterator end();
				ConstIterator begin() const;
				ConstIterator end() const;
				
				Entry* getEntry( const std::string& path );
				const Entry* getEntry( const std::string& path ) const;
				
			protected:
				std::vector< std::unique_ptr< Entry > > children;
				
				ConstIterator getEntryIterator( const std::string& path ) const;
				
				friend class zip::File; // Doing just "File" doesn't compile
		};
	}
}

#endif // ZIP_ENTRYBASE_HPP
