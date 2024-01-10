#ifndef IO_UTIL_H
#define IO_UTIL_H

#include <fstream>
#include "Type.h"

namespace Util
{
	struct IO
	{
		static Vector<char> ReadFile(const String& filename);

		static unsigned char* ReadImage(i32 &width, i32 &height, const String& filename);

	};
}


#endif