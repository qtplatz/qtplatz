<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0" >
<!--  <xsl:output method="html" encoding="UTF-8" indent="yes" media-type="text/html"/> -->

  <xsl:template match="/qtplatz_document">
    <html>
      <meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
      <head>
	<title>Quan Browser Reporting Test</title>
      </head>

      <body>
	<h2>Reporting Test</h2>

	<xsl:apply-templates/>

	<hr/>
      </body>
      
    </html>
  </xsl:template>

  <!-- got from an article posted by Nigel Wilson, August 9, 2011 on
       http://our.umbraco.org/forum/developers/xslt/22963-best-way-to-trim-a-filename
  -->
  <!-- 2. Recursive template (with tail-recursion, of course :-) -->
  <xsl:template name="extract-filename">
    <xsl:param name="path" select="." />
    <xsl:choose>
      <xsl:when test="not(contains($path, '/'))">
        <xsl:value-of select="$path" />
      </xsl:when>
      <xsl:otherwise>
        <xsl:call-template name="extract-filename">
          <xsl:with-param name="path" select="substring-after($path, '/')" />
        </xsl:call-template>
      </xsl:otherwise>
    </xsl:choose>    
  </xsl:template>
  <!-- -->

  <xsl:template match="/qtplatz_document/boost_serialization[@decltype='class adcontrols::idAudit']/idAudit">
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
      <xsl:when test="$attr = 0">UNK</xsl:when>
      <xsl:when test="$attr = 1">STD</xsl:when>
      <xsl:when test="$attr = 2">QC</xsl:when>
      <xsl:when test="$attr = 3">Blank</xsl:when>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="data_generation">
    <xsl:param name="value">raw</xsl:param>
    <xsl:param name="first">0</xsl:param>
    <xsl:param name="second">0</xsl:param>
    <xsl:choose>
      <xsl:when test="$value = 0">AS IS</xsl:when>
      <xsl:when test="$value = 1">Average spectra from: <xsl:value-of select="$first"/> to: <xsl:value-of select='$second'/></xsl:when>
      <xsl:when test="$value = 2">Chromatogram</xsl:when>
      <xsl:when test="$value = 2">Process Raw Spectra</xsl:when>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="level">
    <xsl:choose>
      <xsl:when test="level_ = 0"><i>n/a</i></xsl:when>
      <xsl:otherwise><xsl:value-of select="level_"/></xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="SampleSequence/boost_serialization[@decltype='class adcontrols::QuanSequence']/QuanSequence">
    <h3>Sample Sequence</h3>
    <table border="1" style='table-layout:fixed'>
      <tr bgcolor='#FDFD96'>
	<td> id </td>
	<td> name </td>
	<td> sample type </td>
	<td> level </td>
	<td> process </td>
	<td> dataSource </td>
      </tr>
      <xsl:for-each select="impl/samples_/item/impl">
	<tr>
	  <td> <xsl:value-of select="rowid_"/> </td>
	  <td> <xsl:value-of select="name_"/> </td>
	  <td> 
	    <xsl:call-template name="sample_attribute">
	      <xsl:with-param name="attr"><xsl:value-of select="sampleType_"/></xsl:with-param>
	    </xsl:call-template>
	  </td>
	  <td align="center">
	    <xsl:call-template name="level"/>
	  </td>
	  <td> 
	    <xsl:call-template name="data_generation">
	      <xsl:with-param name="value"><xsl:value-of select="dataGeneration_"/></xsl:with-param>
	      <xsl:with-param name="first"><xsl:value-of select="scan_range_/first"/></xsl:with-param>
	      <xsl:with-param name="second"><xsl:value-of select="scan_range_/second"/></xsl:with-param>
	    </xsl:call-template>
	  </td>
	  <td> 
	    <xsl:call-template name="extract-filename">
	      <xsl:with-param name="path"><xsl:value-of select="dataSource_"/></xsl:with-param>
	    </xsl:call-template>
	  </td>
	</tr>
      </xsl:for-each>
    </table>
  </xsl:template>
  
  <xsl:template name="QR-UNK">
    <xsl:param name="respid">1</xsl:param>
    <xsl:param name="column">id</xsl:param>
    <xsl:value-of select="/qtplatz_document/QuanResponse[@sampleType='UNK']/row[@id=$respid]/column[@name=$column]"/>
  </xsl:template>

  <xsl:template match="QuanResponse[@sampleType='UNK']">
    <h3>Quantitative Analysis Summary</h3>
    <table border="1">
      <tr bgcolor='#FDFD96'>
	<td> description </td>
	<td> data source </td>	
	<td> name </td>
	<td> formula </td>
	<td> mass </td>
	<td> error(mDa) </td>
	<td> intensity </td>
	<td> amount </td>	
      </tr>
      <xsl:for-each select="row">
	<tr>
	  <td> <xsl:value-of select="column[@name='description']"/> </td>
	  <td align="center">
	    <xsl:call-template name="extract-filename">
	      <xsl:with-param name="path"><xsl:value-of select="column[@name='dataSource']"/></xsl:with-param>
	    </xsl:call-template>
	  </td>
	  <td align="center">
	    <xsl:value-of select="column[@name='name']"/>
	  </td>
	  <td align="center">
	    <xsl:value-of select="column[@name='formula' and @decltype='richtext']" disable-output-escaping="yes"/>
	  </td>
	  <td>
	    <xsl:variable name="mass" select="column[@name='mass']"/>
	    <xsl:value-of select='format-number($mass, "#.0000")'/>
	  </td>
	  <td align="right">
	    <xsl:variable name="error" select="column[@name='error(Da)']"/>
	    <xsl:value-of select='format-number($error * 1000, "0.000")'/>
	  </td>
	  <td align="right">
	    <xsl:variable name="intensity" select="column[@name='intensity']"/>
	    <xsl:value-of select='$intensity'/>
	  </td>
	  <td align="right">
	    <xsl:variable name="value">
	      <xsl:call-template name="QR-UNK">
		<xsl:with-param name="respid" select="column[@name='id']"/>
		<xsl:with-param name="column">amount</xsl:with-param>
	      </xsl:call-template>
	    </xsl:variable>
	    <xsl:value-of select='format-number($value, "#.00")'/>
	  </td>
	</tr>
      </xsl:for-each>
    </table>
  </xsl:template>


  <xsl:template match="QuanResponse[@sampleType='STD']">
    <h3>Summary of Standard Samples</h3>
    <table border="1">
      <tr bgcolor='#FDFD96'>
	<td> description </td>
	<td> name </td>
	<td> formula </td>
	<td> mass </td>
	<td> error(mDa) </td>
	<td> level </td>
	<td> intensity </td>
      </tr>

      <xsl:for-each select="row">
	<tr>
	  <td> <xsl:value-of select="column[@name='description']"/> </td>
	  <td align="center"> <xsl:value-of select="column[@name='name']"/> </td>
	  <td align="center">
	    <xsl:value-of select="column[@name='formula' and @decltype='richtext']" disable-output-escaping="yes"/>
	  </td>
	  <td>
	    <xsl:variable name="mass" select="column[@name='mass']"/>
	    <xsl:value-of select='format-number($mass, "#.0000")'/>
	  </td>
	  <td align="right">
	    <xsl:variable name="error" select="column[@name='error(Da)']"/>
	    <xsl:value-of select='format-number($error * 1000, "0.000")'/>
	  </td>
	  <td align="center">
	    <xsl:value-of select="column[@name='level']"/>
	  </td>
	  <td align="right">
	    <xsl:variable name="intensity" select="column[@name='intensity']"/>
	    <xsl:value-of select='$intensity'/>
	  </td>
	</tr>
      </xsl:for-each>
    </table>
  </xsl:template>

  <xsl:template match="QuanCalib">
    <!--
    <h3>Caliubration Curve</h3>
    <table border="1">
      <tr bgcolor='#FDFD96'>
	<td> description </td>
	<td> formula </td>
	<td> N </td>
	<td> Min. intensity </td>
	<td> Max. intensity </td>
	<td> Eq. </td>
	<td style="width:40px"> level </td>
	<td style="width:80px"> amount </td>
	<td style="width:80px"> intensity </td>
      </tr>
      <xsl:for-each select="row">
	<tr>
	  <td>
	    <xsl:value-of select="column[@name='description']"/>
	  </td>
	  <td>
	    <xsl:value-of select="column[@name='formula' and @decltype='richtext']" disable-output-escaping="yes"/>
	  </td>
	  <td align="center">
	    <xsl:value-of select="column[@name='n']"/>
	  </td>
	  <td align="right">
	    <xsl:variable name="min_x" select = "column[@name='min_x']"/>
	    <xsl:value-of select='format-number($min_x, "#.00")'/>
	  </td>
	  <td align="right">
	    <xsl:variable name="max_x" select = "column[@name='max_x']"/>
	    <xsl:value-of select='format-number($max_x, "#.00")'/>
	  </td>
	  <td align="left">
	    <xsl:variable name="a" select = "column[@name='a']"/>
	    <xsl:variable name="b" select = "column[@name='b']"/>
	    Y = <xsl:value-of select='format-number($a, "#.0000")'/> + <xsl:value-of select='format-number($b, "#.0000")'/> X
	  </td>
	  <td colspan="3">
	    <table>
	      <xsl:for-each select="response/row">
		<tr>
		  <td style="width:40px">
		    <xsl:value-of select = "column[@name='level']"/>
		  </td>
		  <td style="width:80px">
		    <xsl:value-of select = "column[@name='amount']"/>
		  </td>
		  <td style="width:80px">
		    <xsl:value-of select = "column[@name='intensity']"/>
		  </td>
		</tr>
	      </xsl:for-each>
	    </table>
	  </td>
	</tr>
      </xsl:for-each>
    </table>
    -->
  </xsl:template>
  
  <xsl:template match="ProcessMethod">
  </xsl:template>

  <xsl:template match="PlotData">
    <h3>Peak Validation</h3>
    <table border="1">
      <xsl:for-each select="PeakResponse">
	<tr>
	  <td> 
	    <table>
	      <tr> <td> <xsl:value-of select='description'/> </td> </tr>
	      <tr>
		<td> Chemical Formula: </td>
		<td align="right"> <xsl:value-of select='formula' disable-output-escaping="yes"/> </td>
	      </tr>
	      <tr>
		<td> Amounts: </td>
		<td align="right">
		  <xsl:variable name="amount">
		    <xsl:call-template name="QR-UNK">
		      <xsl:with-param name="respid" select="@respId"/>
		      <xsl:with-param name="column">amount</xsl:with-param>
		    </xsl:call-template>
		  </xsl:variable>
		  <xsl:value-of select='format-number($amount, "#")'/>
		</td>
		<td>pg/L</td>
	      </tr>
	      <tr>
		<td> Intensity: </td>
		<td align="right">
		  <xsl:variable name="intensity">
		    <xsl:call-template name="QR-UNK">
		      <xsl:with-param name="respid" select="@respId"/>
		      <xsl:with-param name="column">intensity</xsl:with-param>
		    </xsl:call-template>
		  </xsl:variable>
		  <xsl:value-of select='$intensity'/>
		</td>
		<td/>
	      </tr>
	      <tr>
		<td colspan="3">
		  Data Source:
		  <i>
		  <xsl:call-template name="extract-filename">
		    <xsl:with-param name="path"><xsl:value-of select="dataSource"/></xsl:with-param>
		  </xsl:call-template>
		  , #FCN: <xsl:value-of select='./@fcn'/>
		  </i>
		</td>
	      </tr>
	      <xsl:for-each select="classdata[@decltype='class adcontrols::Descriptions']/class/Descriptions/vec_/item">
		<tr>
		  <td colspan="3">
		    <i><xsl:value-of select='text_'/></i>
		  </td>
		</tr>
	      </xsl:for-each>
	    </table>
	  </td>
	  <td> <div style="width:350px;height:300px"> <xsl:copy-of select="trace[@contents='resp_spectrum']"/> </div> </td>
	  <td> <div style="width:350px;height:300px"> <xsl:copy-of select="trace[@contents='resp_calib']"/> </div> </td>
	</tr>
      </xsl:for-each>
    </table>

  </xsl:template>

</xsl:stylesheet>
