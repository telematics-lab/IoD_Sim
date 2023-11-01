#!/usr/bin/env python
import os

def main():
    os.system('grep -i \'Geographic position\' scenario.log | awk \'{print $1\",\"$7}\' | uniq > pos.csv')

    fcsv = open('pos.csv', 'r')
    fkml = open('pos.kml', 'w')

    fkml.write("""
    <?xml version="1.0" encoding="UTF-8"?>
    <kml xmlns="http://www.opengis.net/kml/2.2">
    <Document>
        <name>Untitled layer</name>
        <StyleMap id="icon-1502-1A237E-nodesc">
        <Pair>
            <key>normal</key>
            <styleUrl>#icon-1502-1A237E-nodesc-normal</styleUrl>
        </Pair>
        <Pair>
            <key>highlight</key>
            <styleUrl>#icon-1502-1A237E-nodesc-highlight</styleUrl>
        </Pair>
        </StyleMap>
        <Style id="poly-E65100-6401-0-nodesc-normal">
        <LineStyle>
            <color>ff0051e6</color>
            <width>6.401</width>
        </LineStyle>
        <PolyStyle>
            <color>000051e6</color>
            <fill>1</fill>
            <outline>1</outline>
        </PolyStyle>
        <BalloonStyle>
            <text><![CDATA[<h3>$[name]</h3>]]></text>
        </BalloonStyle>
        </Style>
        <Style id="poly-E65100-6401-0-nodesc-highlight">
        <LineStyle>
            <color>ff0051e6</color>
            <width>9.6015</width>
        </LineStyle>
        <PolyStyle>
            <color>000051e6</color>
            <fill>1</fill>
            <outline>1</outline>
        </PolyStyle>
        <BalloonStyle>
            <text><![CDATA[<h3>$[name]</h3>]]></text>
        </BalloonStyle>
        </Style>
        <StyleMap id="poly-E65100-6401-0-nodesc">
        <Pair>
            <key>normal</key>
            <styleUrl>#poly-E65100-6401-0-nodesc-normal</styleUrl>
        </Pair>
        <Pair>
            <key>highlight</key>
            <styleUrl>#poly-E65100-6401-0-nodesc-highlight</styleUrl>
        </Pair>
        </StyleMap>
        <StyleMap id="icon-1502-1A237E-nodesc">
        <Pair>
            <key>normal</key>
            <styleUrl>#icon-1502-1A237E-nodesc-normal</styleUrl>
        </Pair>
        <Pair>
            <key>highlight</key>
            <styleUrl>#icon-1502-1A237E-nodesc-highlight</styleUrl>
        </Pair>
        </StyleMap>
        <Style id="icon-1899-0288D1-nodesc-normal">
            <IconStyle>
                <color>ffd18802</color>
                <scale>1</scale>
                <Icon>
                <href>https://www.gstatic.com/mapspro/images/stock/503-wht-blank_maps.png</href>
                </Icon>
                <hotSpot x="32" xunits="pixels" y="64" yunits="insetPixels"/>
            </IconStyle>
            <LabelStyle>
                <scale>0</scale>
            </LabelStyle>
            <BalloonStyle>
                <text><![CDATA[<h3>$[name]</h3>]]></text>
            </BalloonStyle>
            </Style>
            <Style id="icon-1899-0288D1-nodesc-highlight">
            <IconStyle>
                <color>ffd18802</color>
                <scale>1</scale>
                <Icon>
                <href>https://www.gstatic.com/mapspro/images/stock/503-wht-blank_maps.png</href>
                </Icon>
                <hotSpot x="32" xunits="pixels" y="64" yunits="insetPixels"/>
            </IconStyle>
            <LabelStyle>
                <scale>1</scale>
            </LabelStyle>
            <BalloonStyle>
                <text><![CDATA[<h3>$[name]</h3>]]></text>
            </BalloonStyle>
            </Style>
            <StyleMap id="icon-1899-0288D1-nodesc">
            <Pair>
                <key>normal</key>
                <styleUrl>#icon-1899-0288D1-nodesc-normal</styleUrl>
            </Pair>
            <Pair>
                <key>highlight</key>
                <styleUrl>#icon-1899-0288D1-nodesc-highlight</styleUrl>
            </Pair>
            </StyleMap>
            <Style id="icon-1899-3949AB-nodesc-normal">
            <IconStyle>
                <color>ffab4939</color>
                <scale>1</scale>
                <Icon>
                <href>https://www.gstatic.com/mapspro/images/stock/503-wht-blank_maps.png</href>
                </Icon>
                <hotSpot x="32" xunits="pixels" y="64" yunits="insetPixels"/>
            </IconStyle>
            <LabelStyle>
                <scale>0</scale>
            </LabelStyle>
            <BalloonStyle>
                <text><![CDATA[<h3>$[name]</h3>]]></text>
            </BalloonStyle>
            </Style>
            <Style id="icon-1899-3949AB-nodesc-highlight">
            <IconStyle>
                <color>ffab4939</color>
                <scale>1</scale>
                <Icon>
                <href>https://www.gstatic.com/mapspro/images/stock/503-wht-blank_maps.png</href>
                </Icon>
                <hotSpot x="32" xunits="pixels" y="64" yunits="insetPixels"/>
            </IconStyle>
            <LabelStyle>
                <scale>1</scale>
            </LabelStyle>
            <BalloonStyle>
                <text><![CDATA[<h3>$[name]</h3>]]></text>
            </BalloonStyle>
            </Style>
            <StyleMap id="icon-1899-3949AB-nodesc">
            <Pair>
                <key>normal</key>
                <styleUrl>#icon-1899-3949AB-nodesc-normal</styleUrl>
            </Pair>
            <Pair>
                <key>highlight</key>
                <styleUrl>#icon-1899-3949AB-nodesc-highlight</styleUrl>
            </Pair>
            </StyleMap>
            <Style id="icon-1899-4E342E-nodesc-normal">
            <IconStyle>
                <color>ff2e344e</color>
                <scale>1</scale>
                <Icon>
                <href>https://www.gstatic.com/mapspro/images/stock/503-wht-blank_maps.png</href>
                </Icon>
                <hotSpot x="32" xunits="pixels" y="64" yunits="insetPixels"/>
            </IconStyle>
            <LabelStyle>
                <scale>0</scale>
            </LabelStyle>
            <BalloonStyle>
                <text><![CDATA[<h3>$[name]</h3>]]></text>
            </BalloonStyle>
            </Style>
            <Style id="icon-1899-4E342E-nodesc-highlight">
            <IconStyle>
                <color>ff2e344e</color>
                <scale>1</scale>
                <Icon>
                <href>https://www.gstatic.com/mapspro/images/stock/503-wht-blank_maps.png</href>
                </Icon>
                <hotSpot x="32" xunits="pixels" y="64" yunits="insetPixels"/>
            </IconStyle>
            <LabelStyle>
                <scale>1</scale>
            </LabelStyle>
            <BalloonStyle>
                <text><![CDATA[<h3>$[name]</h3>]]></text>
            </BalloonStyle>
            </Style>
            <StyleMap id="icon-1899-4E342E-nodesc">
            <Pair>
                <key>normal</key>
                <styleUrl>#icon-1899-4E342E-nodesc-normal</styleUrl>
            </Pair>
            <Pair>
                <key>highlight</key>
                <styleUrl>#icon-1899-4E342E-nodesc-highlight</styleUrl>
            </Pair>
            </StyleMap>
            <Style id="icon-1899-E65100-nodesc-normal">
            <IconStyle>
                <color>ff0051e6</color>
                <scale>1</scale>
                <Icon>
                <href>https://www.gstatic.com/mapspro/images/stock/503-wht-blank_maps.png</href>
                </Icon>
                <hotSpot x="32" xunits="pixels" y="64" yunits="insetPixels"/>
            </IconStyle>
            <LabelStyle>
                <scale>0</scale>
            </LabelStyle>
            <BalloonStyle>
                <text><![CDATA[<h3>$[name]</h3>]]></text>
            </BalloonStyle>
            </Style>
            <Style id="icon-1899-E65100-nodesc-highlight">
            <IconStyle>
                <color>ff0051e6</color>
                <scale>1</scale>
                <Icon>
                <href>https://www.gstatic.com/mapspro/images/stock/503-wht-blank_maps.png</href>
                </Icon>
                <hotSpot x="32" xunits="pixels" y="64" yunits="insetPixels"/>
            </IconStyle>
            <LabelStyle>
                <scale>1</scale>
            </LabelStyle>
            <BalloonStyle>
                <text><![CDATA[<h3>$[name]</h3>]]></text>
            </BalloonStyle>
            </Style>
            <StyleMap id="icon-1899-E65100-nodesc">
            <Pair>
                <key>normal</key>
                <styleUrl>#icon-1899-E65100-nodesc-normal</styleUrl>
            </Pair>
            <Pair>
                <key>highlight</key>
                <styleUrl>#icon-1899-E65100-nodesc-highlight</styleUrl>
            </Pair>
            </StyleMap>
            <Placemark>
            <name>Takeoff/Landing</name>
            <styleUrl>#icon-1899-E65100-nodesc</styleUrl>
            <Point>
                <coordinates>
                15.4843571,78.244789,0
                </coordinates>
            </Point>
            </Placemark>
            <Placemark>
            <name>GEO Satellite</name>
            <styleUrl>#icon-1502-1A237E-nodesc</styleUrl>
            <Point>
                <coordinates>
                -4.95,0.04,0
                </coordinates>
            </Point>
            </Placemark>
            <Placemark>
            <name>Tehran</name>
            <styleUrl>#icon-1899-0288D1-nodesc</styleUrl>
            <Point>
                <coordinates>
                51.1498211,35.7074505,0
                </coordinates>
            </Point>
            </Placemark>
            <Placemark>
            <name>Reykjavik</name>
            <styleUrl>#icon-1899-0288D1-nodesc</styleUrl>
            <Point>
                <coordinates>
                -21.9348416,64.133542,0
                </coordinates>
            </Point>
            </Placemark>
        <Placemark>
        <name>Trajectory</name>
        <styleUrl>#poly-E65100-6401-0-nodesc</styleUrl>
        <Polygon>
            <outerBoundaryIs>
            <LinearRing>
                <tessellate>1</tessellate>
                <coordinates>
    """)

    while line := fcsv.readline():
        line = line.split(',')
        pos = line[1].split(':')
        time = line[0]
        lat = pos[0]
        lon = pos[1]

        fkml.write(f'{lon},{lat},0 \n')

    fkml.write("""
            </coordinates>
            </LinearRing>
            </outerBoundaryIs>
        </Polygon>
        </Placemark>
    </Document>
    </kml>
    """)

    fcsv.close()
    fkml.close()

if __name__ == '__main__':
    main()