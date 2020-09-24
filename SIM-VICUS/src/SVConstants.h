#ifndef SVConstantsH
#define SVConstantsH

#include <QAbstractItemDelegate>

/*! To be used as institution/organization in SVSettings. */
extern const char * const ORG_NAME;
/*! Program name, also used as registry/settings key for saving the user
	defined settings and the MRU list.
	To be used as application name in SVSettings.
*/
extern const char * const PROGRAM_NAME;
/*! E-mail address for support requests. */
extern const char * const SUPPORT_EMAIL;

/*! URL for update info file. */
extern const char * const UPDATE_FILE_URL;
/*! URL for news file. */
extern const char * const NEWS_FILE_URL;
/*! URL for bug reports. */
extern const char * const BUG_REPORT_URL;
/*! URL for discussion forum. */
extern const char * const FORUM_URL;


/*! Fixed-size font family to be used in tables. */
extern const char * const FIXED_FONT_FAMILY;

extern const char * const EditFieldBackground;
extern const char * const AlternativeEditFieldBackground;
extern const char * const ErrorEditFieldBackground;
extern const char * const AlternativeErrorEditFieldBackground;
extern const char * const ReadOnlyEditFieldBackground;

extern const char * const AlternativeBackgroundBright;
extern const char * const AlternativeBackgroundDark;
extern const char * const AlternativeBackgroundText;

/*! \file SVConstants.h
	Contains constants which are used in several ui files. This is to
	avoid multiple specifications of constants or even worse, the use of
	C defines.
*/

#endif  // SVConstantsH