#include  "SVConstants.h"

const char * const ORG_NAME			= "IBK";

const char * const UPDATE_FILE_URL	= "...";
const char * const NEWS_FILE_URL	= "...";
const char * const BUG_REPORT_URL	= "...";
const char * const FORUM_URL		= "...";
const char * const MANUAL_URL		= "https://ghorwin.github.io/SIM-VICUS";

const char * const THIRD_LANGUAGE	= "fr";

const char * const SUPPORT_EMAIL	= "...";

const double SAME_DISTANCE_PARAMETER_ABSTOL = 1e-4;


#if defined(Q_OS_MAC) // Q_OS_UNIX

const char * const FIXED_FONT_FAMILY = "Monaco";

#elif defined(Q_OS_UNIX)

const char * const FIXED_FONT_FAMILY = "Monospace";

#else

const char * const FIXED_FONT_FAMILY = "Courier New";

#endif

