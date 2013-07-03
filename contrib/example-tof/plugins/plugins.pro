# USE .subdir AND .depends !
# OTHERWISE PLUGINS WILL BUILD IN WRONG ORDER (DIRECTORIES ARE COMPILED IN PARALLEL)

TEMPLATE  = subdirs

SUBDIRS   = plugin_toftune

plugin_toftune.subdir = toftune
#plugin_hmqtune.depends = plugin_coreplugin
