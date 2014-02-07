#ifndef PEPTIDECONSTANTS_HPP
#define PEPTIDECONSTANTS_HPP

namespace peptide {

    namespace Constants {

        const char ACTION_ID[]        = "peptide.Action";
        const char MENU_ID[]          = "peptide.Menu";
        const char C_PEPTIDE_MODE[] = "peptide.Mode";

        // common actions
        const char * const FILE_OPEN          = "peptide.FileOpen";
        //const char * const METHOD_SAVE          = "peptide.MethodSave";
        //const char * const METHOD_APPLY         = "peptide.MethodApply";
        //const char * const PRINT_CURRENT_VIEW   = "peptide.PrintCurrentView";

        // icon
        const char * const ICON_FILE_OPEN       = ":/dataproc/image/fileopen.png";

        enum peptide_columns {
            c_peptide_filename
            , c_peptide_process
            , c_peptide_state
            , c_peptide_progress
            , c_peptide_num_columns
        };

    } // namespace Constants

} 

#endif // PEPTIDECONSTANTS_HPP

