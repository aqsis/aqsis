<?xml version="1.0" encoding="UTF-8" ?>


<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">


	<xsl:output method="text"/>
	<xsl:strip-space elements="RiAPI"/>


	<!--	API	-->
	<xsl:template match="RiAPI">
		<!--	Procedures	-->
		<xsl:apply-templates select="Procedures/Procedure"/>
		<xsl:apply-templates select="Procedures/Procedure" mode="macro"/>
		<xsl:text>&#xa;&#xa;</xsl:text>
	</xsl:template>


	<!--	Procedure	-->
	<xsl:template match="Procedure">
		<xsl:if test="not(./NoCache)">
			<xsl:value-of select="concat('class ', @name, 'Cache : public RiCacheBase&#xa;')"/>
			<xsl:text>{&#xa;</xsl:text>
			<xsl:text>public:&#xa;</xsl:text>
			<xsl:call-template name="Constructor"/>
			<xsl:call-template name="Destructor"/>
			<xsl:value-of select="concat('&#x9;virtual void ReCall()&#xa;&#x9;{&#xa;&#x9;&#x9;', @name)"/>
			<xsl:if test="Arguments/Argument[last()]/@type = 'PARAMETERLIST'">
				<xsl:text>V</xsl:text>
			</xsl:if>
			<xsl:text>(</xsl:text>
			<xsl:apply-templates select="Arguments/Argument" mode="recall_args"/>
			<xsl:text>);&#xa;</xsl:text>
			<xsl:text>&#x9;}&#xa;</xsl:text>
			<xsl:text>&#xa;</xsl:text>
			<xsl:text>private:&#xa;</xsl:text>
			<xsl:apply-templates name="Argument" mode="member_vars" select="Arguments/Argument"/>
			<xsl:text>};&#xa;&#xa;</xsl:text>
		</xsl:if>
	</xsl:template>

	<xsl:template match="Procedure" mode="macro">
		<xsl:if test="not(./NoCache)">
			<xsl:value-of select="concat('#define CACHE_', translate(@name,'abcdefghijklmnopqrstuvwxyz', 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'), '\&#xa;')"/>
			<xsl:value-of select="string('&#x9;if( QGetRenderContext()->pCurrentObject()) \&#xa;')"/>
			<xsl:text>	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					</xsl:text>
			<xsl:value-of select="concat('new ', @name, 'Cache(')"/>
			<xsl:apply-templates select="Arguments/Argument" mode="macro_call"/>
			<xsl:value-of select="string(') ); \&#xa;')"/>
			<xsl:text>		return</xsl:text>
			<xsl:if test="@return != 'RtVoid'">
				<xsl:text>(0)</xsl:text>
			</xsl:if>
			<xsl:text>;	\
	}
	</xsl:text>
		</xsl:if>
	</xsl:template>

	<xsl:template name="Constructor">
		<xsl:value-of select="concat('&#x9;', @name, 'Cache(')"/>
		<xsl:apply-templates select="Arguments/Argument" mode="constructor_sig"/>
		<xsl:text>) : RiCacheBase()&#xa;&#x9;{&#xa;</xsl:text>
		<xsl:apply-templates select="Arguments/Argument" mode="constructor_copy"/>
		<xsl:text>&#x9;}&#xa;</xsl:text>
	</xsl:template>

	<xsl:template name="Destructor">
		<xsl:value-of select="concat('&#x9;virtual ~', @name, 'Cache()&#xa;&#x9;{&#xa;')"/>
		<xsl:apply-templates select="Arguments/Argument" mode="destructor"/>
		<xsl:text>&#x9;}&#xa;</xsl:text>
	</xsl:template>

	<!--	Argument to the cache constructor	-->
	<xsl:template match="Argument" mode="constructor_sig">
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

	<!--	Argument copy within the constructor	-->
	<xsl:template match="Argument" mode="constructor_copy">
		<xsl:choose>
			<xsl:when test="@type = 'PARAMETERLIST'">
				<xsl:text>&#x9;&#x9;// Copy the plist here.&#xa;</xsl:text>
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
				<xsl:text>		CachePlist(count, tokens, values, constant_size, uniform_size, varying_size, vertex_size, facevarying_size);&#xa;</xsl:text>
			</xsl:when>
			<xsl:when test="contains( @type, 'Array')">
				<xsl:choose>
					<xsl:when test="./Length">
						<xsl:value-of select="./Length"/>
					</xsl:when>
					<xsl:otherwise>
						<xsl:text>&#x9;&#x9;// \note: Need to specify the length method here!&#xa;</xsl:text>
						<xsl:value-of select="concat('&#x9;&#x9;int __', @name, '_length = 1;&#xa;')"/>
					</xsl:otherwise>
				</xsl:choose>
				<xsl:value-of select="concat('&#x9;&#x9;m_', @name, ' = new ', substring-before(@type, 'Array'), '[__', @name, '_length];&#xa;')"/>
				<xsl:value-of select="concat('&#x9;&#x9;int __', @name, '_index;&#xa;')"/>
				<xsl:value-of select="concat('&#x9;&#x9;for(__', @name, '_index = 0; __', @name, '_index&lt;__', @name, '_length; __', @name, '_index++)&#xa;')"/>
				<xsl:text>&#x9;&#x9;{&#xa;</xsl:text>
				<xsl:choose>
					<xsl:when test="@type = 'RtTokenArray' or @type = 'RtStringArray'">
						<xsl:value-of select="concat('&#x9;&#x9;&#x9;int __', @name, '_slength = strlen(', @name, '[__', @name, '_index]);&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;&#x9;m_', @name, '[__', @name, '_index] = new char[ __', @name, '_slength + 1 ];&#xa;')"/> 
						<xsl:value-of select="concat('&#x9;&#x9;&#x9;strcpy(m_', @name, '[__', @name, '_index], ', @name, '[__', @name, '_index]);&#xa;')"/>
					</xsl:when>
					<xsl:when test="@type = 'RtColorArray' or @type = 'RtPointArray'">
						<xsl:value-of select="concat('&#x9;&#x9;&#x9;m_', @name, '[__', @name, '_index][0] = ', @name, '[__', @name, '_index][0];&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;&#x9;m_', @name, '[__', @name, '_index][1] = ', @name, '[__', @name, '_index][1];&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;&#x9;m_', @name, '[__', @name, '_index][2] = ', @name, '[__', @name, '_index][2];&#xa;')"/>
					</xsl:when>
					<xsl:otherwise>
						<xsl:value-of select="concat('&#x9;&#x9;&#x9;m_', @name, '[__', @name, '_index] = ', @name, '[__', @name, '_index];&#xa;')"/>
					</xsl:otherwise>
				</xsl:choose>
				<xsl:text>&#x9;&#x9;}&#xa;</xsl:text>
			</xsl:when>
			<xsl:otherwise>
				<xsl:choose>
					<xsl:when test="@type = 'RtToken' or @type = 'RtString'">
						<xsl:value-of select="concat('&#x9;&#x9;int __', @name, '_length = strlen(', @name, ');&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;m_', @name, ' = new char[ __', @name, '_length + 1 ];&#xa;')"/> 
						<xsl:value-of select="concat('&#x9;&#x9;strcpy(m_', @name, ', ', @name, ');&#xa;')"/>
					</xsl:when>
					<xsl:when test="@type = 'RtColor' or @type = 'RtPoint'">
						<xsl:value-of select="concat('&#x9;&#x9;m_', @name, '[0] = ', @name, '[0];&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;m_', @name, '[1] = ', @name, '[1];&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;m_', @name, '[2] = ', @name, '[2];&#xa;')"/>
					</xsl:when>
					<xsl:when test="@type = 'RtMatrix' or @type = 'RtBasis'">
						<xsl:value-of select="concat('&#x9;&#x9;int __', @name, '_i, __', @name, '_j;&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;for(__', @name, '_j = 0; __', @name, '_j&lt;4; __', @name, '_j++)&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;&#x9;for(__', @name, '_i = 0; __', @name, '_i&lt;4; __', @name, '_i++)&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;&#x9;&#x9;m_', @name, '[__', @name, '_j][__', @name, '_i] = ', @name, '[__', @name, '_j][__', @name, '_i];&#xa;')"/>
					</xsl:when>
					<xsl:when test="@type = 'RtBound'">
						<xsl:value-of select="concat('&#x9;&#x9;m_', @name, '[0] = ', @name, '[0];&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;m_', @name, '[1] = ', @name, '[1];&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;m_', @name, '[2] = ', @name, '[2];&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;m_', @name, '[3] = ', @name, '[3];&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;m_', @name, '[4] = ', @name, '[4];&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;m_', @name, '[5] = ', @name, '[5];&#xa;')"/>
					</xsl:when>
					<xsl:otherwise>
						<xsl:value-of select="concat('&#x9;&#x9;m_', @name, ' = ', @name, ';&#xa;')"/>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<!--	Argument storage members	-->
	<xsl:template match="Argument" mode="member_vars">
		<xsl:text>&#x9;</xsl:text>
		<xsl:choose>
			<xsl:when test="@type = 'PARAMETERLIST'">
				<xsl:text>// plist information is stored in the base class.</xsl:text>
			</xsl:when>
			<xsl:when test="contains( @type, 'Array')">
				<xsl:value-of select="concat(substring-before(@type, 'Array'), '* m_', @name, ';')"/>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select="concat(@type, ' m_', @name, ';')"/>
			</xsl:otherwise>
		</xsl:choose>
		<xsl:text>&#xa;</xsl:text>
	</xsl:template>

	<!--	Arguments to recall method	-->
	<xsl:template match="Argument" mode="recall_args">
		<xsl:choose>
			<xsl:when test="@type = 'PARAMETERLIST'">
				<xsl:text>m_count, m_tokens, m_values</xsl:text>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select="concat('m_', @name)"/>
			</xsl:otherwise>
		</xsl:choose>
		<xsl:if test="last() != position()">
		   <xsl:text>, </xsl:text>
		</xsl:if>
	</xsl:template>

	<!--	Argument cleanup in the destructor	-->
	<xsl:template match="Argument" mode="destructor">
		<xsl:choose>
			<xsl:when test="@type = 'PARAMETERLIST'">
				<xsl:text>&#x9;&#x9;// plist gets destroyed by the base class.&#xa;</xsl:text>
			</xsl:when>
			<xsl:when test="contains( @type, 'Array')">
				<xsl:choose>
					<xsl:when test="@type = 'RtTokenArray' or @type = 'RtStringArray'">
						<xsl:value-of select="concat('&#x9;&#x9;int __', @name, '_length = 1;&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;int __', @name, '_index;&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;for(__', @name, '_index = 0; __', @name, '_index&lt;__', @name, '_length; __', @name, '_index++)&#xa;')"/>
						<xsl:text>&#x9;&#x9;{&#xa;</xsl:text>
						<xsl:value-of select="concat('&#x9;&#x9;&#x9;delete[](m_', @name, '[__', @name, '_index]);&#xa;')"/>
						<xsl:text>&#x9;&#x9;}&#xa;</xsl:text>
					</xsl:when>
				</xsl:choose>
				<xsl:value-of select="concat('&#x9;&#x9;delete[](m_', @name, ');&#xa;')"/>
			</xsl:when>
			<xsl:otherwise>
				<xsl:choose>
					<xsl:when test="@type = 'RtToken' or @type = 'RtString'">
						<xsl:value-of select="concat('&#x9;&#x9;delete[](m_', @name, ');&#xa;')"/>
					</xsl:when>
				</xsl:choose>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>


			<!--	Arguments to macro construct method	-->
	<xsl:template match="Argument" mode="macro_call">
		<xsl:choose>
			<xsl:when test="@type = 'PARAMETERLIST'">
				<xsl:text>count, tokens, values</xsl:text>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select="@name"/>
			</xsl:otherwise>
		</xsl:choose>
		<xsl:if test="last() != position()">
		   <xsl:text>, </xsl:text>
		</xsl:if>
	</xsl:template>

</xsl:stylesheet>
