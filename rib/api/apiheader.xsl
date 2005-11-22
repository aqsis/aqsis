<?xml version="1.0" encoding="UTF-8" ?>


<!DOCTYPE interface [
	<!ENTITY newline	"<xsl:text>
</xsl:text>">
	<!ENTITY tabulate	"<xsl:text>	</xsl:text>">
]>

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">


	<xsl:output method="text"/>
	<xsl:strip-space elements="interface"/>


	<!--	API	-->
	<xsl:template match="RiAPI">
		<!--	Procedures	-->
		<xsl:apply-templates select="Procedures/Procedure"/>
	</xsl:template>


	<!--	Procedure	-->
	<xsl:template match="Procedure">
		<xsl:choose>
			<xsl:when test="Arguments/Argument[last()]/@type = 'PARAMETERLIST'">
				<xsl:value-of select="concat('&#x9;_qShare ', @return, ' ', @name, '( ')"/>
				<xsl:apply-templates select="Arguments/Argument[position()&lt;last()]"/>
				<xsl:value-of select="string(', ... );&#xa;')"/>
				<xsl:value-of select="concat('&#x9;_qShare ', @return, ' ', @name, 'V( ')"/>
				<xsl:apply-templates select="Arguments/Argument"/>
				<xsl:value-of select="string(' );&#xa;')"/>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select="concat('&#x9;_qShare ', @return, ' ', @name, '( ')"/>
				<xsl:apply-templates select="Arguments/Argument"/>
				<xsl:value-of select="string(' );&#xa;')"/>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>


	<!--	Argument	-->
	<xsl:template match="Argument">
		<xsl:choose>
			<xsl:when test="@type = 'PARAMETERLIST'">
				<xsl:text>RtInt count, RtToken tokens[], RtPointer values[]</xsl:text>
			</xsl:when>
			<xsl:when test="contains( @type, 'Array')">
				<xsl:value-of select="concat(substring-before(@type, 'Array'), ' ', @name, '[]')"/>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select="concat(@type, ' ', @name)"/>
			</xsl:otherwise>
		</xsl:choose>
		<xsl:if test="last() != position()">
		   <xsl:text>, </xsl:text>
		</xsl:if>
	</xsl:template>

</xsl:stylesheet>
