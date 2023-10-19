<?xml version="1.0" encoding="utf-8"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"  xmlns:fo="http://www.w3.org/1999/XSL/Format">

<!-- ************************ root (proofs) template ***************  -->



<xsl:template match="/">
<fo:root>
  <fo:layout-master-set>
    <!-- layout information -->
    <fo:simple-page-master master-name="simple"
                  page-width="29.7cm"
                  page-height="21cm"
                  margin-top="1cm" 
                  margin-bottom="1cm"
                  margin-left="1.5cm"
                  margin-right="1.5cm">
      <fo:region-body margin-top="0cm" margin-bottom="0cm"/>
      <fo:region-before extent="3cm"/>
      <fo:region-after extent="1.5cm"/>
    </fo:simple-page-master>
  </fo:layout-master-set>
  <!-- end: defines page layout -->

  <fo:page-sequence master-reference="simple">
    <fo:flow flow-name="xsl-region-body">
      <fo:block font-size="16pt"
            font-family="serif"
            line-height="24pt"
            font-weight="bold"
            padding-top="3pt">
        Prover9 Job
      </fo:block>
      <xsl:if test="proofs/heading">
      <fo:block space-before="15pt"
                font-size="9pt"
                font-family="sans-serif" 
                line-height="12pt"
                space-after.optimum="3pt"
                margin-left="20pt"
                linefeed-treatment="preserve"
                white-space-collapse="false"
                white-space-treatment="preserve">
<xsl:value-of select="proofs/heading"/>
      </fo:block>
      </xsl:if>
      <xsl:if test="proofs/source">
      <fo:block space-before="10pt"
                font-size="12pt"
                font-family="serif"
                line-height="12pt"
                space-after.optimum="3pt"
                text-align="justify">
        This page was generated from file <xsl:value-of select="proofs/source"/>.
      </fo:block>
      </xsl:if>
      <xsl:if test="proofs/@number_of_proofs">
      <fo:block space-before="10pt"
                font-size="12pt"
                font-family="serif"
                line-height="12pt"
                space-after.optimum="3pt"
                text-align="justify">
        Number of proofs here: <xsl:value-of select="proofs/@number_of_proofs"/>.
      </fo:block>
      </xsl:if>

      <xsl:apply-templates select="proofs/proof"/>

      <fo:block>
        <fo:footnote>
          <fo:inline color="white" font-size="0pt" >a</fo:inline>
          <fo:footnote-body>
             <fo:block font-size="8pt" font-family="serif">
               This pdf was generated automatically; there might be overful boxes.
             </fo:block>
          </fo:footnote-body>
        </fo:footnote>
      </fo:block>
      
    </fo:flow> <!-- closes the flow element-->
  </fo:page-sequence> <!-- closes the page-sequence -->
</fo:root>
</xsl:template>

<!-- ************************ proof template ***********************  -->
<xsl:template match="proof">

  <fo:block space-before="5pt"
    space-after.optimum="15pt"
    background-color="black"
    padding-top="0.5pt">
  </fo:block>
  <xsl:if test="@number">
  <fo:block font-size="16pt"
            font-family="serif"
            line-height="24pt"
            font-weight="bold"
            padding-top="3pt">
    Proof <xsl:value-of select="@number"/>
   </fo:block>
   </xsl:if>
   <xsl:if test="comments">
   <fo:block space-before="15pt"
             font-size="9pt"
             font-family="sans-serif"
             line-height="12pt"
             space-after.optimum="3pt"
             margin-left="20pt"
             linefeed-treatment="preserve"
             white-space-collapse="false"
             white-space-treatment="preserve">
               <xsl:value-of select="comments"/>
   </fo:block>
   </xsl:if>
<!--<style type="text/css">
td { white-space: nowrap; }
</style>-->

  <fo:block>
    <fo:table border-width="1px" border-style="solid" border-color="white" text-align="left" table-layout="fixed" font-size="9pt" width="100%">
      <fo:table-column column-width="1cm"/>
      <fo:table-column column-width="17cm"/>
      <fo:table-column column-width="3.7cm"/>
      <fo:table-column column-width="5cm"/>
      <fo:table-body>
        <xsl:apply-templates select="clause"/>
      </fo:table-body>
    </fo:table>
  </fo:block></xsl:template>

<!-- ************************ clause template ***********************  -->

<xsl:template match="clause">
  <fo:table-row>
    <fo:table-cell color="red">
      <fo:block><xsl:value-of select="@id"/></fo:block>
    </fo:table-cell>
    <xsl:choose>
      <xsl:when test="@type='goal'">
        <fo:table-cell background-color="#bfbfbf" color="red">
          <fo:block>
            <xsl:value-of select="literal[1]"/>
            <xsl:for-each select="literal[position()&gt;1]">
              |<xsl:value-of select="."/>
            </xsl:for-each>
          </fo:block>
        </fo:table-cell>
      </xsl:when>

      <xsl:when test="@type='assumption'">
        <fo:table-cell background-color="#bfbfbf">
          <fo:block>
            <xsl:value-of select="literal[1]"/>
            <xsl:for-each select="literal[position()&gt;1]">
              |<xsl:value-of select="."/>
            </xsl:for-each>
          </fo:block>
        </fo:table-cell>
      </xsl:when>      

      <xsl:when test="@type='deny' or @type='clausify'">
        <fo:table-cell background-color="#eaeaea">
          <fo:block>
            <xsl:value-of select="literal[1]"/>
            <xsl:for-each select="literal[position()&gt;1]">
              |<xsl:value-of select="."/>
            </xsl:for-each>
          </fo:block>
        </fo:table-cell>
      </xsl:when>

      <xsl:otherwise>
          <fo:table-cell>
            <fo:block>
              <xsl:value-of select="literal[1]"/>
              <xsl:for-each select="literal[position()&gt;1]">
                 |<fo:inline color="red"> <xsl:value-of select="."/> </fo:inline>
              </xsl:for-each>
            </fo:block>
          </fo:table-cell>
      </xsl:otherwise>

    </xsl:choose>
    <fo:table-cell>
      <fo:block color="blue">
        <xsl:for-each select="attribute">
          # <xsl:value-of select="."/>
        </xsl:for-each>
      </fo:block>
    </fo:table-cell>
    <fo:table-cell>
      <fo:block color="green">
        <xsl:apply-templates select="justification"/>
      </fo:block>
    </fo:table-cell>

  </fo:table-row>
</xsl:template>

<!-- ************************ justification template *********************  -->

<xsl:template match="justification">
  [
    <xsl:value-of select="j1/@rule"/>          <!-- output primary -->
    <xsl:text> </xsl:text>                     <!-- to get some whitespace -->
    <fo:inline color="red">
      <xsl:value-of select="j1/@parents"/>       <!-- ok if empty -->
    </fo:inline>

    <xsl:for-each select="j2">                 <!-- output each secondary -->
      <xsl:text>, </xsl:text>
      <xsl:value-of select="@rule"/>
      <xsl:text> </xsl:text>                   <!-- to get some whitespace -->
      <fo:inline color="red">
        <xsl:value-of select="@parents"/>        <!-- ok if empty -->
      </fo:inline>
    </xsl:for-each>
  ]
</xsl:template>

</xsl:stylesheet>
