#!/usr/bin/env python
import os

def main():
    os.system('grep -i \'Position after update\' scenario.log | awk \'{print $1\",\"$8}\' | uniq > pos.csv')

    fcsv = open('pos.csv', 'r')
    fkml = open('pos.kml', 'w')

    fkml.write("""
    <kml>
        <Document>
            <name>Untitled layer</name>
            <Style id="icon-1899-0288D1-nodesc-normal">
                <IconStyle>
                    <color>ffd18802</color>
                    <scale>1</scale>
                    <Icon>
                        <href>
                        https://www.gstatic.com/mapspro/images/stock/503-wht-blank_maps.png
                        </href>
                    </Icon>
                    <hotSpot x="32" xunits="pixels" y="64" yunits="insetPixels"/>
                </IconStyle>
                <LabelStyle>
                    <scale>0</scale>
                </LabelStyle>
                <BalloonStyle>
                    <text><h3>$[name]</h3></text>
                </BalloonStyle>
            </Style>
            <Style id="icon-1899-0288D1-nodesc-highlight">
                <IconStyle>
                    <color>ffd18802</color>
                    <scale>1</scale>
                    <Icon>
                        <href>
                        https://www.gstatic.com/mapspro/images/stock/503-wht-blank_maps.png
                        </href>
                    </Icon>
                    <hotSpot x="32" xunits="pixels" y="64" yunits="insetPixels"/>
                </IconStyle>
                <LabelStyle>
                    <scale>1</scale>
                </LabelStyle>
                <BalloonStyle>
                    <text><h3>$[name]</h3></text>
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
    """)

    """
    <Placemark>
    <name>Takeoff/Landing</name>
    <styleUrl>#icon-1899-E65100-nodesc</styleUrl>
    <Point>
        <coordinates>
        17.4031484,40.5157964,0
        </coordinates>
    </Point>
    </Placemark>
    <Placemark>
    <name>Center</name>
    <styleUrl>#icon-1899-4E342E-nodesc</styleUrl>
    <Point>
        <coordinates>
        10.8265158,46.1872092,0
        </coordinates>
    </Point>
    </Placemark>
    <Placemark>
    <name>South</name>
    <styleUrl>#icon-1899-0288D1-nodesc</styleUrl>
    <Point>
        <coordinates>
        10.9203494,38.5344019,0
        </coordinates>
    </Point>
    </Placemark>
    <Placemark>
    <name>North</name>
    <styleUrl>#icon-1899-0288D1-nodesc</styleUrl>
    <Point>
        <coordinates>
        10.6959556,53.5541228,0
        </coordinates>
    </Point>
    </Placemark>
    <Placemark>
    <name>East</name>
    <styleUrl>#icon-1899-0288D1-nodesc</styleUrl>
    <Point>
        <coordinates>
        21.4891396,46.1578555,0
        </coordinates>
    </Point>
    </Placemark>
    <Placemark>
    <name>West</name>
    <styleUrl>#icon-1899-0288D1-nodesc</styleUrl>
    <Point>
        <coordinates>
        0.1111296,46.1404245,0
        </coordinates>
    </Point>
    </Placemark>
    <Placemark>
    <name>Satellite</name>
    <styleUrl>#icon-1899-3949AB-nodesc</styleUrl>
    <Point>
        <coordinates>
        -6.1223182,3.2288364,0
        </coordinates>
    </Point>
    </Placemark>
    """

    while line := fcsv.readline():
        line = line.split(',')
        pos = line[1].split(':')
        time = line[0]
        lat = pos[0]
        lon = pos[1]

        fkml.write(f"""
            <Placemark>
                <name>{time}</name>
                <styleUrl>#icon-1899-0288D1-nodesc</styleUrl>
                <Point>
                    <coordinates>{lon},{lat},0</coordinates>
                </Point>
            </Placemark>
        """)

    fkml.write("""
        </Document>
    </kml>
    """)

    fcsv.close()
    fkml.close()

if __name__ == '__main__':
    main()