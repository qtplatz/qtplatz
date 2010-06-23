# USE .subdir AND .depends !
# OTHERWISE PLUGINS WILL BUILD IN WRONG ORDER (DIRECTORIES ARE COMPILED IN PARALLEL)

TEMPLATE  = subdirs

SUBDIRS   = plugin_coreplugin \
            plugin_welcome \
            plugin_find \
            plugin_texteditor \
            plugin_locator \
#            plugin_dataanalysis \ #on trial for mass spec data analysis
            plugin_acquire \
#            plugin_tune \
            plugin_sequence \
            plugin_analysis \
#            plugin_batchproc \
            plugin_adbroker \
            plugin_help

plugin_coreplugin.subdir = coreplugin

plugin_welcome.subdir = welcome
plugin_welcome.depends = plugin_coreplugin

plugin_find.subdir = find
plugin_find.depends += plugin_coreplugin

plugin_texteditor.subdir = texteditor
plugin_texteditor.depends = plugin_find
plugin_texteditor.depends += plugin_locator
plugin_texteditor.depends += plugin_coreplugin

plugin_locator.subdir = locator
plugin_locator.depends = plugin_coreplugin

plugin_dataanalysis.subdir = dataanalysis
plugin_dataanalysis.depends = plugin_coreplugin

plugin_acquire.subdir = acquire
plugin_acquire.depends = plugin_coreplugin

plugin_tune.subdir = tune
plugin_tune.depends = plugin_coreplugin

plugin_sequence.subdir = sequence
plugin_sequence.depends = plugin_coreplugin

plugin_analysis.subdir = analysis
plugin_analysis.depends = plugin_coreplugin

plugin_batchproc.subdir = batchproc
plugin_batchproc.depends = plugin_coreplugin

plugin_adbroker.subdir = adbroker
plugin_adbroker.depends = plugin_coreplugin

plugin_help.subdir = help
plugin_help.depends = plugin_find
plugin_help.depends += plugin_locator
plugin_help.depends += plugin_coreplugin

