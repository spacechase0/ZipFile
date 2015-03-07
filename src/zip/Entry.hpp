#ifndef ZIP_ENTRY_HPP
#define ZIP_ENTRY_HPP

#include <string>

#include "zip/EntryBase.hpp"

namespace zip
{
	class File;
	
	class Entry : public priv::EntryBase
	{
		public:
			File* getFile();
			const File* getFile() const;
			
			Entry* getParent();
			const Entry* getParent() const;
			
			bool isFile() const;
			bool isDirectory() const;
			
			std::string getName() const;
			std::string getContents() const;
		
		private:
			Entry();
			
			File* file;
			Entry* parent;
			std::string name;
			std::string contents;
			bool dir;
			
			friend class File;
	};
}

#endif // ZIP_ENTRY_HPP
