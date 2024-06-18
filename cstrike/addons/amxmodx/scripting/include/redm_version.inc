#if defined _redm_version_included
	#endinput
#endif
#define _redm_version_included

// redm version
#define REDM_VERSION_MAJOR 1
#define REDM_VERSION_MINOR 0
#define REDM_VERSION_PATCH "manual"

#define REDM_VERSION fmt("%i.%i.%s", REDM_VERSION_MAJOR, REDM_VERSION_MINOR, REDM_VERSION_PATCH)
