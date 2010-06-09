# USE .subdir AND .depends !
# OTHERWISE PLUGINS WILL BUILD IN WRONG ORDER (DIRECTORIES ARE COMPILED IN PARALLEL)

TEMPLATE  = subdirs

SUBDIRS   = plugin_coreplugin \
            plugin_welcome \
            plugin_find \
            plugin_texteditor \
            plugin_locator \
            plugin_dataanalysis \ #on trial for mass spec data analysis
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

plugin_help.subdir = help
plugin_help.depends = plugin_find
plugin_help.depends += plugin_locator
plugin_help.depends += plugin_coreplugin

