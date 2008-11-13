<?xml version="1.0" encoding="UTF-8" ?>

<!DOCTYPE interface [
	<!ENTITY cr "&#xa;">
	<!ENTITY tab "&#x9;">
]>

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
	<xsl:output method="text"/>
	<xsl:strip-space elements="RiAPI"/>

	<xsl:template match="RiAPI">
		<xsl:apply-templates select="Procedures/Procedure"/>
		<xsl:text>&cr;</xsl:text>
	</xsl:template>

	<!-- Many RI calls are only valid within a given scope.  This is 
	-->
	<xsl:template match="Procedure">
#define VALIDATE_<xsl:value-of select="translate(Name, 'abcdefghijklmnopqrstuvwxyz', 'ABCDEFGHIJKLMNOPQRSTUVWXYZ')"/>
<xsl:if test="ValidScope"> \
{ \
	if(!ValidateState(<xsl:value-of select="count(ValidScope/*)"/>, <xsl:apply-templates select="ValidScope/*"/>) ) \
	{ \
		Aqsis::log() &lt;&lt; error &lt;&lt; "Invalid state for <xsl:value-of select="Name"/> [" &lt;&lt; GetStateAsString() &lt;&lt; "]" &lt;&lt; std::endl; \
		return<xsl:if test="ReturnType != 'RtVoid'">(0)</xsl:if>; \
	} \
}
</xsl:if>
	</xsl:template>

	<xsl:template match="ValidScope/*">
		<xsl:value-of select="name()"/>
		<xsl:if test="position() != last()">
			<xsl:text>, </xsl:text>
		</xsl:if>
	</xsl:template>

</xsl:stylesheet>
