<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<!-- ************************ root (proofs) template ***************  -->

<xsl:template match="/">
<html>
<body>

<!-- print each attribute of "proofs"
<xsl:for-each select="proofs/@*">
  <xsl:value-of select="name()" /> = <xsl:value-of select="."/><br/>
</xsl:for-each>
-->

<xsl:if test="proofs/heading">
  <h3>Prover9 Job</h3>
  <blockquote>
  <pre>
  <xsl:value-of select="proofs/heading" />
  </pre>
  </blockquote>
</xsl:if>

<xsl:if test="proofs/source">
  <p />This page was generated from file
	<a href="{proofs/source}">
	<xsl:value-of select="proofs/source" />
	</a>.
</xsl:if>

<xsl:if test="proofs/@number_of_proofs">
  <p />Number of proofs here: 
	<xsl:value-of select="proofs/@number_of_proofs" />.
</xsl:if>

<xsl:apply-templates select="proofs/proof"/> 

</body>
</html>
</xsl:template>

<!-- ************************ proof template ***********************  -->

<xsl:template match="proof">

<hr />
<h3>Proof
<xsl:if test="@number">
     <xsl:value-of select="@number" />
</xsl:if>
</h3>

<xsl:if test="comments">
  <blockquote>
  <pre>
  <xsl:value-of select="comments" />
  </pre>
  </blockquote>
</xsl:if>

<style type="text/css">
td { white-space: nowrap; }
</style>

<table>
<xsl:apply-templates select="clause"/> 
</table>
</xsl:template>

<!-- ************************ clause template ***********************  -->

<xsl:template match="clause">
<tr>

<td>
<font color="red">
<xsl:value-of select="@id"/>
</font>
</td>
<td></td>  <!-- empty column puts a little extra space after the ID -->

<xsl:choose>

<xsl:when test="@type='goal'">
  <td bgcolor="#bfbfbf">
  <font color="red">
  <xsl:value-of select="literal[1]"/>
  <xsl:for-each select="literal[position()>1]">
     |<xsl:value-of select="."/>
  </xsl:for-each>
  </font>
  </td>
</xsl:when>

<xsl:when test="@type='assumption'">
  <td bgcolor="#bfbfbf">
  <xsl:value-of select="literal[1]"/>
  <xsl:for-each select="literal[position()>1]">
     |<xsl:value-of select="."/>
  </xsl:for-each>
  </td>
</xsl:when>

<xsl:when test="@type='deny' or @type='clausify'">
  <td bgcolor="#eaeaea">
  <xsl:value-of select="literal[1]"/>
  <xsl:for-each select="literal[position()>1]">
     |<xsl:value-of select="."/>
  </xsl:for-each>
  </td>
</xsl:when>

<xsl:otherwise>
  <td>
  <xsl:value-of select="literal[1]"/>
  <xsl:for-each select="literal[position()>1]">
    <font color="red"> | </font> <xsl:value-of select="."/>
  </xsl:for-each>
  </td>
</xsl:otherwise>

</xsl:choose>

<td>
<font color="blue">
<xsl:for-each select="attribute">
  # <xsl:value-of select="."/>
</xsl:for-each>
</font>
</td>

<td>
<font color="green">
<xsl:apply-templates select="justification"/>
</font>
</td>

</tr>
</xsl:template>

<!-- ************************ justification template *********************  -->

<xsl:template match="justification">

<!-- <xsl:value-of select="@jstring"/> -->

[
<xsl:value-of select="j1/@rule"/>          <!-- output primary -->
<xsl:text> </xsl:text>                     <!-- to get some whitespace -->
<font color="red">
<xsl:value-of select="j1/@parents"/>       <!-- ok if empty -->
</font>

<xsl:for-each select="j2">                 <!-- output each secondary -->
  <xsl:text>, </xsl:text>
  <xsl:value-of select="@rule"/>
  <xsl:text> </xsl:text>                   <!-- to get some whitespace -->
  <font color="red">
  <xsl:value-of select="@parents"/>        <!-- ok if empty -->
  </font>
</xsl:for-each>
]

</xsl:template>

</xsl:stylesheet>
