#ifndef XORGGTEST_DEFINES
#define XORGGTEST_DEFINES

#define DEFAULT_XORG_LOGFILE LOGFILE_DIR "/Xorg.GTest.log"
#define DEFAULT_DISPLAY 133

/* Allow user to override default Xorg server*/
#ifndef DEFAULT_XORG_SERVER
#define DEFAULT_XORG_SERVER "Xorg"
#endif

#endif
