#include "zip/Entry.hpp"

namespace zip
{
	File* Entry::getFile()
	{
		return file;
	}
	
	const File* Entry::getFile() const
	{
		return file;
	}
	
	Entry* Entry::getParent()
	{
		return parent;
	}
	
	const Entry* Entry::getParent() const
	{
		return parent;
	}
	
	bool Entry::isFile() const
	{
		return !isDirectory();
	}
	
	bool Entry::isDirectory() const
	{
		return dir;
	}
	
	std::string Entry::getName() const
	{
		return name;
	}
	
	std::string Entry::getContents() const
	{
		return contents;
	}
	
	Entry::Entry()
	   : file( NULL ),
	     parent( NULL ),
	     dir( false )
	{
	}
}
