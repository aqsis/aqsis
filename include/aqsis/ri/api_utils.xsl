<?xml version="1.0" encoding="UTF-8" ?>

<!DOCTYPE interface [
	<!ENTITY cr "&#xa;">
	<!ENTITY tab "&#x9;">
	<!-- Ugly hack - need to redeclare xmlns:xsl to workaround an xsltproc bug. -->
	<!ENTITY space "<xsl:text xmlns:xsl='http://www.w3.org/1999/XSL/Transform'> </xsl:text>">
]>

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
	<xsl:output method="text"/>
	<xsl:strip-space elements="RiAPI"/>


	<!-- Get the procedure name.
	params:
		useVarargs - if true, get the varargs form of the name (eg, RiCurves),
		otherwise use the "V" form of the name (eg, RiCurvesV) when the
		procedure has a varargs list.
	-->
	<xsl:template match="Procedure" mode="procedure_name">
		<xsl:param name="useVarargs" select="false()"/>
		<xsl:value-of select="Name"/>
		<xsl:if test="not($useVarargs) and Arguments/ParamList">
			<xsl:text>V</xsl:text>
		</xsl:if>
	</xsl:template>



	<!-- Get the list of formal arguments for a procedure.
	params:
		useVarargs - if true, use the varargs (...) form for the formals,
		otherwise use the (count, tokens, values) form.
	-->
	<xsl:template match="Arguments" mode="procedure_formals">
		<xsl:param name="useVarargs" select="false()"/>
		<xsl:apply-templates select="Argument" mode="procedure_formals"/>
		<xsl:if test="ParamList">
			<xsl:if test="Argument"><xsl:text>, </xsl:text></xsl:if>
			<xsl:choose>
				<xsl:when test="$useVarargs">
					<xsl:text>...</xsl:text>
				</xsl:when>
				<xsl:otherwise>
					<xsl:text>RtInt count, RtToken tokens[], RtPointer values[]</xsl:text>
				</xsl:otherwise>
			</xsl:choose>
		</xsl:if>
	</xsl:template>
	<!-- helper template for procedure_formals -->
	<xsl:template match="Argument" mode="procedure_formals">
		<xsl:apply-templates select="." mode="argument_definition"><xsl:with-param name="interface" select="'C'"/></xsl:apply-templates>
		<xsl:if test="last() != position()">
		   <xsl:text>, </xsl:text>
		</xsl:if>
	</xsl:template>



	<!-- Print the definition of an argument
	eg1: RtToken name 
	eg2: RtInt nvertices[]
	params:
		interface - either "C" or "Rib", which determines whether the type of
					the argument is the one used in the C interface or the RIB
					parser.
	-->
	<xsl:template match="Argument" mode="argument_definition">
		<xsl:param name="interface" select="'C'"/>
		<xsl:variable name="type">
			<xsl:choose>
				<xsl:when test="$interface='C' and /RiAPI/Types/Type[Name=current()/Type]/C/Type">
					<xsl:value-of select="/RiAPI/Types/Type[Name=current()/Type]/C/Type"/>
				</xsl:when>
				<xsl:when test="$interface='Rib' and /RiAPI/Types/Type[Name=current()/Type]/Rib/Type">
					<xsl:value-of select="/RiAPI/Types/Type[Name=current()/Type]/Rib/Type"/>
				</xsl:when>
				<xsl:otherwise>
					<xsl:value-of select="Type"/>
				</xsl:otherwise>
			</xsl:choose>
		</xsl:variable>
		<xsl:choose>
			<xsl:when test="contains($type, '[]')">
				<xsl:value-of select="substring-before($type, '[]')"/>
				<xsl:text> </xsl:text>
				<xsl:value-of select="Name"/>
				<xsl:text>[]</xsl:text>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select="$type"/>
				<xsl:text> </xsl:text>
				<xsl:value-of select="Name"/>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<!-- Print the list of arguments for calling a RI procedure
	params:
		prefix - prefix to add to the parameter names
	-->
	<xsl:template match="Arguments" mode="procedure_call">
		<xsl:param name="prefix" select="''"/>
		<xsl:apply-templates select="Argument[Name!='...']" mode="procedure_call">
			<xsl:with-param name="prefix" select="$prefix"/>
		</xsl:apply-templates>
		<xsl:if test="ParamList">
			<xsl:if test="Argument"><xsl:text>, </xsl:text></xsl:if>
			<xsl:value-of select="$prefix"/>
			<xsl:text>count, </xsl:text>
			<xsl:value-of select="$prefix"/>
			<xsl:text>tokens, </xsl:text>
			<xsl:value-of select="$prefix"/>
			<xsl:text>values</xsl:text>
		</xsl:if>
	</xsl:template>
	<xsl:template match="Argument" mode="procedure_call">
		<xsl:param name="prefix" select="''"/>
		<xsl:value-of select="$prefix"/>
		<xsl:value-of select="Name"/>
		<xsl:if test="last() != position()">
		   <xsl:text>, </xsl:text>
		</xsl:if>
	</xsl:template>

</xsl:stylesheet>
