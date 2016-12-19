#ifndef DATAPROCCONSTANTS_HPP
#define DATAPROCCONSTANTS_HPP

#include <adcontrols/constants.hpp>

namespace dataproc {
namespace Constants {

    const char ACTION_ID[] = "Dataproc.Action";
    const char MENU_ID[] = "Dataproc.Menu";

    const char * const C_DATAPROCESSOR        = "Dataprocessor";
    const char * const C_DATAPROC_MODE        = "Dataproc.Mode";
    const char * const C_DATAPROC_NAVI        = "Dataproc.Navigator";
    const char * const ABOUT_QTPLATZ          = "Dataproc.AboutQtPlatz";
    const char * const C_DATA_TEXT_MIMETYPE   = "application/txt";
    const char * const C_DATA_NATIVE_MIMETYPE = "application/adfs";

    // common actions
    const char * const METHOD_OPEN          = "dataproc.MethodOpen";
    const char * const METHOD_SAVE          = "dataproc.MethodSave";
    const char * const METHOD_APPLY         = "dataproc.MethodApply";
    const char * const PRINT_CURRENT_VIEW   = "dataproc.PrintCurrentView";
    const char * const CALIBFILE_APPLY      = "dataproc.ApplyCalibration";
    const char * const PROCESS_ALL_CHECKED  = "dataproc.ProcessAllCheckedSpectra";
    const char * const IMPORT_ALL_CHECKED   = "dataproc.ImportAllCheckedSpectra";
    const char * const LISTPEAKS_ON_CHECKED = "dataproc.PeakListAllChecked";
    const char * const HIDE_DOCK            = "dataproc.HideDock";

    const char * const CREATE_SPECTROGRAM   = "dataproc.Spectrogram";
    const char * const CLUSTER_SPECTROGRAM  = "dataproc.ClusterSpectrogram";

    // edit
    const char * const CHECK_ALL_SPECTRA   = "dataproc.Edit.CheckAllSpectra";
    const char * const UNCHECK_ALL_SPECTRA = "dataproc.Edit.UncheckAllSpectra";

    // icon
    const char * const ICON_METHOD_SAVE     = ":/dataproc/image/filesave.png";
    const char * const ICON_METHOD_OPEN     = ":/dataproc/image/fileopen.png";
    const char * const ICON_SAVE            = ":/dataproc/image/filesave.png";
    const char * const ICON_OPEN            = ":/dataproc/image/fileopen.png";
    const char * const ICON_METHOD_APPLY    = ":/dataproc/image/apply_small.png";
    const char * const ICON_PDF             = ":/dataproc/image/file_pdf.png"; // http://findicons.com/icon/74870/file_pdf?id=355001
    const char * const ICON_CALIBFILE       = ":/dataproc/image/calibration32.png";
    const char * const ICON_DOCKHIDE        = ":/dataproc/image/button_close.png";
    const char * const ICON_DOCKSHOW        = ":/dataproc/image/control-090-small.png";
    // freeware license, Designed by Andy Gongea
    // Folium (attachment) name
    const wchar_t * const F_DFT_FILTERD        = adcontrols::constants::F_DFT_FILTERD;
    const wchar_t * const F_CENTROID_SPECTRUM  = adcontrols::constants::F_CENTROID_SPECTRUM;
    const wchar_t * const F_MSPEAK_INFO        = adcontrols::constants::F_MSPEAK_INFO;
    const wchar_t * const F_TARGETING          = adcontrols::constants::F_TARGETING;
    const wchar_t * const F_PROFILED_HISTOGRAM = adcontrols::constants::F_PROFILED_HISTOGRAM;

    // settings
    const char * const GRP_DATA_FILES       = "DataFiles";
    const char * const GRP_METHOD_FILES     = "MethodFiles";
    const char * const KEY_FILES            = "Files";

    const char * const GRP_SPECTRUM_IMAGE   = "SpectrumImage";
    const char * const KEY_IMAGEE_FORMAT    = "ImageFormat"; // pdf|svg|ps
    const char * const KEY_COMPRESS         = "VectorCompression"; // true|false
    const char * const KEY_DPI              = "DPI";

    // shared with Quan
    const wchar_t * const F_QUANSAMPLE         = L"QuanSample";

    // QObject names
    const char * const EDIT_PROCMETHOD    = "dataproc.MainWindow.procmethodname";

} // Constants

    enum ProcessType {
        CentroidProcess
        , TargetingProcess
        , CalibrationProcess
        , PeakFindProcess 
    };

} // namespace dataproc

#endif // DATAPROCCONSTANTS_HPP

