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
		<xsl:value-of select="concat('void ', Name, 'Debug(')"/>
		<xsl:apply-templates select="Arguments/Argument" mode="arg_parameters"/>
		<xsl:text>)&#xa;</xsl:text>
		<xsl:text>{&#xa;</xsl:text>
		<xsl:text>&#x9;if(QGetRenderContext() == 0 || !QGetRenderContext()->poptCurrent()) return;&#xa;</xsl:text>
		<xsl:text>&#x9;const TqInt* poptEcho = QGetRenderContext()->poptCurrent()->GetIntegerOption( "statistics", "echoapi" );&#xa;</xsl:text>
		<xsl:text>&#x9;if(poptEcho != 0 &amp;&amp; *poptEcho != 0 )&#xa;</xsl:text>
		<xsl:text>&#x9;{&#xa;</xsl:text>
		<xsl:text>&#x9;&#x9;std::stringstream _message;&#xa;</xsl:text>
		<xsl:value-of select="concat('&#x9;&#x9;_message &lt;&lt; &quot;', Name, ' &quot;;&#xa;')"/>
		<xsl:apply-templates select="Arguments/Argument" mode="arg_output"/>
		<xsl:text>&#x9;&#x9;Aqsis::log() &lt;&lt;  _message.str().c_str() &lt;&lt; std::endl;&#xa;</xsl:text>
		<xsl:text>&#x9;}&#xa;</xsl:text>
		<xsl:text>}&#xa;</xsl:text>
	</xsl:template>

	<xsl:template match="Procedure" mode="macro">
		<xsl:value-of select="concat('#define DEBUG_', translate(Name, 'abcdefghijklmnopqrstuvwxyz', 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'), ' ')"/>
		<xsl:value-of select="concat(Name, 'Debug(')"/>
		<xsl:apply-templates select="Arguments/Argument" mode="macro_call"/>
		<xsl:text>);&#xa;</xsl:text>
	</xsl:template>

	<!--	Argument copy within the constructor	-->
	<xsl:template match="Argument" mode="arg_output">
		<xsl:choose>
			<xsl:when test="Type = 'PARAMETERLIST'">
				<xsl:text>&#x9;&#x9;// Output the plist here.&#xa;</xsl:text>
				<xsl:text>		int constant_size = 1;
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
				<xsl:text>		DebugPlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size, _message);&#xa;</xsl:text>
			</xsl:when>
			<xsl:when test="contains( Type, 'Array')">
				<xsl:choose>
					<xsl:when test="./Length">
						<xsl:value-of select="./Length"/>
					</xsl:when>
					<xsl:otherwise>
						<xsl:text>&#x9;// \note: Need to specify the length method here!&#xa;</xsl:text>
						<xsl:value-of select="concat('&#x9;int __', Name, '_length = 1;&#xa;')"/>
					</xsl:otherwise>
				</xsl:choose>
				<xsl:value-of select="concat('&#x9;&#x9;_message &lt;&lt; ', Name, ';&#xa;')"/>
				<xsl:value-of select="concat('&#x9;&#x9;int __', Name, '_index;&#xa;')"/>
				<xsl:value-of select="concat('&#x9;&#x9;for(__', Name, '_index = 0; __', Name, '_index&lt;__', Name, '_length; __', Name, '_index++)&#xa;')"/>
				<xsl:text>&#x9;&#x9;{&#xa;</xsl:text>
				<xsl:choose>
					<xsl:when test="Type = 'RtTokenArray' or Type = 'RtStringArray'">
						<xsl:value-of select="concat('&#x9;&#x9;&#x9;_message &lt;&lt; ', Name, '[__', Name, '_index];&#xa;')"/>
					</xsl:when>
					<xsl:when test="Type = 'RtColorArray' or Type = 'RtPointArray'">
						<xsl:value-of select="concat('&#x9;&#x9;&#x9;_message &lt;&lt; ', Name, '[__', Name, '_index][0] &lt;&lt; &quot; &quot; &lt;&lt; ', Name, '[__', Name, '_index][1] &lt;&lt; &quot; &quot; &lt;&lt; ', Name, '[__', Name, '_index][2];&#xa;')"/>
					</xsl:when>
					<xsl:otherwise>
						<xsl:value-of select="concat('&#x9;&#x9;&#x9;_message &lt;&lt; ', Name, '[__', Name, '_index];&#xa;')"/>
					</xsl:otherwise>
				</xsl:choose>
				<xsl:text>&#x9;}&#xa;</xsl:text>
			</xsl:when>
			<xsl:otherwise>
				<xsl:choose>
					<xsl:when test="Type = 'RtToken' or Type = 'RtString'">
						<xsl:value-of select="concat('&#x9;&#x9;_message &lt;&lt; &quot;\&quot;&quot; &lt;&lt; ', Name, ' &lt;&lt; &quot;\&quot; &quot;;&#xa;')"/>
					</xsl:when>
					<xsl:when test="Type = 'RtColor' or Type = 'RtPoint'">
						<xsl:value-of select="concat('&#x9;&#x9;_message &lt;&lt; ', Name, '[0] &lt;&lt; &quot; &quot; &lt;&lt; ', Name, '[1] &lt;&lt; &quot; &quot; &lt;&lt; ', Name, '[2];&#xa;')"/>
					</xsl:when>
					<xsl:when test="Type = 'RtMatrix' or Type = 'RtBasis'">
						<xsl:value-of select="string('&#x9;&#x9;_message &lt;&lt; &quot;[&quot;;&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;int __', Name, '_i, __', Name, '_j;&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;for(__', Name, '_j = 0; __', Name, '_j&lt;4; __', Name, '_j++)&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;&#x9;for(__', Name, '_i = 0; __', Name, '_i&lt;4; __', Name, '_i++)&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;&#x9;&#x9;_message &lt;&lt; ', Name, '[__', Name, '_j][__', Name, '_i] &lt;&lt; &quot; &quot;;&#xa;')"/>
						<xsl:value-of select="string('&#x9;&#x9;_message &lt;&lt; &quot;]&quot; &lt;&lt; &quot; &quot;;&#xa;')"/>
					</xsl:when>
					<xsl:when test="Type = 'RtBound'">
						<xsl:value-of select="concat('&#x9;&#x9;_message &lt;&lt; ', Name, '[0] &lt;&lt; &quot; &quot;;&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;_message &lt;&lt; ', Name, '[1] &lt;&lt; &quot; &quot;;&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;_message &lt;&lt; ', Name, '[2] &lt;&lt; &quot; &quot;;&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;_message &lt;&lt; ', Name, '[3] &lt;&lt; &quot; &quot;;&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;_message &lt;&lt; ', Name, '[4] &lt;&lt; &quot; &quot;;&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;_message &lt;&lt; ', Name, '[5] &lt;&lt; &quot; &quot;;&#xa;')"/>
					</xsl:when>
					<xsl:otherwise>
						<xsl:if test="not(Name = '...') and not(Name = '')">
							<xsl:value-of select="concat('&#x9;&#x9;_message &lt;&lt; ', Name, ' &lt;&lt; &quot; &quot;;&#xa;')"/>
						</xsl:if>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>


	<!--	Argument to the cache constructor	-->
	<xsl:template match="Argument" mode="arg_parameters">
		<xsl:choose>
			<xsl:when test="Type = 'PARAMETERLIST'">
				<xsl:text>RtInt count, RtToken tokens[], RtPointer values[]</xsl:text>
			</xsl:when>
			<xsl:when test="contains( Type, 'Array')">
				<xsl:value-of select="concat(substring-before(Type, 'Array'), ' ', Name, '[]')"/>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select="concat(Type, ' ', Name)"/>
			</xsl:otherwise>
		</xsl:choose>
		<xsl:if test="last() != position()">
		   <xsl:text>, </xsl:text>
		</xsl:if>
	</xsl:template>

	<!--	Arguments to macro construct method	-->
	<xsl:template match="Argument" mode="macro_call">
		<xsl:choose>
			<xsl:when test="Type = 'PARAMETERLIST'">
				<xsl:if test="position() != 1">
				   <xsl:text>, </xsl:text>
				</xsl:if>
				<xsl:text>count, tokens, values</xsl:text>
			</xsl:when>
			<xsl:otherwise>
				<xsl:if test="not(Type = '')">
					<xsl:if test="position() != 1">
					   <xsl:text>, </xsl:text>
					</xsl:if>
					<xsl:value-of select="Name"/>
				</xsl:if>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>


</xsl:stylesheet>
