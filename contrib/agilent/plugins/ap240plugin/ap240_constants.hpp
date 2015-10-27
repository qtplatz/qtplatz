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

    const char * const ACTION_CONNECT       = "ap240.Connect";    
    const char * const ACTION_RUN           = "ap240.Run";
    const char * const ACTION_STOP          = "ap240.Stop";
    const char * const ACTION_REC           = "ap240.Rec";
    const char * const ACTION_SNAPSHOT      = "ap240.Snapshot";
    const char * const ACTION_SYNC          = "ap240.SyncTrig";
    const char * const PRINT_CURRENT_VIEW   = "ap240.print_current_view";
    const char * const SAVE_CURRENT_IMAGE   = "ap240.save_current_image";
    const char * const HIDE_DOCK            = "ap240.HideDock";
    // icon
    const char * const ICON_FILE_OPEN       = ":/dataproc/image/fileopen.png";
    const char * const ICON_INITRUN         = ":/acquire/images/Button Last.png";
    const char * const ICON_INJECT          = ":/acquire/images/Button Add.png";
    //
    const char * const ICON_DOCKHIDE        = ":/dataproc/image/button_close.png";
    const char * const ICON_DOCKSHOW        = ":/dataproc/image/control-090-small.png";
    
    const char * const ICON_CONNECT         = ":/ap240/images/control_power_blue.png";
    const char * const ICON_RUN             = ":/ap240/images/control.png";
    const char * const ICON_STOP            = ":/ap240/images/control_stop.png";
    const char * const ICON_REC_ON          = ":/ap240/images/control_record.png";
    const char * const ICON_REC_PAUSE       = ":/ap240/images/control-pause-record.png";
    const char * const ICON_SNAPSHOT        = ":/ap240/images/camera.png";
    const char * const ICON_SYNC            = ":/ap240/images/arrow-circle-double.png";
    const char * const ICON_FOLDER_OPEN     = ":/ap240/images/folder-horizontal-open.png";
    const char * const ICON_DISK_PLUS       = ":/ap240/images/disk--plus.png";
    const char * const ICON_PDF             = ":/ap240/images/document-pdf.png";
    const char * const ICON_IMAGE           = ":/ap240/images/image.png";
    // settings
    const char * const GRP_DATA_FILES       = "DataFiles";
    const char * const GRP_METHOD_FILES     = "MethodFiles";
    const char * const KEY_FILES            = "Files";

} // namespace u5303
} // namespace Constants

#endif // U5303CONSTANTS_HPP

