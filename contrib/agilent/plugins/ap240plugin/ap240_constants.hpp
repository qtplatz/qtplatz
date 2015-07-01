#ifndef U5303CONSTANTS_HPP
#define U5303CONSTANTS_HPP

namespace ap240 {
namespace Constants {

    const char ACTION_ID[] = "ap240.Action";
    const char MENU_ID[] = "ap240.Menu";

    const char AP240_MODE[] = "ap240.Mode";

    // common actions
    const char * const FILE_OPEN         = "ap240.FileOpen";
    const char * const ACT_CONNECT       = "ap240.Connect";
    const char * const ACT_INITRUN       = "ap240.InitRun";
    const char * const ACT_RUN           = "ap240.Run";
    const char * const ACT_STOP          = "ap240.Stop";
    const char * const ACT_INJECT        = "ap240.Inject";
    const char * const ACT_SNAPSHOT      = "ap240.Snapshot";

    // icon
    const char * const ICON_FILE_OPEN       = ":/dataproc/image/fileopen.png";
    const char * const ICON_CONNECT = ":/acquire/images/Button Refresh.png";
    const char * const ICON_INITRUN = ":/acquire/images/Button Last.png";
    const char * const ICON_RUN     = ":/acquire/images/Button Play.png";
    const char * const ICON_STOP    = ":/acquire/images/Button Stop.png";
    const char * const ICON_INJECT  = ":/acquire/images/Button Add.png";
    const char * const ICON_SNAPSHOT = ":/acquire/images/snapshot_small.png";

    // settings
    const char * const GRP_DATA_FILES       = "DataFiles";
    const char * const GRP_METHOD_FILES     = "MethodFiles";
    const char * const KEY_FILES            = "Files";

} // namespace u5303
} // namespace Constants

#endif // U5303CONSTANTS_HPP

