set(IDE_VERSION "18.0.82")                            # The IDE version.
set(IDE_VERSION_COMPAT "18.0.82")                     # The IDE Compatibility version.
set(IDE_VERSION_DISPLAY "19.0.0-beta1")               # The IDE display version.

set(IDE_SETTINGSVARIANT "QtProject")                  # The IDE settings variation.
set(IDE_DISPLAY_NAME "QtPlatz")                       # The IDE display name.
set(IDE_ID "qtcreator")                               # The IDE id (no spaces, lowercase!)
set(IDE_CASED_ID "QtPlatz")                           # The cased IDE id (no spaces!)
set(IDE_BUNDLE_IDENTIFIER "com.ms-cheminfo.${IDE_ID}")# The macOS application bundle identifier.
set(IDE_APP_ID "com.ms-cheminfo.${IDE_ID}")           # The free desktop application identifier.
set(IDE_PUBLISHER "MS-Cheminfomatics LLC")
set(IDE_AUTHOR "${IDE_PUBLISHER} and other contributors.")
set(IDE_COPYRIGHT "Copyright (C) ${IDE_AUTHOR}")

set(PROJECT_USER_FILE_EXTENSION .user)
set(IDE_DOC_FILE "qtcreator/qtcreator.qdocconf")
set(IDE_DOC_FILE_ONLINE "qtcreator/qtcreator-online.qdocconf")

# Absolute, or relative to <qtcreator>/src/app
# Should contain qtcreator.ico, qtcreator.xcassets
set(IDE_ICON_PATH "")
# Absolute, or relative to <qtcreator>/src/app
# Should contain images/logo/(16|24|32|48|64|128|256|512)/QtProject-${IDE_ID}.png
set(IDE_LOGO_PATH "")
