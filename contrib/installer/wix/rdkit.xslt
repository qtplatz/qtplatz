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
  <xsl:key name="lib-search" match="wix:Component[contains(wix:File/@Source, '.lib')]" use="@Id" />  
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

  <!-- exclude .lib -->
  <xsl:template match="wix:Component[key('lib-search', @Id)]" />
  <xsl:template match="wix:ComponentRef[key('lib-search', @Id)]" />

  <!-- exclude unused dlls/files -->
  <xsl:template match="wix:Component[key('Qt5Declarative-search', @Id)]" />
  <xsl:template match="wix:ComponentRef[key('Qt5Declarative-search', @Id)]" />
  
</xsl:stylesheet>
