/*!
	@file
	@author		George Evmenov
	@date		07/2009
*/

#ifndef COMPONENTS_MYGUI_OSG_DIAGNOSTIC_H
#define COMPONENTS_MYGUI_OSG_DIAGNOSTIC_H

#include "MYGUI/MyGUI_Prerequest.h"

#define MYGUI_PLATFORM_LOG_SECTION "Platform"
#define MYGUI_PLATFORM_LOG_FILENAME "MyGUI.log"
#define MYGUI_PLATFORM_LOG(level, text) MYGUI_LOGGING(MYGUI_PLATFORM_LOG_SECTION, level, text)

#define MYGUI_PLATFORM_EXCEPT(dest) do { \
	MYGUI_PLATFORM_LOG(Critical, dest); \
	MYGUI_DBG_BREAK;\
	std::ostringstream stream; \
	stream << dest << "\n"; \
	MYGUI_BASE_EXCEPT(stream.str().c_str(), "MyGUI"); \
} while(0)

#define MYGUI_PLATFORM_ASSERT(exp, dest) do { \
	if ( ! (exp) ) \
	{ \
		MYGUI_PLATFORM_LOG(Critical, dest); \
		MYGUI_DBG_BREAK;\
		std::ostringstream stream; \
		stream << dest << "\n"; \
		MYGUI_BASE_EXCEPT(stream.str().c_str(), "MyGUI"); \
	} \
} while(0)

#endif /* COMPONENTS_MYGUI_OSG_DIAGNOSTIC_H */
