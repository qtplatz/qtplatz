
#pragma once

#include <adcontrols/constants.hpp>

namespace lipidid {
namespace Constants {

    const char ACTION_ID[] = "lipidid.Action";
    const char MENU_ID[] = "lipidid.Menu";

    const char * const C_LIPIDID              = "lipidid";
    const char * const C_LIPIDID_MODE         = "lipidid.Mode";
    const char * const C_LIPIDID_NAVI         = "lipidid.Navigator";
    // const char * const C_DATA_TEXT_MIMETYPE   = "application/txt";
    // const char * const C_DATA_NATIVE_MIMETYPE = "application/adfs";

    // common actions
    const char * const FIND_ALL             = "lipidid.FindAll";
    const char * const SAVE_ALL             = "lipidid.SaveAll";
    const char * const PRINT_CURRENT_VIEW   = "lipidid.PrintCurrentView";
    const char * const LISTPEAKS_ON_CHECKED = "lipidid.PeakListAllChecked";
    const char * const EXPORT_ALL_CHECKED   = "lipidid.ExportAllChecked";
    const char * const HIDE_DOCK            = "lipidid.HideDock";
    const char * const SDF_IMPORT           = "lipidid.SDFImport";

    const char * const CREATE_SPECTROGRAM   = "lipidid.Spectrogram";
    const char * const CLUSTER_SPECTROGRAM  = "lipidid.ClusterSpectrogram";

    // edit
    const char * const CHECK_ALL_SPECTRA   = "lipidid.Edit.CheckAllSpectra";
    const char * const UNCHECK_ALL_SPECTRA = "lipidid.Edit.UncheckAllSpectra";

    // icon
    const char * const ICON_METHOD_SAVE     = ":/dataproc/image/filesave.png";
    const char * const ICON_METHOD_OPEN     = ":/dataproc/image/fileopen.png";
    const char * const ICON_SAVE            = ":/dataproc/image/filesave.png";
    const char * const ICON_OPEN            = ":/dataproc/image/fileopen.png";
    const char * const ICON_METHOD_APPLY    = ":/dataproc/image/apply_small.png";
    const char * const ICON_PDF             = ":/dataproc/image/file_pdf.png";
    //const char * const ICON_CALIBFILE       = ":/lipidid/image/calibration32.png";
    const char * const ICON_DOCKHIDE        = ":/dataproc/image/button_close.png";
    const char * const ICON_DOCKSHOW        = ":/dataproc/image/control-090-small.png";
    // freeware license, Designed by Andy Gongea
    // Folium (attachment) name
    const wchar_t * const F_DFT_FILTERD        = adcontrols::constants::F_DFT_FILTERD;
    const wchar_t * const F_CENTROID_SPECTRUM  = adcontrols::constants::F_CENTROID_SPECTRUM;
    const wchar_t * const F_MSPEAK_INFO        = adcontrols::constants::F_MSPEAK_INFO;
    const wchar_t * const F_TARGETING          = adcontrols::constants::F_TARGETING;
    const wchar_t * const F_PROFILED_HISTOGRAM = adcontrols::constants::F_PROFILED_HISTOGRAM;
    const wchar_t * const F_PEAKRESULT         = adcontrols::constants::F_PEAKRESULT;

    // settings
    const char * const GRP_DATA_FILES       = "DataFiles";
    const char * const GRP_METHOD_FILES     = "MethodFiles";
    const char * const KEY_FILES            = "Files";
    const char * const GRP_EXPORT_FILES     = "ExportFiles";

    const char * const GRP_SPECTRUM_IMAGE   = "SpectrumImage";
    const char * const KEY_IMAGEE_FORMAT    = "ImageFormat"; // pdf|svg|ps
    const char * const KEY_COMPRESS         = "VectorCompression"; // true|false
    const char * const KEY_DPI              = "DPI";
    const char * const KEY_IMAGE_SAVE_DIR   = "ImageSaveDir";

    // QObject names
    const char * const EDIT_PROCMETHOD    = "lipidid.MainWindow.procmethodname";

    const char * const LIPIDID_TASK_SDFIMPORT = "lipidid.task.sdfimport";
    const char * const LIPIDID_TASK_FIND_ALL  = "lipidid.task.find_all";
    //
    const char * const THIS_GROUP           = "lipidid";

} // Constants
} // namespace lipidid
