<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0" >
  <xsl:output method="html" encoding="UTF-8" indent="yes" media-type="text/html"/>

  <xsl:template match="/boost_serialization">
    <html>
      <meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
      <head>
	<title>process method</title>
      </head>
      <body>
	<h4>process method</h4>
	<xsl:apply-templates/>
	<hr/>
      </body>
      
    </html>
  </xsl:template>

  <xsl:template match="ProcessMethod/ident_">
    <div>
      <hr/>
      <table>
	<tr> <td><i>Process Date:</i></td><td><xsl:value-of select="dateCreated_"/> </td></tr>
	<tr> <td><i>Created by:  </i></td><td><xsl:value-of select="nameCreatedBy_"/> </td></tr>
	<tr> <td><i>Reference #: </i></td><td><xsl:value-of select="uuid_"/> </td></tr>
      </table>
      <hr/>
    </div>
  </xsl:template>

  <!-- remove -->
  <xsl:template match="ProcessMethod/vec_/count"></xsl:template> 
  <xsl:template match="ProcessMethod/vec_/item_version"></xsl:template>
  <xsl:template match="ProcessMethod/vec_/item/which"></xsl:template>
  
  <xsl:template name="scan_type">
    <xsl:param name="value">TOF</xsl:param>
    <xsl:choose>
      <xsl:when test="$value = 0">TOF</xsl:when>
      <xsl:when test="$value = 1">Proportional</xsl:when>
      <xsl:when test="$value = 2">Constant</xsl:when>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="peak_width">
    <xsl:param name="type">TOF</xsl:param>
    <xsl:choose>
      <xsl:when test="$type = 0">
	<xsl:variable name="daltons" select="rsTofInDa_"/><xsl:value-of select="format-number($daltons, '#0.0000')"/> Da at <i>m/z</i>&#160; 
        <xsl:variable name="mass" select="rsTofAtMz_"/><xsl:value-of select="format-number($mass, '#.0')"/>
      </xsl:when>
      <xsl:when test="$type = 1"><xsl:value-of select="rsPropoInPpm_"/>ppm</xsl:when>
      <xsl:when test="$type = 2"><xsl:value-of select="rsConstInDa_"/>Da</xsl:when>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="centroid_area_method">
    <xsl:param name="value"></xsl:param>
    <xsl:choose>
      <xsl:when test="$value = 0">Intens. &#215; mDa</xsl:when>
      <xsl:when test="$value = 1">Intens. &#215; Time(ns)</xsl:when>
      <xsl:when test="$value = 2">Width norm.</xsl:when>
      <xsl:when test="$value = 3">Samp. interval</xsl:when>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="centroid_area_height">
    <xsl:param name="type">Height</xsl:param>
    <xsl:choose>
      <xsl:when test="$type = 0">Height</xsl:when>
      <xsl:when test="$type = 1">Area <xsl:call-template name="centroid_area_method">
                                        <xsl:with-param name="value"><xsl:value-of select="areaMethod_"/></xsl:with-param>
				      </xsl:call-template>
      </xsl:when>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="centroid_noise_filter">
    <xsl:param name="type">OFF</xsl:param>
    <xsl:choose>
      <xsl:when test="$type = 0">OFF</xsl:when>
      <xsl:when test="$type = 1">DFT <xsl:value-of select="cutoffFreqHz_"/>Hz</xsl:when>
    </xsl:choose>
  </xsl:template>
  
  <!-- Centroid Methoid -->
  <xsl:template match="ProcessMethod/vec_/item/value[@class_id='3']">
    <table>
      <tr>
	<td>Scan Type:</td>
	<td>
	  <xsl:call-template name="scan_type">
	    <xsl:with-param name="value"><b><xsl:value-of select="peakWidthMethod_"/></b></xsl:with-param>
	  </xsl:call-template>
	</td>
      </tr>
      <tr>
	<td>Peak width:</td>
	<td>
	  <xsl:call-template name="peak_width">
	    <xsl:with-param name="type"><b><xsl:value-of select="peakWidthMethod_"/></b></xsl:with-param>
	  </xsl:call-template>
	</td>
      </tr>
      <tr>	
	<td>Area/Height</td>
	<td>
	  <xsl:call-template name="centroid_area_height">
	    <xsl:with-param name="type"><b><xsl:value-of select="bCentroidAreaIntensity_"/></b></xsl:with-param>
	  </xsl:call-template>
	</td>
      </tr>
      <tr>
	<td>Noise filter</td>
	<td>
	  <xsl:call-template name="centroid_noise_filter">
	    <xsl:with-param name="type"><xsl:value-of select="noiseFilterMethod_"/></xsl:with-param>
	  </xsl:call-template>
	</td>	
      </tr>
    </table>
  </xsl:template>
  
</xsl:stylesheet>
