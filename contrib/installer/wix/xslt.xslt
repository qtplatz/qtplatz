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

  <xsl:key name="xsl-search" match="wix:Component[not(contains(wix:File/@Source, '.xsl'))]" use="@Id" />

  <!-- exclude !.xsl -->
  <xsl:template match="wix:Component[key('xsl-search', @Id)]" />
  <xsl:template match="wix:ComponentRef[key('xsl-search', @Id)]" />
  
</xsl:stylesheet>
