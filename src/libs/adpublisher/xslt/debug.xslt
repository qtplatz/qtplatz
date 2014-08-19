<?xml version='1.0'?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="html" encoding="UTF-8" indent="yes" media-type="text/html"/>

  <xsl:template match="/qtplatz_document">
    <html>
      <meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
      <head>
	<title>Quan Browseer Reporting Test</title>
      </head>

      <body>
	<h2>Reporting Test</h2>

	<xsl:apply-templates/>

	<hr/>
      </body>
      
    </html>
  </xsl:template>

  <xsl:template match="classdata[@decltype='class adcontrols::idAudit']/class|ident_[@version|@tracking_level]">
    <div>
      <hr/>
      <table>
	<tr>	  <td><i>Date:</i>             </td>  <td><xsl:value-of select="dateCreated_"/>	</td>	</tr>
	<tr>	  <td><i>Createed by:</i>      </td>  <td><xsl:value-of select="nameCreatedBy_"/></td>	</tr>
	<tr>	  <td><i>Reference number:</i> </td>  <td><xsl:value-of select="uuid_"/>	</td>	</tr>
      </table>
      <hr/>
    </div>
  </xsl:template>

  <xsl:template name="sample_attribute">
    <xsl:param name="attr">UNK</xsl:param>
    <xsl:choose>
      <xsl:when test="$attr = 0">unk</xsl:when>
      <xsl:when test="$attr = 1">std</xsl:when>
      <xsl:when test="$attr = 2">QC</xsl:when>
      <xsl:when test="$attr = 3">Blank</xsl:when>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="data_generation">
    <xsl:param name="value">raw</xsl:param>
    <xsl:choose>
      <xsl:when test="$value = 0">average all</xsl:when>
      <xsl:when test="$value = 1">take 1st spectrum</xsl:when>
      <xsl:when test="$value = 2">take 2nd spectrum</xsl:when>
      <xsl:when test="$value = 3">take last spectrum</xsl:when>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="level">
    <xsl:choose>
      <xsl:when test="level_ = 0"><i>n/a</i></xsl:when>
      <xsl:otherwise><xsl:value-of select="level_"/></xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="SampleSequence/classdata[@declType='class adcontrols::QuanSequence']">
    <h3>Sample Sequence</h3>
    <table border="1">
      <tr>
	<td> "id" </td>
	<td> "name" </td>
	<td> "sample type" </td>
	<td> "level" </td>
	<td> "process" </td>
	<td> "scan_range(first)" </td>
	<td> "scan_range(second)" </td>
	<td> "dataSource" </td>
	<td> "uuid" </td>
      </tr>
      <xsl:for-each select="class/samples_/item">
	<tr>
	  <td> <xsl:value-of select="rowid_"/> </td>
	  <td> <xsl:value-of select="name_"/> </td>
	  <td> 
	    <xsl:call-template name="sample_attribute">
	      <xsl:with-param name="attr"><xsl:value-of select="sampleType_"/></xsl:with-param>
	    </xsl:call-template>
	  </td>
	  <td>
	    <xsl:call-template name="level"/>
	  </td>
	  <td> 
	    <xsl:call-template name="data_generation">
	      <xsl:with-param name="value"><xsl:value-of select="dataGeneration_"/></xsl:with-param>
	    </xsl:call-template>
	  </td>
	  <td> <xsl:value-of select="scan_range_/first"/> </td>
	  <td> <xsl:value-of select="scan_range_/second"/> </td>
	  <td> <xsl:value-of select="dataSource_"/> </td>
	  <td> <xsl:value-of select="uuid_"/> </td>
	</tr>
      </xsl:for-each>
    </table>
  </xsl:template>

  
  <xsl:template match="ProcessMethod">
  </xsl:template>
  
  <xsl:template match="QuanResponse[@sampleType='UNK']">
    <h2>Summary Report (UNK)</h2>
  </xsl:template>

  <xsl:template match="QuanResponse[@sampleType='STD']">
    <h2>Summary Report (STD)</h2>
  </xsl:template>

  <xsl:template match="QuanCalib">
    <h2>Caliubration Curve</h2>
  </xsl:template>


</xsl:stylesheet>
