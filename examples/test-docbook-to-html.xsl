<?xml version ="1.0"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
		version="1.0">

<!-- This is a simple hackish test to try to transform DocBook to HTML -->

<xsl:template match ="book/article">
	<html>
		<head>	
			<title>Result of an XSLT transformation</title>
		</head>
		<body>
			<xsl:apply-templates/>
		</body>
	</html>
</xsl:template>

<xsl:template match ="book | article">
	<html>
		<head>	
			<title>Result of an XSL transformation</title>
		</head>
		<body>
			<xsl:apply-templates/>
		</body>
	</html>
</xsl:template>

<xsl:template match ="bookinfo">
	<!-- strip out this text -->
</xsl:template>

<xsl:template match ="chapter/title">
	<h1>
	<xsl:apply-templates/>
	</h1>
</xsl:template>

<xsl:template match ="sect1/title">
	<h2>
	<xsl:apply-templates/>
	</h2>
</xsl:template>

<xsl:template match ="sect2/title">
	<h3>
	<xsl:apply-templates/>
	</h3>
</xsl:template>

<xsl:template match ="sect3/title">
	<h4>
	<xsl:apply-templates/>
	</h4>
</xsl:template>

<xsl:template match ="emphasis">
	<em><xsl:apply-templates/></em>
</xsl:template>

<xsl:template match ="replaceable">
	<var><xsl:apply-templates/></var>
</xsl:template>

<xsl:template match ="para">
	<p><xsl:apply-templates/></p>
</xsl:template>

<xsl:template match ="programlisting">
	<code><pre><xsl:apply-templates/></pre></code>
</xsl:template>

<xsl:template match ="ulink">
	<a>	
		<xsl:attribute name="href">
			<xsl:apply-templates/>
		</xsl:attribute>
		<xsl:apply-templates/>
	</a>
</xsl:template>

<xsl:template match = "orderedlist">
	<ol>
		<xsl:apply-templates/>
	</ol>
</xsl:template>

<xsl:template match = "orderedlist/listitem">
	<li>
		<xsl:apply-templates/>
	</li>
</xsl:template>

<xsl:template match = "itemizedlist">
	<ul>
		<xsl:apply-templates/>
	</ul>
</xsl:template>

<xsl:template match = "itemizedlist/listitem">
	<li>
		<xsl:apply-templates/>
	</li>
</xsl:template>

<xsl:template match = "variablelist">
	<dl>
		<xsl:apply-templates/>
	</dl>
</xsl:template>

<xsl:template match = "varlistentry/term">
	<dt><b>
		<xsl:apply-templates/>
	</b></dt>	
</xsl:template>

<xsl:template match = "varlistentry/listitem">
	<dd>
		<xsl:apply-templates/>
	</dd>	
</xsl:template>

</xsl:stylesheet>

