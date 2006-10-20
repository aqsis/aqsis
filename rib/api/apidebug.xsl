<?xml version="1.0" encoding="UTF-8" ?>


<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">


	<xsl:output method="text"/>
	<xsl:strip-space elements="RiAPI"/>


	<!--	API	-->
	<xsl:template match="RiAPI">
		<!--	Procedures	-->
		<xsl:apply-templates select="Procedures/Procedure"/>
		<xsl:apply-templates select="Procedures/Procedure" mode="macro"/>
	</xsl:template>


	<!--	Procedure	-->
	<xsl:template match="Procedure">
		<xsl:value-of select="concat('void ', @name, 'Debug(')"/>
		<xsl:apply-templates select="Arguments/Argument" mode="arg_parameters"/>
		<xsl:text>)&#xa;</xsl:text>
		<xsl:text>{&#xa;</xsl:text>
		<xsl:text>&#x9;std::stringstream _message;&#xa;</xsl:text>
		<xsl:value-of select="concat('&#x9;_message &lt;&lt; &quot;', @name, ' &quot;;&#xa;')"/>
		<xsl:apply-templates select="Arguments/Argument" mode="arg_output"/>
		<xsl:text>&#x9;Aqsis::log() &lt;&lt; debug &lt;&lt; _message.str().c_str() &lt;&lt; std::endl;&#xa;</xsl:text>
		<xsl:text>}&#xa;</xsl:text>
	</xsl:template>

	<xsl:template match="Procedure" mode="macro">
		<xsl:value-of select="concat('#define DEBUG_', translate(@name, 'abcdefghijklmnopqrstuvwxyz', 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'), ' ')"/>
		<xsl:value-of select="concat(@name, 'Debug(')"/>
		<xsl:apply-templates select="Arguments/Argument" mode="macro_call"/>
		<xsl:text>);&#xa;</xsl:text>
	</xsl:template>

	<!--	Argument copy within the constructor	-->
	<xsl:template match="Argument" mode="arg_output">
		<xsl:choose>
			<xsl:when test="@type = 'PARAMETERLIST'">
				<xsl:text>&#x9;// Output the plist here.&#xa;</xsl:text>
				<xsl:text>	int constant_size = 1;
	int uniform_size = 1;
	int varying_size = 1;
	int vertex_size = 1;
	int facevarying_size = 1;
