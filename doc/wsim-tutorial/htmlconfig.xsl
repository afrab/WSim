<?xml version="1.0" encoding="ASCII"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
		xmlns:exsl="http://exslt.org/common" 
		xmlns:cf="http://docbook.sourceforge.net/xmlns/chunkfast/1.0" 
		xmlns:ng="http://docbook.org/docbook-ng" 
		xmlns:db="http://docbook.org/ns/docbook" 
		xmlns="http://www.w3.org/1999/xhtml" 
		version="1.0" 
		exclude-result-prefixes="exsl cf ng db">
	<!-- Configure the stylesheet to use -->
	<xsl:param name="html.stylesheet" select="'css/mybean.css'"/>
	<!-- Use ids for filenames -->
	<xsl:param name="use.id.as.filename" select="'1'"/>
	<!-- Turn on admonition and navigational graphics. -->
	<xsl:param name="admon.graphics" select="'1'"/>
	<xsl:param name="admon.textlabel" select="'0'"/>
	<xsl:param name="admon.graphics.path">admon/</xsl:param>
	<xsl:param name="navig.graphics" select="'1'"/>
	<xsl:param name="navig.graphics.path">navig/</xsl:param>
	<xsl:param name="navig.graphics.extension">.png</xsl:param>
	<xsl:param name="navig.showtitles">1</xsl:param>
	<xsl:param name="header.rule" select="0"/>
	<xsl:param name="footer.rule" select="0"/>
	<!-- Per chapter chunking in files -->
	<xsl:param name="chunk.section.depth" select="0"/>
	<xsl:template match="chapter/title" mode="titlepage.mode" priority="10"/>
	<xsl:template match="bookinfo/title" mode="titlepage.mode" priority="10"/>
	<xsl:param name="chunk.first.sections" select="0"/>
	<!-- Use callout numbers -->
	<xsl:param name="callout.graphics" select="'1'"/>
	<xsl:param name="callout.graphics.path">callouts/</xsl:param>
	<!-- Numbering -->
	<xsl:param name="appendix.autolabel" select="'A'"/>
	<xsl:param name="chapter.autolabel" select="'I'"/>
	<xsl:param name="part.autolabel" select="0"/>
	<xsl:param name="reference.autolabel" select="'I'"/>
	<xsl:param name="section.autolabel" select="1"/>
	<!-- Try to connect with the css -->
	<!--<xsl:template match="programlisting" mode="class.value">
		<xsl:value-of select="'-programlisting -block-indent'"/>
	</xsl:template>
	<xsl:template match="sect1" mode="class.value">
		<xsl:value-of select="'-sect1'"/>
	</xsl:template>
	<xsl:template match="title" mode="class.value">
		<xsl:value-of select="'-title'"/>
	</xsl:template>
	<xsl:template match="para" mode="class.value">
		<xsl:value-of select="'-para'"/>
	</xsl:template>-->
	<xsl:param name="formal.title.placement">
	figure after
	example before
	equation before
	table before
	procedure before
	task before
	</xsl:param>
</xsl:stylesheet>