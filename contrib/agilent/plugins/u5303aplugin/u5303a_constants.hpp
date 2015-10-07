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
    ////////////////////////
    const char * const ACTION_CONNECT       = "u5303a.Connect";    
    const char * const ACTION_RUN           = "u5303a.Run";
    const char * const ACTION_STOP          = "u5303a.Stop";
    const char * const ACTION_REC           = "u5303a.Rec";
    const char * const ACTION_SNAPSHOT      = "u5303a.Snapshot";
    const char * const ACTION_SYNC          = "u5303a.SyncTrig";
    const char * const PRINT_CURRENT_VIEW   = "u5303a.print_current_view";
    const char * const SAVE_CURRENT_IMAGE   = "u5303a.save_current_image";

    // icon
    const char * const ICON_FILE_OPEN       = ":/dataproc/image/fileopen.png";
    const char * const ICON_INITRUN         = ":/acquire/images/Button Last.png";
    //const char * const ICON_RUN           = ":/acquire/images/Button Play.png";
    //const char * const ICON_STOP          = ":/acquire/images/Button Stop.png";
    const char * const ICON_INJECT          = ":/acquire/images/Button Add.png";
    //const char * const ICON_SNAPSHOT      = ":/acquire/images/snapshot_small.png";

    // icon
    const char * const ICON_CONNECT         = ":/u5303a/images/control_power_blue.png";
    const char * const ICON_RUN             = ":/u5303a/images/control.png";
    const char * const ICON_STOP            = ":/u5303a/images/control_stop.png";
    const char * const ICON_REC_ON          = ":/u5303a/images/control_record.png";
    const char * const ICON_REC_PAUSE       = ":/u5303a/images/control-pause-record.png";
    const char * const ICON_SNAPSHOT        = ":/u5303a/images/camera.png";
    const char * const ICON_SYNC            = ":/u5303a/images/arrow-circle-double.png";
    const char * const ICON_FOLDER_OPEN     = ":/u5303a/images/folder-horizontal-open.png";
    const char * const ICON_DISK_PLUS       = ":/u5303a/images/disk--plus.png";
    const char * const ICON_PDF             = ":/u5303a/images/document-pdf.png";
    const char * const ICON_IMAGE           = ":/u5303a/images/image.png";
    ///////////////////////////////////////////

    // settings
    const char * const GRP_DATA_FILES       = "DataFiles";
    const char * const GRP_METHOD_FILES     = "MethodFiles";
    const char * const KEY_FILES            = "Files";

} // namespace u5303
} // namespace Constants

#endif // U5303CONSTANTS_HPP

