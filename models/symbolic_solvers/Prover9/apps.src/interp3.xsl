<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<!-- ************************ root (interps) template ***************  -->

<xsl:template match="/">
<html>
<body>

<style type="text/css">
td { text-align: right }
</style>

<xsl:if test="interps/heading">
  <h3>Mace4 Job</h3>
  <blockquote>
  <pre>
  <xsl:value-of select="interps/heading" />
  </pre>
  </blockquote>
</xsl:if>

<xsl:if test="interps/source">
  <p />This page was generated from file
	<a href="{interps/source}">
	<xsl:value-of select="interps/source" />
	</a>.
</xsl:if>

<xsl:if test="interps/input">
  <hr />
  <h3>Constraints</h3>
  <blockquote>
  <pre>
  <xsl:value-of select="interps/input" />
  </pre>
  </blockquote>
</xsl:if>

<!-- print any attributes of interps -->

<xsl:for-each select="interps/@*">
  <xsl:value-of select="name()" /> = <xsl:value-of select="."/><br/>
</xsl:for-each>

<xsl:apply-templates select="interps/interp"/> 

</body>
</html>
</xsl:template>

<!-- ************************ interp template ***********************  -->

<xsl:template match="interp">

<hr />
<h3>Interpretation
<xsl:for-each select="@*">
  , <xsl:value-of select="name()" /> = <xsl:value-of select="."/>
</xsl:for-each>

</h3>

<xsl:apply-templates select="op0" /> 
<xsl:apply-templates select="op1" /> 
<xsl:apply-templates select="op2" /> 
<xsl:apply-templates select="opn" /> 

</xsl:template>

<!-- ************************ op0 template ***********************  -->

<xsl:template match="op0">

<p />
Constant <xsl:value-of select="sym"/> = <xsl:value-of select="v"/>
</xsl:template>

<!-- ************************ op1 template ***********************  -->

<xsl:template match="op1">

<p />
Unary
<xsl:value-of select="@type"/>
<xsl:text> </xsl:text>
 <xsl:value-of select="sym"/>

<blockquote>
<table cellpadding="4" rules="groups">

<colgroup />
<tr><td></td>
<xsl:for-each select="head/i">
  <td><font color="red"><xsl:value-of select="."/></font></td>
</xsl:for-each>
</tr>
<tbody>
<tr><td></td>
<xsl:for-each select="row/v">
  <td><xsl:value-of select="."/></td>
</xsl:for-each>
</tr>
</tbody>

</table>
</blockquote>

</xsl:template>

<!-- ************************ row template ***********************  -->

<xsl:template match="row">

<tr>
<td><font color="red"><xsl:value-of select="i"/></font></td>
<xsl:for-each select="v">
  <td><xsl:value-of select="."/></td>
</xsl:for-each>
</tr>

</xsl:template>

<!-- ************************ op2 template ***********************  -->

<xsl:template match="op2">

<p />
Binary
<xsl:value-of select="@type"/>
<xsl:text> </xsl:text>
<xsl:value-of select="sym"/>

<blockquote>
<table cellpadding="4" rules="groups">
<colgroup />
<tr><td></td>
<xsl:for-each select="head/i">
  <td><font color="red"><xsl:value-of select="."/></font></td>
</xsl:for-each>
</tr>
<tbody>
<xsl:apply-templates select="row"/>
</tbody>
</table>
</blockquote>

</xsl:template>

<!-- ************************ tupval template ***********************  -->

<xsl:template match="tupval">

<tr>
<xsl:for-each select="tup/i">
  <td><font color="red"><xsl:value-of select="."/></font></td>
</xsl:for-each>
<td><xsl:value-of select="v"/></td>
</tr>

</xsl:template>

<!-- ************************ opn template ***********************  -->

<xsl:template match="opn">

<p />
<xsl:value-of select="@arity"/>-ary
<xsl:value-of select="@type"/>
<xsl:text> </xsl:text>
<xsl:value-of select="sym"/>

<xsl:variable name="arity"><xsl:value-of select="@arity"/></xsl:variable>

<blockquote>
<table cellpadding="1" rules="groups">
<colgroup span="{$arity}"/>
<tr><th colspan="{$arity}">tuple</th><th>value</th></tr>
<tbody align="center">
<xsl:apply-templates select="tupval"/>
</tbody>
</table>
</blockquote>

</xsl:template>

</xsl:stylesheet>

