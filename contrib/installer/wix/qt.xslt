<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:wix="http://schemas.microsoft.com/wix/2006/wi"
    xmlns="http://schemas.microsoft.com/wix/2006/wi"
  exclude-result-prefixes="wix">

  <xsl:output method="xml" encoding="UTF-8" indent="yes" />

  <xsl:template match="wix:Wix">
    <xsl:copy>
      <!-- The following enters the directive for adding the config.wxi include file to the dynamically generated file -->
      <!--xsl:processing-instruction name="include">$(sys.CURRENTDIR)wix\config.wxi</xsl:processing-instruction-->
      <xsl:apply-templates select="@*" />
      <xsl:apply-templates />
    </xsl:copy>
  </xsl:template>

  <!-- ### Adding the Win64-attribute to all Components -->
  <xsl:template match="wix:Component">

    <xsl:copy>
      <xsl:apply-templates select="@*" />
        <!-- Adding the Win64-attribute as we have a x64 application -->
        <xsl:attribute name="Win64">yes</xsl:attribute>

        <!-- Now take the rest of the inner tag -->
        <xsl:apply-templates select="node()" />
    </xsl:copy>

  </xsl:template>


  <xsl:template match="@*|node()">
    <xsl:copy>
      <xsl:apply-templates select="@*|node()" />
    </xsl:copy>
  </xsl:template>

  <xsl:key name="exe-search" match="wix:Component[contains(wix:File/@Source, '.exe')]" use="@Id" />
  <xsl:key name="pdb-search" match="wix:Component[contains(wix:File/@Source, '.pdb')]" use="@Id" />
  <xsl:key name="ddll-search" match="wix:Component[contains(wix:File/@Source, 'd.dll')]" use="@Id" />
  <!-- <xsl:key name="Qt5Qml-search" match="wix:Component[contains(wix:File/@Source, 'Qt5Qml.dll')]" use="@Id" /> -->
  <!-- <xsl:key name="Qt5Quick-search" match="wix:Component[contains(wix:File/@Source, 'Qt5Quick.dll')]" use="@Id" /> -->
  <xsl:key name="Qt5QuickTest-search" match="wix:Component[contains(wix:File/@Source, 'Qt5QuickTest.dll')]" use="@Id" />
  <xsl:key name="Qt5QuickWidgets-search" match="wix:Component[contains(wix:File/@Source, 'Qt5QuickWidgets.dll')]" use="@Id" />
  <xsl:key name="qtenv2-search" match="wix:Component[contains(wix:File/@Source, 'qtenv2.bat')]" use="@Id" />
  <xsl:key name="syncqt-search" match="wix:Component[contains(wix:File/@Source, 'syncqt.pl')]" use="@Id" />
  <xsl:key name="Qt5Designer-search" match="wix:Component[contains(wix:File/@Source, 'Qt5Designer.dll')]" use="@Id" />
  <xsl:key name="Qt5DesignerComponents-search" match="wix:Component[contains(wix:File/@Source, 'Qt5DesignerComponents.dll')]" use="@Id" />
  <xsl:key name="Qt5Declarative-search" match="wix:Component[contains(wix:File/@Source, 'Qt5Declarative.dll')]" use="@Id" />

  <!-- exclude .exe -->
  <xsl:template match="wix:Component[key('exe-search', @Id)]" />
  <xsl:template match="wix:ComponentRef[key('exe-search', @Id)]" />

  <!-- exclude .pdb -->
  <xsl:template match="wix:Component[key('pdb-search', @Id)]" />
  <xsl:template match="wix:ComponentRef[key('pdb-search', @Id)]" />

  <!-- exclude d.dll -->
  <xsl:template match="wix:Component[key('ddll-search', @Id)]" />
  <xsl:template match="wix:ComponentRef[key('ddll-search', @Id)]" />

  <!-- exclude unused dlls/files -->
  <xsl:template match="wix:Component[key('Qt5Declarative-search', @Id)]" />
  <xsl:template match="wix:ComponentRef[key('Qt5Declarative-search', @Id)]" />

  <xsl:template match="wix:Component[key('Qt5Designer-search', @Id)]" />
  <xsl:template match="wix:ComponentRef[key('Qt5Designer-search', @Id)]" />

  <xsl:template match="wix:Component[key('Qt5DesignerComponents-search', @Id)]" />
  <xsl:template match="wix:ComponentRef[key('Qt5DesignerComponents-search', @Id)]" />

  <xsl:template match="wix:Component[key('syncqt-search', @Id)]" />
  <xsl:template match="wix:ComponentRef[key('syncqt-search', @Id)]" />

  <xsl:template match="wix:Component[key('qtenv2-search', @Id)]" />
  <xsl:template match="wix:ComponentRef[key('qtenv2-search', @Id)]" />

  <xsl:template match="wix:Component[key('Qt5QuickTest-search', @Id)]" />
  <xsl:template match="wix:ComponentRef[key('Qt5QuickTest-search', @Id)]" />

  <xsl:template match="wix:Component[key('Qt5QuickWidgets-search', @Id)]" />
  <xsl:template match="wix:ComponentRef[key('Qt5QuickWidgets-search', @Id)]" />
  
</xsl:stylesheet>
