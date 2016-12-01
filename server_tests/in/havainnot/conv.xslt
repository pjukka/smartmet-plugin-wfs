<xsl:stylesheet version="1.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:wfs="http://www.opengis.net/wfs/2.0"
                xmlns:gml="http://www.opengis.net/gml/3.2"
                xmlns:omso="http://inspire.ec.europa.eu/schemas/omso/2.9"
                xmlns:om="http://www.opengis.net/om/2.0">

    <xsl:output omit-xml-declaration="yes" indent="yes"/>

    <xsl:param name="newTimeStamp" select="'2013-01-01T00:00:00Z'"/>

    <xsl:template match="node()|@*">
        <xsl:copy>
            <xsl:apply-templates select="node()|@*"/>
        </xsl:copy>
    </xsl:template>

    <xsl:template match="om:resultTime/gml:TimeInstant/gml:timePosition/text()">
        <xsl:value-of select="$newTimeStamp"/>
    </xsl:template>

    <xsl:template match="wfs:FeatureCollection/@timeStamp">
        <xsl:attribute name="timeStamp">
            <xsl:value-of select="$newTimeStamp"/>
        </xsl:attribute>
    </xsl:template>
</xsl:stylesheet>
