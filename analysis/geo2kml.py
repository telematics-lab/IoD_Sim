#!/usr/bin/env python
import os
from argparse import ArgumentParser
from pathlib import Path
from typing import Optional


def existing_file(input_string):
    """
    Checks whether the user passed a correct and existing file path.
    """
    p = Path(input_string)
    if p.is_file():
        return p


def parse_args(description=Optional[str]):
    """
    Parse the arguments given by the user in input to the program through the shell.

    Returns a Namespace containing all the arguments.
    """
    p = ArgumentParser(description=description)
    p.add_argument(
        "scenario_log_file", type=existing_file, help="Path to the scenario.log file."
    )
    return p.parse_args()


if __name__ == "__main__":
    args = parse_args(
        description="Parse the coordinates in scenario.log and export it in KML format. The output file is saved in the same directory of the input file."
    )

    os.system(
        "grep -i 'Position after update' {} \
              | awk '{{print $1\",\"$8}}' \
              | uniq > {}/pos.csv \
              ".format(
            args.scenario_log_file, args.scenario_log_file.parent
        )
    )

    with open(f"{args.scenario_log_file.parent}/pos.kml", "w") as fkml:
        fkml.write(
            """
<?xml version="1.0" encoding="UTF-8"?>
<kml xmlns="http://www.opengis.net/kml/2.2">
<Document>
    <name>Trajectory</name>
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
        """
        )

        with open(f"{args.scenario_log_file.parent}/pos.csv", "r") as fcsv:
            while line := fcsv.readline():
                line = line.split(",")
                pos = line[1].split(":")
                time = line[0]
                lat = pos[0]
                lon = pos[1]

                fkml.write(
                    f"""
                    <Placemark>
                        <name>{time}</name>
                        <styleUrl>#icon-1899-0288D1-nodesc</styleUrl>
                        <Point>
                            <coordinates>{lon},{lat},0</coordinates>
                        </Point>
                    </Placemark>
                """
                )

        fkml.write("</Document></kml>")
