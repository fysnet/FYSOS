/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * ids.h
 *  
 *  This file contains all the ID_ resources for all files.  It is separated so that
 *   it can be used in other utilities.
 *
 *  This must remain as #define's instead of enum's so that we can easily find the
 *   actual value of the ID_ resource instead of having to count it each time.
 *  
 *  All ID_ resources should remain in numerical order so to not accidentally add
 *   conflicting values, and should all be decimal numbers.
 *
 */
#ifndef FYSOS_IDS
#define FYSOS_IDS

// these ID values must be at least larger than
//  the value of the last EVENT ID, EVENT LAST.

// standard ICON images
#define  ID_ICON_ARROW_UP            1000
#define  ID_ICON_ARROW_DOWN          1001
#define  ID_ICON_ARROW_LEFT          1002
#define  ID_ICON_ARROW_RIGHT         1003
#define  ID_ICON_SYS_UP              1004
#define  ID_ICON_SYS_DOWN            1005
#define  ID_ICON_SYS_LEFT            1006
#define  ID_ICON_SYS_RIGHT           1007
#define  ID_ICON_FOLDER_OPEN         1008
#define  ID_ICON_FOLDER_CLOSED       1009
#define  ID_ICON_COPY                1010
#define  ID_ICON_DELETE              1011
#define  ID_ICON_DOCUMENT            1012
#define  ID_ICON_MAIL                1013
#define  ID_ICON_PROMPT              1014
#define  ID_ICON_TRASH               1015
#define  ID_ICON_CLOSE               1016
  
#define  ID_ICON_CAUTION             2000
#define  ID_ICON_CAUTION_SM          2001
#define  ID_ICON_INFO                2002
#define  ID_ICON_QUESTION            2003
#define  ID_ICON_STOP                2004
#define  ID_ANIMATE_DEMO             2005

#define  ID_ICON_CALCULATOR          2010
#define  ID_ICON_SETTINGS            2011
#define  ID_ICON_MINESWEEP           2012
#define  ID_ICON_TASKMAN             2013
#define  ID_ICON_MEDIA_PLAYER        2014
#define  ID_ICON_HARDWARE            2015

#define  ID_ICON_BUSY                2050

#define  ID_ICON_USER_ADD            2100

#define  ID_ICON_TEXT                2200



#define  ID_BUTTON_MORE              2950
#define  ID_BUTTON_DROP_DN           2951
#define  ID_BUTTON_DROP_UP           2952
#define  ID_BUTTON_PREV              2953
#define  ID_BUTTON_NEXT              2954


  // standard FILE menu Items
#define  ID_FILE_NEW                 3000
#define  ID_FILE_OPEN                3001
#define  ID_FILE_SAVEAS              3002
#define  ID_FILE_SAVE                3003
#define  ID_FILE_PAGE_SETUP          3004
#define  ID_FILE_PRINT               3005
#define  ID_FILE_RECENT              3007
#define  ID_FILE_EXIT                3006
#define  ID_RECENT_FILE_0            3040
#define  ID_RECENT_FILE_1            3041
#define  ID_RECENT_FILE_2            3042
#define  ID_RECENT_FILE_3            3043
#define  ID_RECENT_FILE_4            3044
#define  ID_RECENT_FILE_5            3045

  // standard EDIT menu Items
#define  ID_EDIT_CUT                 3050
#define  ID_EDIT_COPY                3051
#define  ID_EDIT_PASTE               3052
#define  ID_EDIT_DELETE              3053
#define  ID_EDIT_SELECTALL           3054
#define  ID_EDIT_FIND                3055
#define  ID_EDIT_REPLACE             3056
#define  ID_EDIT_GOTO                3057
  
  // standard HELP menu Items
#define  ID_HELP_ABOUT               3400
#define  ID_HELP_HELP                3401
  
#define  ID_BUTTON_BAR_DIV           3450
#define  ID_BUTTON_UP                3451
#define  ID_BUTTON_DOWN              3452

#define  ID_GREEN_BALL               3500
#define  ID_ON_OFF                   3501
#define  ID_LIST_PLUS_MINUS          3502


  // file extension id types
