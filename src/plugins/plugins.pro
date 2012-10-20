# USE .subdir AND .depends !
# OTHERWISE PLUGINS WILL BUILD IN WRONG ORDER (DIRECTORIES ARE COMPILED IN PARALLEL)

TEMPLATE  = subdirs

SUBDIRS   = plugin_coreplugin \
            plugin_servant \
            plugin_acquire \
            plugin_sequence \
            plugin_dataproc \
            plugin_chemspider \
            plugin_chemistry

plugin_coreplugin.subdir = coreplugin

plugin_chemspider.subdir = chemspider
plugin_chemspider.depends = plugin_coreplugin

plugin_servant.subdir = servant
plugin_servant.depends = plugin_coreplugin

plugin_acquire.subdir = acquire
plugin_acquire.depends = plugin_coreplugin

plugin_sequence.subdir = sequence
plugin_sequence.depends = plugin_coreplugin

plugin_dataproc.subdir = dataproc
plugin_dataproc.depends = plugin_coreplugin

plugin_chemistry.subdir = chemistry
plugin_chemistry.depends = plugin_coreplugin
