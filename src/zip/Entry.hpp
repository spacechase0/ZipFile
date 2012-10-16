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