</xsl:text>
				<xsl:if test="../../ConstantSize">
					<xsl:value-of select="../../ConstantSize"/>
				</xsl:if>
				<xsl:if test="../../UniformSize">
					<xsl:value-of select="../../UniformSize"/>
				</xsl:if>
				<xsl:if test="../../VaryingSize">
					<xsl:value-of select="../../VaryingSize"/>
				</xsl:if>
				<xsl:if test="../../VertexSize">
					<xsl:value-of select="../../VertexSize"/>
				</xsl:if>
				<xsl:if test="../../FaceVaryingSize">
					<xsl:value-of select="../../FaceVaryingSize"/>
				</xsl:if>
				<xsl:text>	DebugPlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size, _message);&#xa;</xsl:text>
			</xsl:when>
			<xsl:when test="contains( @type, 'Array')">
				<xsl:choose>
					<xsl:when test="./Length">
						<xsl:value-of select="./Length"/>
					</xsl:when>
					<xsl:otherwise>
						<xsl:text>&#x9;// \note: Need to specify the length method here!&#xa;</xsl:text>
						<xsl:value-of select="concat('&#x9;int __', @name, '_length = 1;&#xa;')"/>
					</xsl:otherwise>
				</xsl:choose>
				<xsl:value-of select="concat('&#x9;_message &lt;&lt; ', @name, ';&#xa;')"/>
				<xsl:value-of select="concat('&#x9;int __', @name, '_index;&#xa;')"/>
				<xsl:value-of select="concat('&#x9;for(__', @name, '_index = 0; __', @name, '_index&lt;__', @name, '_length; __', @name, '_index++)&#xa;')"/>
				<xsl:text>&#x9;{&#xa;</xsl:text>
				<xsl:choose>
					<xsl:when test="@type = 'RtTokenArray' or @type = 'RtStringArray'">
						<xsl:value-of select="concat('&#x9;&#x9;_message &lt;&lt; ', @name, '[__', @name, '_index];&#xa;')"/>
					</xsl:when>
					<xsl:when test="@type = 'RtColorArray' or @type = 'RtPointArray'">
						<xsl:value-of select="concat('&#x9;&#x9;_message &lt;&lt; ', @name, '[__', @name, '_index][0] &lt;&lt; &quot; &quot; &lt;&lt; ', @name, '[__', @name, '_index][1] &lt;&lt; &quot; &quot; &lt;&lt; ', @name, '[__', @name, '_index][2];&#xa;')"/>
					</xsl:when>
					<xsl:otherwise>
						<xsl:value-of select="concat('&#x9;&#x9;_message &lt;&lt; ', @name, '[__', @name, '_index];&#xa;')"/>
					</xsl:otherwise>
				</xsl:choose>
				<xsl:text>&#x9;}&#xa;</xsl:text>
			</xsl:when>
			<xsl:otherwise>
				<xsl:choose>
					<xsl:when test="@type = 'RtToken' or @type = 'RtString'">
						<xsl:value-of select="concat('&#x9;_message &lt;&lt; &quot;\&quot;&quot; &lt;&lt; ', @name, ' &lt;&lt; &quot;\&quot; &quot;;&#xa;')"/>
					</xsl:when>
					<xsl:when test="@type = 'RtColor' or @type = 'RtPoint'">
						<xsl:value-of select="concat('&#x9;_message &lt;&lt; ', @name, '[0] &lt;&lt; &quot; &quot; &lt;&lt; ', @name, '[1] &lt;&lt; &quot; &quot; &lt;&lt; ', @name, '[2];&#xa;')"/>
					</xsl:when>
					<xsl:when test="@type = 'RtMatrix' or @type = 'RtBasis'">
						<xsl:value-of select="string('&#x9;_message &lt;&lt; &quot;[&quot;;&#xa;')"/>
						<xsl:value-of select="concat('&#x9;int __', @name, '_i, __', @name, '_j;&#xa;')"/>
						<xsl:value-of select="concat('&#x9;for(__', @name, '_j = 0; __', @name, '_j&lt;4; __', @name, '_j++)&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;for(__', @name, '_i = 0; __', @name, '_i&lt;4; __', @name, '_i++)&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;&#x9;_message &lt;&lt; ', @name, '[__', @name, '_j][__', @name, '_i] &lt;&lt; &quot; &quot;;&#xa;')"/>
						<xsl:value-of select="string('&#x9;_message &lt;&lt; &quot;]&quot; &lt;&lt; &quot; &quot;;&#xa;')"/>
					</xsl:when>
					<xsl:when test="@type = 'RtBound'">
						<xsl:value-of select="concat('&#x9;_message &lt;&lt; ', @name, '[0] &lt;&lt; &quot; &quot;;&#xa;')"/>
						<xsl:value-of select="concat('&#x9;_message &lt;&lt; ', @name, '[1] &lt;&lt; &quot; &quot;;&#xa;')"/>
						<xsl:value-of select="concat('&#x9;_message &lt;&lt; ', @name, '[2] &lt;&lt; &quot; &quot;;&#xa;')"/>
						<xsl:value-of select="concat('&#x9;_message &lt;&lt; ', @name, '[3] &lt;&lt; &quot; &quot;;&#xa;')"/>
						<xsl:value-of select="concat('&#x9;_message &lt;&lt; ', @name, '[4] &lt;&lt; &quot; &quot;;&#xa;')"/>
						<xsl:value-of select="concat('&#x9;_message &lt;&lt; ', @name, '[5] &lt;&lt; &quot; &quot;;&#xa;')"/>
					</xsl:when>
					<xsl:otherwise>
						<xsl:if test="not(@name = '...') and not(@name = '')">
							<xsl:value-of select="concat('&#x9;_message &lt;&lt; ', @name, ' &lt;&lt; &quot; &quot;;&#xa;')"/>
						</xsl:if>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>


	<!--	Argument to the cache constructor	-->
	<xsl:template match="Argument" mode="arg_parameters">
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

	<!--	Arguments to macro construct method	-->
	<xsl:template match="Argument" mode="macro_call">
		<xsl:choose>
			<xsl:when test="@type = 'PARAMETERLIST'">
				<xsl:if test="position() != 1">
				   <xsl:text>, </xsl:text>
				</xsl:if>
				<xsl:text>count, tokens, values</xsl:text>
			</xsl:when>
			<xsl:otherwise>
				<xsl:if test="not(@type = '')">
					<xsl:if test="position() != 1">
					   <xsl:text>, </xsl:text>
					</xsl:if>
					<xsl:value-of select="@name"/>
				</xsl:if>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>


</xsl:stylesheet>
