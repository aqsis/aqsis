<?xml version="1.0" encoding="UTF-8" ?>

<!DOCTYPE interface [
	<!ENTITY cr "&#xa;">
	<!ENTITY tab "&#x9;">
]>

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
	<xsl:output method="text"/>
	<xsl:strip-space elements="RiAPI"/>
	<xsl:include href="api_utils.xsl"/>

	<xsl:template match="RiAPI">
		<xsl:apply-templates select="Procedures/Procedure"/>
	</xsl:template>

	<xsl:template match="Procedure">
		<xsl:call-template name="procedure_definition">
			<xsl:with-param name="useVarargs" select="true()"/>
		</xsl:call-template>
		<xsl:if test="Arguments/ParamList">
			<xsl:call-template name="procedure_definition">
				<xsl:with-param name="useVarargs" select="false()"/>
			</xsl:call-template>
		</xsl:if>
	</xsl:template>

	<xsl:template name="procedure_definition">
		<xsl:param name="useVarargs"/>
		<xsl:text>&tab;RI_SHARE </xsl:text>
		<xsl:value-of select="ReturnType"/>
		<xsl:text> </xsl:text>
		<xsl:apply-templates select="." mode="procedure_name">
			<xsl:with-param name="useVarargs" select="$useVarargs"/>
		</xsl:apply-templates>
		<xsl:text>( </xsl:text>
		<xsl:apply-templates select="Arguments" mode="procedure_formals">
			<xsl:with-param name="useVarargs" select="$useVarargs"/>
		</xsl:apply-templates>
		<xsl:text> );&cr;</xsl:text>
	</xsl:template>

</xsl:stylesheet>
