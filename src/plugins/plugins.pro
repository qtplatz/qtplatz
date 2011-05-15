# USE .subdir AND .depends !
# OTHERWISE PLUGINS WILL BUILD IN WRONG ORDER (DIRECTORIES ARE COMPILED IN PARALLEL)

TEMPLATE  = subdirs

SUBDIRS   = plugin_coreplugin \
            plugin_welcome \
            plugin_servant \
            plugin_acquire \
#            plugin_tune \
            plugin_sequence \
            plugin_dataproc

plugin_coreplugin.subdir = coreplugin

plugin_welcome.subdir = welcome
plugin_welcome.depends = plugin_coreplugin

plugin_servant.subdir = servant
plugin_servant.depends = plugin_coreplugin

plugin_acquire.subdir = acquire
plugin_acquire.depends = plugin_coreplugin

#plugin_tune.subdir = tune
#plugin_tune.depends = plugin_coreplugin

plugin_sequence.subdir = sequence
plugin_sequence.depends = plugin_coreplugin

plugin_dataproc.subdir = dataproc
plugin_dataproc.depends = plugin_coreplugin

