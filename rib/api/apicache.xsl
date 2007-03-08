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
			<xsl:value-of select="concat('class ', Name, 'Cache : public RiCacheBase&#xa;')"/>
			<xsl:text>{&#xa;</xsl:text>
			<xsl:text>public:&#xa;</xsl:text>
			<xsl:call-template name="Constructor"/>
			<xsl:call-template name="Destructor"/>
			<xsl:value-of select="concat('&#x9;virtual void ReCall()&#xa;&#x9;{&#xa;&#x9;&#x9;', Name)"/>
			<xsl:if test="Arguments/Argument[last()]/Type = 'PARAMETERLIST'">
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
			<xsl:value-of select="concat('#define CACHE_', translate(Name,'abcdefghijklmnopqrstuvwxyz', 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'), '\&#xa;')"/>
			<xsl:value-of select="string('&#x9;if( QGetRenderContext()->pCurrentObject()) \&#xa;')"/>
			<xsl:text>	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					</xsl:text>
			<xsl:value-of select="concat('new ', Name, 'Cache(')"/>
			<xsl:apply-templates select="Arguments/Argument" mode="macro_call"/>
			<xsl:value-of select="string(') ); \&#xa;')"/>
			<xsl:text>		return</xsl:text>
			<xsl:if test="ReturnType != 'RtVoid'">
				<xsl:text>(0)</xsl:text>
			</xsl:if>
			<xsl:text>;	\
	}
	</xsl:text>
		</xsl:if>
	</xsl:template>

	<xsl:template name="Constructor">
		<xsl:value-of select="concat('&#x9;', Name, 'Cache(')"/>
		<xsl:apply-templates select="Arguments/Argument" mode="constructor_sig"/>
		<xsl:text>) : RiCacheBase()&#xa;&#x9;{&#xa;</xsl:text>
		<xsl:apply-templates select="Arguments/Argument" mode="constructor_copy"/>
		<xsl:text>&#x9;}&#xa;</xsl:text>
	</xsl:template>

	<xsl:template name="Destructor">
		<xsl:value-of select="concat('&#x9;virtual ~', Name, 'Cache()&#xa;&#x9;{&#xa;')"/>
		<xsl:apply-templates select="Arguments/Argument" mode="destructor"/>
		<xsl:text>&#x9;}&#xa;</xsl:text>
	</xsl:template>

	<!--	Argument to the cache constructor	-->
	<xsl:template match="Argument" mode="constructor_sig">
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

	<!--	Argument copy within the constructor	-->
	<xsl:template match="Argument" mode="constructor_copy">
		<xsl:choose>
			<xsl:when test="Type = 'PARAMETERLIST'">
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
			<xsl:when test="contains( Type, 'Array')">
				<xsl:choose>
					<xsl:when test="./Length">
						<xsl:value-of select="./Length"/>
					</xsl:when>
					<xsl:otherwise>
						<xsl:text>&#x9;&#x9;// \note: Need to specify the length method here!&#xa;</xsl:text>
						<xsl:value-of select="concat('&#x9;&#x9;int __', Name, '_length = 1;&#xa;')"/>
					</xsl:otherwise>
				</xsl:choose>
				<xsl:value-of select="concat('&#x9;&#x9;m_', Name, ' = new ', substring-before(Type, 'Array'), '[__', Name, '_length];&#xa;')"/>
				<xsl:value-of select="concat('&#x9;&#x9;int __', Name, '_index;&#xa;')"/>
				<xsl:value-of select="concat('&#x9;&#x9;for(__', Name, '_index = 0; __', Name, '_index&lt;__', Name, '_length; __', Name, '_index++)&#xa;')"/>
				<xsl:text>&#x9;&#x9;{&#xa;</xsl:text>
				<xsl:choose>
					<xsl:when test="Type = 'RtTokenArray' or Type = 'RtStringArray'">
						<xsl:value-of select="concat('&#x9;&#x9;&#x9;int __', Name, '_slength = strlen(', Name, '[__', Name, '_index]);&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;&#x9;m_', Name, '[__', Name, '_index] = new char[ __', Name, '_slength + 1 ];&#xa;')"/> 
						<xsl:value-of select="concat('&#x9;&#x9;&#x9;strcpy(m_', Name, '[__', Name, '_index], ', Name, '[__', Name, '_index]);&#xa;')"/>
					</xsl:when>
					<xsl:when test="Type = 'RtColorArray' or Type = 'RtPointArray'">
						<xsl:value-of select="concat('&#x9;&#x9;&#x9;m_', Name, '[__', Name, '_index][0] = ', Name, '[__', Name, '_index][0];&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;&#x9;m_', Name, '[__', Name, '_index][1] = ', Name, '[__', Name, '_index][1];&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;&#x9;m_', Name, '[__', Name, '_index][2] = ', Name, '[__', Name, '_index][2];&#xa;')"/>
					</xsl:when>
					<xsl:otherwise>
						<xsl:value-of select="concat('&#x9;&#x9;&#x9;m_', Name, '[__', Name, '_index] = ', Name, '[__', Name, '_index];&#xa;')"/>
					</xsl:otherwise>
				</xsl:choose>
				<xsl:text>&#x9;&#x9;}&#xa;</xsl:text>
			</xsl:when>
			<xsl:otherwise>
				<xsl:choose>
					<xsl:when test="Type = 'RtToken' or Type = 'RtString'">
						<xsl:value-of select="concat('&#x9;&#x9;int __', Name, '_length = strlen(', Name, ');&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;m_', Name, ' = new char[ __', Name, '_length + 1 ];&#xa;')"/> 
						<xsl:value-of select="concat('&#x9;&#x9;strcpy(m_', Name, ', ', Name, ');&#xa;')"/>
					</xsl:when>
					<xsl:when test="Type = 'RtColor' or Type = 'RtPoint'">
						<xsl:value-of select="concat('&#x9;&#x9;m_', Name, '[0] = ', Name, '[0];&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;m_', Name, '[1] = ', Name, '[1];&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;m_', Name, '[2] = ', Name, '[2];&#xa;')"/>
					</xsl:when>
					<xsl:when test="Type = 'RtMatrix' or Type = 'RtBasis'">
						<xsl:value-of select="concat('&#x9;&#x9;int __', Name, '_i, __', Name, '_j;&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;for(__', Name, '_j = 0; __', Name, '_j&lt;4; __', Name, '_j++)&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;&#x9;for(__', Name, '_i = 0; __', Name, '_i&lt;4; __', Name, '_i++)&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;&#x9;&#x9;m_', Name, '[__', Name, '_j][__', Name, '_i] = ', Name, '[__', Name, '_j][__', Name, '_i];&#xa;')"/>
					</xsl:when>
					<xsl:when test="Type = 'RtBound'">
						<xsl:value-of select="concat('&#x9;&#x9;m_', Name, '[0] = ', Name, '[0];&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;m_', Name, '[1] = ', Name, '[1];&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;m_', Name, '[2] = ', Name, '[2];&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;m_', Name, '[3] = ', Name, '[3];&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;m_', Name, '[4] = ', Name, '[4];&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;m_', Name, '[5] = ', Name, '[5];&#xa;')"/>
					</xsl:when>
					<xsl:otherwise>
						<xsl:value-of select="concat('&#x9;&#x9;m_', Name, ' = ', Name, ';&#xa;')"/>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<!--	Argument storage members	-->
	<xsl:template match="Argument" mode="member_vars">
		<xsl:text>&#x9;</xsl:text>
		<xsl:choose>
			<xsl:when test="Type = 'PARAMETERLIST'">
				<xsl:text>// plist information is stored in the base class.</xsl:text>
			</xsl:when>
			<xsl:when test="contains( Type, 'Array')">
				<xsl:value-of select="concat(substring-before(Type, 'Array'), '* m_', Name, ';')"/>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select="concat(Type, ' m_', Name, ';')"/>
			</xsl:otherwise>
		</xsl:choose>
		<xsl:text>&#xa;</xsl:text>
	</xsl:template>

	<!--	Arguments to recall method	-->
	<xsl:template match="Argument" mode="recall_args">
		<xsl:choose>
			<xsl:when test="Type = 'PARAMETERLIST'">
				<xsl:text>m_count, m_tokens, m_values</xsl:text>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select="concat('m_', Name)"/>
			</xsl:otherwise>
		</xsl:choose>
		<xsl:if test="last() != position()">
		   <xsl:text>, </xsl:text>
		</xsl:if>
	</xsl:template>

	<!--	Argument cleanup in the destructor	-->
	<xsl:template match="Argument" mode="destructor">
		<xsl:choose>
			<xsl:when test="Type = 'PARAMETERLIST'">
				<xsl:text>&#x9;&#x9;// plist gets destroyed by the base class.&#xa;</xsl:text>
			</xsl:when>
			<xsl:when test="contains( Type, 'Array')">
				<xsl:choose>
					<xsl:when test="Type = 'RtTokenArray' or Type = 'RtStringArray'">
						<xsl:value-of select="concat('&#x9;&#x9;int __', Name, '_length = 1;&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;int __', Name, '_index;&#xa;')"/>
						<xsl:value-of select="concat('&#x9;&#x9;for(__', Name, '_index = 0; __', Name, '_index&lt;__', Name, '_length; __', Name, '_index++)&#xa;')"/>
						<xsl:text>&#x9;&#x9;{&#xa;</xsl:text>
						<xsl:value-of select="concat('&#x9;&#x9;&#x9;delete[](m_', Name, '[__', Name, '_index]);&#xa;')"/>
						<xsl:text>&#x9;&#x9;}&#xa;</xsl:text>
					</xsl:when>
				</xsl:choose>
				<xsl:value-of select="concat('&#x9;&#x9;delete[](m_', Name, ');&#xa;')"/>
			</xsl:when>
			<xsl:otherwise>
				<xsl:choose>
					<xsl:when test="Type = 'RtToken' or Type = 'RtString'">
						<xsl:value-of select="concat('&#x9;&#x9;delete[](m_', Name, ');&#xa;')"/>
					</xsl:when>
				</xsl:choose>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>


			<!--	Arguments to macro construct method	-->
	<xsl:template match="Argument" mode="macro_call">
		<xsl:choose>
			<xsl:when test="Type = 'PARAMETERLIST'">
				<xsl:text>count, tokens, values</xsl:text>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select="Name"/>
			</xsl:otherwise>
		</xsl:choose>
		<xsl:if test="last() != position()">
		   <xsl:text>, </xsl:text>
		</xsl:if>
	</xsl:template>

</xsl:stylesheet>
