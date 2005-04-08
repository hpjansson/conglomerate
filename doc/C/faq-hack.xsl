<?xml version ="1.0"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
		version="1.0">

  <xsl:template match ="qandaset|question|answer">
    <xsl:apply-templates />
  </xsl:template>

  <xsl:template match ="qandaentry">
    <sect2>
      <xsl:copy-of select="@*"/>
      <xsl:apply-templates />
    </sect2>
  </xsl:template>

  <xsl:template match ="qandadiv">
    <sect1>
      <xsl:copy-of select="@*"/>
      <xsl:apply-templates />
    </sect1>
  </xsl:template>

  <xsl:template match ="question/para">
    <title><xsl:apply-templates /></title>
  </xsl:template>

  <xsl:template match="*">
    <xsl:copy>
      <xsl:copy-of select="@*"/>
      <xsl:apply-templates/>
    </xsl:copy>
  </xsl:template>

</xsl:stylesheet>