#define  ID_TYPE_UNKNOWN             4000
#define  ID_TYPE_DRIVE               4001
#define  ID_TYPE_FLOPPY              4002
#define  ID_TYPE_CDROM               4003
#define  ID_TYPE_RAM                 4004
#define  ID_TYPE_DIR                 4005
#define  ID_TYPE_ASM                 4006
#define  ID_TYPE_BAT                 4007
#define  ID_TYPE_C                   4008
#define  ID_TYPE_COM                 4009
#define  ID_TYPE_CPP                 4010
#define  ID_TYPE_EXE                 4011
#define  ID_TYPE_FNT                 4012
#define  ID_TYPE_H                   4013
#define  ID_TYPE_IMG                 4014
#define  ID_TYPE_SYS                 4015
#define  ID_TYPE_TXT                 4016
#define  ID_TYPE_ZIP                 4017

  // hardware id types
#define  ID_TYPE_USB                 4100
#define  ID_TYPE_CPU                 4101
#define  ID_TYPE_KEYBRD              4102
#define  ID_TYPE_MOUSE               4103
#define  ID_TYPE_FDD                 4104
#define  ID_TYPE_ATA                 4105
#define  ID_TYPE_GAME                4106
#define  ID_TYPE_SERIAL              4107
#define  ID_TYPE_PARA                4108
#define  ID_TYPE_SPKR                4109
#define  ID_TYPE_TIMER               4110

  // standard Border items
#define  ID_BORDER_WIN              10000
#define  ID_BORDER_CLOSE            10001
#define  ID_BORDER_MIN              10002
#define  ID_BORDER_MAX              10003
#define  ID_BORDER_RESIZE           10004
#define  ID_ICON_RESIZE             10005

#define  ID_SCROLL_BACK             10020
#define  ID_SCROLL_LEFT             10021
#define  ID_SCROLL_RIGHT            10022
#define  ID_SCROLL_UP               10023
#define  ID_SCROLL_DOWN             10024

#define  ID_SYSTEM_MENU             10030
#define  ID_TITLE_BAR               10031
#define  ID_TASKBAR_UP              10032
#define  ID_TASKBAR_DOWN            10033

  // used in Message Boxes for return
#define  ID_MB_OKAY                 10100
#define  ID_MB_CANCEL               10101
#define  ID_MB_RETRY                10102
#define  ID_MB_YES                  10103
#define  ID_MB_NO                   10104


// calendar days
#define  CLD_START                  20001
                // 20001 -> 20031 used up
#define  CLD_END                    20031
#define  ID_TODAY                   20132

// Mine Sweep (must stay in this order)
#define  ID_MS_BLANK                20200
#define  ID_MS_ONE                  20201
#define  ID_MS_TWO                  20202
#define  ID_MS_THREE                20203
#define  ID_MS_FOUR                 20204
#define  ID_MS_FIVE                 20205
#define  ID_MS_SIX                  20206
#define  ID_MS_SEVEN                20207
#define  ID_MS_EIGHT                20208
#define  ID_MS_BOMB                 20209

#define  ID_MS_BLANK_TAG            20210
#define  ID_MS_ONE_TAG              20211
#define  ID_MS_TWO_TAG              20212
#define  ID_MS_THREE_TAG            20213
#define  ID_MS_FOUR_TAG             20214
#define  ID_MS_FIVE_TAG             20215
#define  ID_MS_SIX_TAG              20216
#define  ID_MS_SEVEN_TAG            20217
#define  ID_MS_EIGHT_TAG            20218
#define  ID_MS_BOMB_TAG             20219

#define  ID_MS_BOMB_RED             20220
#define  ID_MS_RESET                20221

// seven segment digits
#define  ID_SEVENSEG_ZERO           20230
#define  ID_SEVENSEG_ONE            20231
#define  ID_SEVENSEG_TWO            20232
#define  ID_SEVENSEG_THREE          20233
#define  ID_SEVENSEG_FOUR           20234
#define  ID_SEVENSEG_FIVE           20235
#define  ID_SEVENSEG_SIX            20236
#define  ID_SEVENSEG_SEVEN          20237
#define  ID_SEVENSEG_EIGHT          20238
#define  ID_SEVENSEG_NINE           20239
#define  ID_SEVENSEG_NEG            20240



#endif   // FYSOS_IDS
