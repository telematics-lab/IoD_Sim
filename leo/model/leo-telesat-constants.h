/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Tim Schubert <ns-3-leo@timschubert.net>
 */

#ifndef LEO_TELESAT_CONSTANTS
#define LEO_TELESAT_CONSTANTS

/**
 * \file
 * \ingroup leo
 * Definition of Telesat link constants
 */

namespace ns3 {

/**
 * \ingroup leo
 * \brief Constants for Telesat network estimated from channel parameters
 *
 * Source http://systemarchitect.mit.edu/docs/delportillo18b.pdf
 *
 */

// gateway uplink
#define LEO_TELESAT_GATEWAY_FREQUENCY        28.5         // GHz
#define LEO_TELESAT_GATEWAY_BANDWIDTH        2.1          // GHz
#define LEO_TELESAT_GATEWAY_TX_ANTENNA_D     3.5          // m
#define LEO_TELESAT_GATEWAY_EIRP             105.9        // dBm
#define LEO_TELESAT_GATEWAY_MODCOD           "64APSK 3/4" // -
#define LEO_TELESAT_GATEWAY_ROLL_OFF_FACTOR  0.1          // -
#define LEO_TELESAT_GATEWAY_SPECTRAL_EFF     4.1          // bps/Hz
#define LEO_TELESAT_GATEWAY_PATH_DISTANCE    2439         // km
#define LEO_TELESAT_GATEWAY_ELEVATION_ANGLE  20           // deg
#define LEO_TELESAT_GATEWAY_FSPL             189.3        // dB
#define LEO_TELESAT_GATEWAY_ATMOSPHERIC_LOSS 4.8          // dB
#define LEO_TELESAT_GATEWAY_RX_ANTENNA_GAIN  31.8         // dBi
#define LEO_TELESAT_GATEWAY_SYSTEM_TEMP      868.4        // K
#define LEO_TELESAT_GATEWAY_G_T              2.4          // dB/K
#define LEO_TELESAT_GATEWAY_RX_C_N0          25.6         // dB
#define LEO_TELESAT_GATEWAY_RX_C_ACI         27           // dB
#define LEO_TELESAT_GATEWAY_RX_C_ASI         23.5         // dB
#define LEO_TELESAT_GATEWAY_RX_C_XPI         25           // dB
#define LEO_TELESAT_GATEWAY_HPA_C_3IM        25           // dB
#define LEO_TELESAT_GATEWAY_RX_EB_N0_I0      11.4         // dB
#define LEO_TELESAT_GATEWAY_REQ_EB_N0        11.0         // dB
#define LEO_TELESAT_GATEWAY_LINK_MARGIN      0.36         // dB
#define LEO_TELESAT_GATEWAY_DATA_RATE        "9857.1Mbps"       // Mbps
#define LEO_TELESAT_GATEWAY_SHANNON_LIMIT    1.09         // dB

// user uplink
#define LEO_TELESAT_USER_FREQUENCY            13.5        // GHz
#define LEO_TELESAT_USER_BANDWIDTH            0.25        // GHz
#define LEO_TELESAT_USER_EIRP                 64.6        // dBm
#define LEO_TELESAT_USER_MODCOD               "16APSK2/3" // -
#define LEO_TELESAT_USER_ROLL_OFF_FACTOR      0.1         // -
#define LEO_TELESAT_USER_SPECTRAL_EFF         2.4         // bps/Hz
#define LEO_TELESAT_USER_PATH_DISTANCE        1504        // km
#define LEO_TELESAT_USER_ELEVATION_ANGLE      55          // deg
#define LEO_TELESAT_USER_FSPL                 178.6       // dB
#define LEO_TELESAT_USER_ATMOSPHERIC_LOSS     0.41        // dB
#define LEO_TELESAT_GATEWAY_RX_ANTENNA_D      0.75        // m
#define LEO_TELESAT_USER_RX_ANTENNA_GAIN      38.3        // dBi
#define LEO_TELESAT_USER_SYSTEM_TEMP          350.1       // K
#define LEO_TELESAT_USER_RX_C_N0              10.5        // dB
#define LEO_TELESAT_USER_RX_C_ASI             25          // dB
#define LEO_TELESAT_USER_RX_C_XPI             20          // dB
#define LEO_TELESAT_USER_HPA_C_3IM            30          // dB
#define LEO_TELESAT_USER_RX_EB_N0_I0          5.9         // dB
#define LEO_TELESAT_USER_REQ_EB_N0            5.2         // dB
#define LEO_TELESAT_USER_LINK_MARGIN          0.76        // dB
#define LEO_TELESAT_USER_DATA_RATE            "599.4Mbps"       // Mbps
#define LEO_TELESAT_USER_SHANNON_LIMIT        1.49        // dB

};

#endif
