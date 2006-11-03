<?xml version="1.0" encoding="UTF-8" ?>


<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">


	<xsl:output method="text"/>
	<xsl:strip-space elements="RiAPI"/>


	<!--	API	-->
	<xsl:template match="RiAPI">
		<!--	Procedures	-->
		<xsl:apply-templates select="Procedures/Procedure"/>
	</xsl:template>

	<xsl:template match="Procedure">
		<xsl:value-of select="concat('#define VALIDATE_', translate(@name, 'abcdefghijklmnopqrstuvwxyz', 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'))"/>
		<xsl:if test="Valid">
			<xsl:value-of select="string(' \&#xa;')"/>
			<xsl:text>{ \&#xa;</xsl:text>
			<xsl:value-of select="concat('&#x9;if(!ValidateState(', count(Valid/child::*), ', ')"/>
			<xsl:for-each select="Valid/child::*">
				<xsl:value-of select="string(name())"/>
				<xsl:if test="position() != last()">
					<xsl:text>, </xsl:text>
				</xsl:if>
			</xsl:for-each>
			<xsl:value-of select="string(') ) \&#xa;')"/>
			<xsl:text>&#x9;{ \&#xa;</xsl:text>
			<xsl:value-of select="concat('&#x9;&#x9;Aqsis::log() &lt;&lt; error &lt;&lt; &quot;Invalid state for ', @name, ' [&quot; &lt;&lt; GetStateAsString() &lt;&lt; &quot;]&quot; &lt;&lt; std::endl; \&#xa;')"/>
			<xsl:value-of select="string('&#x9;&#x9;return')"/>
			<xsl:if test="@return != 'RtVoid'">
				<xsl:text>(0)</xsl:text>
			</xsl:if>
			<xsl:text>; \&#xa;</xsl:text>
			<xsl:text>&#x9;} \&#xa;</xsl:text>
			<xsl:text>}&#xa;</xsl:text>
		</xsl:if>
		<xsl:text>&#xa;</xsl:text>
	</xsl:template>

</xsl:stylesheet>
