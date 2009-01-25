<?xml version="1.0" encoding="UTF-8" ?>

<!DOCTYPE interface [
	<!ENTITY cr "&#xa;">
	<!ENTITY tab "&#x9;">
]>

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<!-- Stylesheet setup -->
<xsl:output method="text"/>
<xsl:strip-space elements="RiAPI"/>
<xsl:include href="api_utils.xsl"/>


<!-- Main entry point for matches. -->
<xsl:template match="RiAPI">
	<xsl:apply-templates select="Procedures/Procedure[Rib and not(Rib/CustomImpl)]"/>
</xsl:template>


<!-- ====================================================================== -->
<!--
For each RI procedure, create a method to recognize the form of the RIB
binding for that procedure.
-->
<xsl:template match="Procedure">
void CqRibRequestHandler::handle<xsl:value-of select="substring-after(Name,'Ri')"/>(IqRibParser&amp; parser)
{
<!--	// Extract all arguments for the RIB binding of the current procedure from
	// the parser, and compute the implicit arguments given by array lengths.  
	-->
<xsl:apply-templates select="." mode="collect_arguments"/>
<!--
	// Compute any remaining arguments necessary for the C binding
<xsl:apply-templates select="Arguments/Argument" mode="computed_arguments"/>
	// Validate lengths of all argument arrays (TODO)
<xsl:apply-templates select="Arguments/Argument" mode="check_array_lengths"/>
-->
	// Call through to the C binding.
	<xsl:apply-templates select="." mode="procedure_name"/>(<xsl:apply-templates select="Arguments/Argument" mode="procedure_call_args"/>
	<xsl:if test="Arguments/ParamList">, paramList.count(), paramList.tokens(), paramList.values()</xsl:if>);
}
</xsl:template>


<!-- ====================================================================== -->
<!-- For each argument in an RI call, extract the argument from the RIB parser.
-->
<xsl:template match="Procedure" mode="collect_arguments">

	<!-- First collect an array of all args in the case that the argument
	list can be an array -->
<xsl:if test="Arguments/RibArgsCanBeArray">
	// Collect all args as an array
	const IqRibParser::TqFloatArray&amp; allArgs = parser.getFloatArray(<xsl:value-of select="count(Arguments/Argument)"/>);
</xsl:if>

	<!-- Next collect arguments which don't have special values specified
	for RIB -->
<xsl:if test="Arguments/Argument[not(RibValue)]">
	// Collect arguments from parser.
<xsl:apply-templates select="Arguments/Argument[not(RibValue)]"
		mode="collect_arguments"/>
</xsl:if>

	<!-- Collect the parameter list if necessary -->
<xsl:if test="Arguments/ParamList">
	// Extract the parameter list
	CqParamListHandler paramList(m_tokenDict);
	parser.getParamList(paramList);
</xsl:if>
	<!-- // Validate the length of the parameter arrays (TODO) -->

	<!-- Finally collect arguments which have special values for the RIB
	binding.  These can depend on arguments previously collected. -->
<xsl:if test="Arguments/Argument[RibValue]">
	<xsl:text>&cr;</xsl:text>
	<xsl:apply-templates select="Arguments/Argument[RibValue]"
		mode="collect_arguments"/>
</xsl:if>
</xsl:template>

<!-- ================================================== -->
<!-- Collect a single argument from the RIB parser or from other sources,
depending on what is specified in the API xml doc. -->
<xsl:template match="Argument" mode="collect_arguments">
	<xsl:text>&tab;</xsl:text>
	<xsl:value-of select="/RiAPI/Types/Type[Name=current()/Type]/Rib/Type"/>
	<xsl:text> </xsl:text>
	<xsl:value-of select="Name"/>
	<xsl:text> = </xsl:text>
	<xsl:choose>
		<xsl:when test="../RibArgsCanBeArray">
			<!-- Get the value from the array of all args in the special
				case that all args are in an array (WHY Pixar?!) -->
			<xsl:text>allArgs[</xsl:text>
			<xsl:value-of select="position()-1"/>
			<xsl:text>]</xsl:text>
		</xsl:when>
		<xsl:when test="RibValue">
			<xsl:value-of select="RibValue"/>
		</xsl:when>
		<xsl:when test="/RiAPI/Types/Type[Name=current()/Type]/Rib/Getter">
			<!-- Extract the variable from the parser if an appropriate
				Getter is defined -->
			<xsl:text>parser.</xsl:text>
			<xsl:value-of select="/RiAPI/Types/Type[Name=current()/Type]/Rib/Getter"/>
			<xsl:text>(</xsl:text>
			<xsl:choose>
				<xsl:when test="GetterArgs">
					<xsl:value-of select="GetterArgs"/>
				</xsl:when>
				<xsl:otherwise>
					<xsl:value-of select="/RiAPI/Types/Type[Name=current()/Type]/Rib/GetterArgs"/>
				</xsl:otherwise>
			</xsl:choose>
			<xsl:text>)</xsl:text>
		</xsl:when>
		<xsl:otherwise>
			<!-- Else use a default value for the variable -->
			<xsl:value-of select="/RiAPI/Types/Type[Name=current()/Type]/Rib/DefaultValue"/>
		</xsl:otherwise>
	</xsl:choose>
	<xsl:text>;&cr;</xsl:text>
</xsl:template>


<!-- ====================================================================== -->
<!--
Output the arguments of the RI call in a list suitable for calling the
-->
<xsl:template match="Argument" mode="procedure_call_args">
	<xsl:text>toRiType(</xsl:text>
	<xsl:value-of select="Name"/>
	<xsl:text>)</xsl:text>
	<xsl:if test="position() != last()">
		<xsl:text>, </xsl:text>
	</xsl:if>
</xsl:template>


</xsl:stylesheet>
