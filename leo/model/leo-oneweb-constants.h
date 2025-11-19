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

#ifndef LEO_ONEWEB_CONSTANTS
#define LEO_ONEWEB_CONSTANTS

/**
 * \file
 * \ingroup leo
 *
 * Declaration of constants
 */

namespace ns3 {

/**
 * \ingroup leo
 * \brief Constants for oneweb network estimated from channel parameters
 *
 * Source http://systemarchitect.mit.edu/docs/delportillo18b.pdf
 *
 */

// gateway uplink
#define LEO_ONEWEB_GATEWAY_FREQUENCY        28.5            // GHz
#define LEO_ONEWEB_GATEWAY_BANDWIDTH        0.25            // GHz
#define LEO_ONEWEB_GATEWAY_TX_ANTENNA_D     2.4             // m
#define LEO_ONEWEB_GATEWAY_EIRP             93.2            // dBm
#define LEO_ONEWEB_GATEWAY_MODCOD           "256APSK 32/45" // -
#define LEO_ONEWEB_GATEWAY_ROLL_OFF_FACTOR  0.1             // -
#define LEO_ONEWEB_GATEWAY_SPECTRAL_EFF     5.1             // bps/Hz
#define LEO_ONEWEB_GATEWAY_PATH_DISTANCE    1504            // km
#define LEO_ONEWEB_GATEWAY_ELEVATION_ANGLE  55              // deg
#define LEO_ONEWEB_GATEWAY_FSPL             185.1           // dB
#define LEO_ONEWEB_GATEWAY_ATMOSPHERIC_LOSS 2.3             // dB
#define LEO_ONEWEB_GATEWAY_RX_ANTENNA_GAIN  37.8            // dBi
#define LEO_ONEWEB_GATEWAY_SYSTEM_TEMP      447.2           // K
#define LEO_ONEWEB_GATEWAY_G_T              11.3            // dB/K
#define LEO_ONEWEB_GATEWAY_RX_C_N0          32.5            // dB
#define LEO_ONEWEB_GATEWAY_RX_C_ACI         27              // dB
#define LEO_ONEWEB_GATEWAY_RX_C_ASI         27              // dB
#define LEO_ONEWEB_GATEWAY_RX_C_XPI         25              // dB
#define LEO_ONEWEB_GATEWAY_HPA_C_3IM        30              // dB
#define LEO_ONEWEB_GATEWAY_RX_EB_N0_I0      13.3            // dB
#define LEO_ONEWEB_GATEWAY_REQ_EB_N0        12.3            // dB
#define LEO_ONEWEB_GATEWAY_LINK_MARGIN      1.03            // dB
#define LEO_ONEWEB_GATEWAY_DATA_RATE        "1341.1Mbps"          // Mbps
#define LEO_ONEWEB_GATEWAY_SHANNON_LIMIT    1.06            // dB

// user uplink
#define LEO_ONEWEB_USER_FREQUENCY            18.5           // GHz
#define LEO_ONEWEB_USER_BANDWIDTH            0.25           // GHz
#define LEO_ONEWEB_USER_EIRP                 36.0           // dBm
#define LEO_ONEWEB_USER_MODCOD               "16APSK28/45"  // -
#define LEO_ONEWEB_USER_ROLL_OFF_FACTOR      0.1            // -
#define LEO_ONEWEB_USER_SPECTRAL_EFF         2.23           // bps/Hz
#define LEO_ONEWEB_USER_PATH_DISTANCE        2439           // km
#define LEO_ONEWEB_USER_ELEVATION_ANGLE      20             // deg
#define LEO_ONEWEB_USER_FSPL                 185.5          // dB
#define LEO_ONEWEB_USER_ATMOSPHERIC_LOSS     2.0            // dB
#define LEO_ONEWEB_GATEWAY_RX_ANTENNA_D      1              // m
#define LEO_ONEWEB_USER_RX_ANTENNA_GAIN      43.5           // dBi
#define LEO_ONEWEB_USER_SYSTEM_TEMP          285.3          // K
#define LEO_ONEWEB_USER_RX_C_N0              9.6            // dB
#define LEO_ONEWEB_USER_RX_C_ASI             30             // dB
#define LEO_ONEWEB_USER_RX_C_XPI             25             // dB
#define LEO_ONEWEB_USER_HPA_C_3IM            20             // dB
#define LEO_ONEWEB_USER_RX_EB_N0_I0          5.5            // dB
#define LEO_ONEWEB_USER_REQ_EB_N0            4.6            // dB
#define LEO_ONEWEB_USER_LINK_MARGIN          0.85           // dB
#define LEO_ONEWEB_USER_DATA_RATE            "558.7Mbps"          // Mbps
#define LEO_ONEWEB_USER_SHANNON_LIMIT        1.49           // dB

};

#endif
