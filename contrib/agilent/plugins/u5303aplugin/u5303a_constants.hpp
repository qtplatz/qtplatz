#ifndef U5303CONSTANTS_HPP
#define U5303CONSTANTS_HPP

namespace u5303a {
namespace Constants {

    const char ACTION_ID[] = "u5303a.Action";
    const char MENU_ID[] = "u5303a.Menu";

    const char U5303A_MODE[] = "u5303a.Mode";

    // common actions
    const char * const FILE_OPEN          = "u5303a.FileOpen";
    const char * const ACT_CONNECT       = "u5303a.Connect";
    const char * const ACT_INITRUN       = "u5303a.InitRun";
    const char * const ACT_RUN           = "u5303a.Run";
    const char * const ACT_STOP          = "u5303a.Stop";
    const char * const ACT_INJECT        = "u5303a.Inject";
    const char * const ACT_SNAPSHOT      = "u5303a.Snapshot";

    // icon
    const char * const ICON_FILE_OPEN       = ":/dataproc/image/fileopen.png";
    const char * const ICON_CONNECT = ":/acquire/images/Button Refresh.png";
    const char * const ICON_INITRUN = ":/acquire/images/Button Last.png";
    const char * const ICON_RUN     = ":/acquire/images/Button Play.png";
    const char * const ICON_STOP    = ":/acquire/images/Button Stop.png";
    const char * const ICON_INJECT  = ":/acquire/images/Button Add.png";
    const char * const ICON_SNAPSHOT = ":/acquire/images/snapshot_small.png";

} // namespace u5303
} // namespace Constants

#endif // U5303CONSTANTS_HPP

