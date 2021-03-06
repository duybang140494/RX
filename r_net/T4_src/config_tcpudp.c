//=====================================================================//
/*!	@file
	@brief	TCP/UDP config @n
			Copyright 2017 Kunihito Hiramatsu
	@author	平松邦仁 (hira@rvf-rc45.net)
*/
//=====================================================================//
#include "config_tcpudp.h"

/****************************************************************************/
/**********************     TCP-related definition     **********************/
/****************************************************************************/
/* Number of LAN port, Number of Serial port */
const uint8_t t4_channel_num = 1;

/****************************************************************************/
/**********************     TCP-related definition     **********************/
/****************************************************************************/

/***  Definition of TCP reception point (only port number needs to be set) ***/
T_TCP_CREP tcp_crep[TCP_CREP_MAX] = {
	{ 0x0000, { 0, 80 } },
	{ 0x0000, { 0, 80 } },
	{ 0x0000, { 0, 80 } },
	{ 0x0000, { 0, 80 } },
	{ 0x0000, { 0, 80 } },
	{ 0x0000, { 0, 80 } },
	{ 0x0000, { 0, 80 } },
	{ 0x0000, { 0, 80 } },
};

/* Total number of TCP reception points */
const uint32_t tcp_crep_num = sizeof(tcp_crep) / sizeof(T_TCP_CREP);

/***  Definition of TCP communication end point
      (only receive window size needs to be set) ***/
T_TCP_CCEP tcp_ccep[TCP_CCEP_MAX] = {
    { 0, 0, 0, 0, 1460, 0 },
    { 0, 0, 0, 0, 1460, 0 },
    { 0, 0, 0, 0, 1460, 0 },
    { 0, 0, 0, 0, 1460, 0 },
    { 0, 0, 0, 0, 1460, 0 },
    { 0, 0, 0, 0, 1460, 0 },
    { 0, 0, 0, 0, 1460, 0 },
    { 0, 0, 0, 0, 1460, 0 },
};

/* Total number of TCP communication end points */
const uint32_t tcp_ccep_num = sizeof(tcp_ccep) / sizeof(T_TCP_CCEP);

/***  TCP MSS  ***/
const uint16_t tcp_mss[] = {
	1460, 1460
};    /* MAX:1460 bytes */

/***  Initial value of sequence number (Set the value other than 0)  ***/
const uint32_t tcp_initial_seqno[] = {
	1, 1,
};

/***  2MSL wait time (unit:10ms)  ***/
const uint16_t tcp_2msl[] = {
	1, 1,
};      /* 10 ms */

/***  Maximum value of retransmission timeout period (unit:10ms)  ***/
const uint32_t tcp_rt_tmo_rst[] = {
	(10 * 60 * (1000 / 10)),     /* 10 min */
	(10 * 60 * (1000 / 10)),
};

/***  Transmit for delay ack (ON=1/OFF=0) ***/
const uint8_t tcp_dack[] = {
	1, 1
};

/****************************************************************************/
/**********************     UDP-related definition     **********************/
/****************************************************************************/
/***  Definition of UDP communication end point  ***/
_UDP_CB  *_udp_cb;

T_UDP_CCEP udp_ccep[UDP_CCEP_MAX] = {
    { 0x0000, { 0, 1365 }, t4_udp_callback },
    { 0x0000, { 0, 1365 }, t4_udp_callback },
    { 0x0000, { 0, 1365 }, t4_udp_callback },
    { 0x0000, { 0, 1365 }, t4_udp_callback },
};

/* Total number of UDP communication end points */
const uint32_t udp_ccep_num = (sizeof(udp_ccep) / sizeof(T_UDP_CCEP));

/***  TTL for multicast transmission  ***/
const uint8_t multi_TTL[] = { 1, 1 };

/*** Behavior of UDP zero checksum ***/
/* 0 = disable, other = enable */
const uint8_t udp_enable_zerochecksum[] = {	0, 0 };

/// ARP table count
const uint16_t arp_tbl_count_[] = { 3, 3 };

#define MY_IP_ADDR     192,168,0,3            /* Local IP address  */
#define GATEWAY_ADDR   0,0,0,0                /* Gateway address (invalid if all 0s) */
#define SUBNET_MASK    255,255,255,0          /* Subnet mask  */

TCPUDP_ENV tcpudp_env[] = {
{	{ MY_IP_ADDR },
	{ SUBNET_MASK },
	{ GATEWAY_ADDR }  }
};


/****************************************************************************/
/**********************     Driver-related definition     *******************/
/****************************************************************************/
/*--------------------------------------------------------------------------*/
/*    Set of Ethernet-related                                               */
/*--------------------------------------------------------------------------*/
/* Local MAC address (Set all 0s when unspecified) */
MAC_ADDRESS ethernet_mac[] = {
	{ { 0x02,0x00,0x00,0x00,0x00,0x00 } }
};

/*--------------------------------------------------------------------------*/
/*    Set of PPP-related                                                    */
/*--------------------------------------------------------------------------*/
/* Authentication-related setting */
const uint16_t ppp_auth  = AUTH_PAP;              /* PAP,No authentication enabled */
uint8_t user_name[6]    = "abcde";                /* user name */
uint8_t user_passwd[6]  = "abc00";                /* password  */

/* Dial up-related setting */
const uint8_t peer_dial[]      = "0,123";                /* Destination telephone number */
const uint8_t at_commands[]   = "ATW2S0=2&C0&D0&S0M0S10=255X3";    /* Modem initialization command */


/* Copyright 2014, RENESAS ELECTRONICS CORPORATION */
