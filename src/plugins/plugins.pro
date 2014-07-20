# USE .subdir AND .depends !
# OTHERWISE PLUGINS WILL BUILD IN WRONG ORDER (DIRECTORIES ARE COMPILED IN PARALLEL)

include(../config.pri)

TEMPLATE  = subdirs

SUBDIRS   = plugin_coreplugin \
            plugin_servant \
            plugin_batchproc \
            plugin_quan


contains( QTPLATZ_CONFIG, Acquire ) {
  SUBDIRS += plugin_acquire
}

contains( QTPLATZ_CONFIG, Sequence ) {
  SUBDIRS += plugin_sequence
}

contains( QTPLATZ_CONFIG, Dataproc ) {
  SUBDIRS += plugin_dataproc
}

contains( QTPLATZ_CONFIG, ChemSpider ) {
  SUBDIRS += plugin_chemspider
}

contains( QTPLATZ_CONFIG, Chemistry ) {
  SUBDIRS += plugin_chemistry
}

contains( QTPLATZ_CONFIG, Peptide ) {
  SUBDIRS += plugin_peptide
}

contains( QTPLATZ_CONFIG, Quan ) {
  SUBDIRS += plugin_quan
}

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

plugin_batchproc.subdir = batchproc
plugin_batchproc.depends = plugin_coreplugin

plugin_peptide.subdir = peptide
plugin_peptide.depends = plugin_coreplugin

plugin_quan.subdir = quan
plugin_quan.depends = plugin_coreplugin

