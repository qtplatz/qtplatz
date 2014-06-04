#ifndef BATCHPROCCONSTANTS_HPP
#define BATCHPROCCONSTANTS_HPP

namespace batchproc {

    namespace Constants {

        const char * const ACTION_ID        = "batchproc.Action";
        const char * const MENU_ID          = "batchproc.Menu";
        const char * const C_BATCHPROC_MODE = "batchproc.Mode";

        enum batchproc_columns {
            c_batchproc_filename
            , c_batchproc_process
            , c_batchproc_state
            , c_batchproc_progress
            , c_batchproc_num_columns
        };

    } // namespace Constants

} 

#endif // BATCHPROCCONSTANTS_HPP

