<?xml version="1.0" encoding="UTF-8" ?>


<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
	<xsl:output method="text"/>
	<xsl:strip-space elements="RiAPI"/>
	<xsl:include href="api_utils.xsl"/>


	<!--	API	-->
	<xsl:template match="RiAPI">
		<!--	Procedures	-->
		<xsl:apply-templates select="Procedures/Procedure"/>
		<xsl:apply-templates select="Procedures/Procedure" mode="macro"/>
		<xsl:text>&#xa;&#xa;</xsl:text>
	</xsl:template>


<xsl:template match="Procedure">
<xsl:if test="not(./NoCache)">
class <xsl:value-of select="Name"/>Cache : public RiCacheBase
{
public:
	<xsl:value-of select="Name"/>Cache(<xsl:apply-templates select="Arguments" mode="procedure_formals"/>) : RiCacheBase()
	{
<xsl:apply-templates select="Arguments" mode="constructor_copy"/>
	}
	virtual ~<xsl:value-of select="Name"/>Cache()
	{
<xsl:apply-templates select="Arguments/Argument" mode="destructor"/>
	}
	virtual void ReCall()
	{
		<xsl:apply-templates select="." mode="procedure_name"/>(<xsl:apply-templates select="Arguments" mode="procedure_call"><xsl:with-param name="prefix" select="'m_'"/></xsl:apply-templates>);
	}

private:
<xsl:apply-templates name="Argument" mode="member_vars" select="Arguments/Argument"/>
};
</xsl:if>
</xsl:template>


<xsl:template match="Procedure" mode="macro">
<xsl:if test="not(./NoCache)">
#define CACHE_<xsl:value-of select="translate(Name,'abcdefghijklmnopqrstuvwxyz', 'ABCDEFGHIJKLMNOPQRSTUVWXYZ')"/> \
	if( QGetRenderContext()->pCurrentObject()) \
	{ \
			QGetRenderContext()->pCurrentObject()->AddCacheCommand( \
					new <xsl:value-of select="Name"/>Cache(<xsl:apply-templates select="Arguments" mode="procedure_call"/>) ); \
		return<xsl:if test="ReturnType != 'RtVoid'">(0)</xsl:if>; \
	}
</xsl:if>
</xsl:template>


	<!--	Argument copy within the constructor	-->
	<xsl:template match="Arguments" mode="constructor_copy">
		<xsl:apply-templates select="Argument" mode="constructor_copy"/>
		<xsl:if test="ParamList">
			<xsl:text>&#x9;&#x9;// Copy the plist here.&#xa;</xsl:text>
			<xsl:text>		int uniform_size = 1;
		int varying_size = 1;
		int vertex_size = 1;
		int facevarying_size = 1;
		int facevertex_size = 1;
</xsl:text>
			<xsl:if test="../UniformSize">
				<xsl:value-of select="../UniformSize"/>
			</xsl:if>
			<xsl:if test="../VaryingSize">
				<xsl:value-of select="../VaryingSize"/>
			</xsl:if>
			<xsl:if test="../VertexSize">
				<xsl:value-of select="../VertexSize"/>
			</xsl:if>
			<xsl:if test="../FaceVaryingSize">
				<xsl:value-of select="../FaceVaryingSize"/>
			</xsl:if>
			<xsl:if test="../FaceVertexSize">
				<xsl:value-of select="../FaceVertexSize"/>
			</xsl:if>
			<xsl:text>		CachePlist(count, tokens, values, SqInterpClassCounts(uniform_size, varying_size,
			vertex_size, facevarying_size, facevertex_size) );&#xa;</xsl:text>
		</xsl:if>
	</xsl:template>
	<xsl:template match="Argument" mode="constructor_copy">
		<xsl:choose>
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
			<xsl:when test="contains( Type, 'Array')">
				<xsl:value-of select="concat(substring-before(Type, 'Array'), '* m_', Name, ';')"/>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select="concat(Type, ' m_', Name, ';')"/>
			</xsl:otherwise>
		</xsl:choose>
		<xsl:text>&#xa;</xsl:text>
	</xsl:template>

	<!--	Argument cleanup in the destructor	-->
	<xsl:template match="Argument" mode="destructor">
		<xsl:choose>
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


</xsl:stylesheet>
