#pragma once
//=====================================================================//
/*!	@file
	@brief	ip.h @n
			Copyright 2017 Kunihito Hiramatsu
	@author	平松邦仁 (hira@rvf-rc45.net)
*/
//=====================================================================//
#include "type.h"
#include "ether.h"

extern uint8_t *data_link_buf_ptr;

/* protocol type */
#define _IPH_HBH  0  /* Relay point */
#define _IPH_ICMP  1  /* ICMP packets */
#define _IPH_IGMP  2  /* IGMP packets */
#define _IPH_IP   4  /* IP in I */
#define _IPH_TCP  6  /* TCP packets */
#define _IPH_UDP  17  /* UDP packets */
#define _IPH_RH   43  /* Route control header(IPv6) */
#define _IPH_FH   44  /* Fragment header */
#define _IPH_ESP  50  /* Encryption data payload */
#define _IPH_AH   51  /* Authentication header */
#define _IPH_ICMPV6  58  /* ICMPv6 packets */
#define _IPH_NULL  59  /* next header is null */
#define _IPH_ENDOPT  60  /* endpoint */

#define _IPH_VERSION4 4  /* IPv4 (IP version 4) */
#define _IPH_VERSION6 6  /* IPv6 (IP version 6) */

#define _ICMP_HLEN  8  /* ICMP header length (in bytes) */
#define _ICMPV6_HLEN_MIN 4  /* ICMPv6 header minimum length (in bytes) */
#define _ICMPV6_HLEN (_ICMPV6_HLEN_MIN + 56)  /* ICMPv6 MAX header length (in bytes) */
#define _IP_HLEN_MIN 20  /* minimum IP   header length (in bytes)  */
#define _IPV6_HLEN  40  /* IPv6 header length (fixed) (in bytes)  */

#define _IPH_TTL  80  /* Initial time-to-live value */
#define _IPH_MF   0x2000 /* more fragments bit   */
#define _IPH_DF   0x4000 /* don't fragment bit   */
#define _IPH_FRAGOFF 0x1fff  /* fragment offset mask   */

#define E_IP_SENT  0
#define E_IP_PENDING -1

#define _IPV6_ADDR_INVALID  0x00
#define _IPV6_ADDR_TENTATIVE 0x01
#define _IPV6_ADDR_PREFERRED 0x02
#define _IPV6_ADDR_DEPRECATED 0x04
#define _IPV6_ADDR_VALID  0x08

/* ICMPv4 */
#define _ICMP_ECHO_REPLY (0)  /* Echo reply    */
#define _ICMP_ECHO_REQ  (8)  /* Echo request    */

/* ICMPv6 */
#define _ICMPV6_ECHO_REQ (128)
#define _ICMPV6_ECHO_REPLY (129)
#define _ICMPV6_GRP_QUE  (130)
#define _ICMPV6_GRP_REP  (131)
#define _ICMPV6_GRP_TERM (132)
#define _ICMPV6_RTR_SOL  (133)
#define _ICMPV6_RTR_ADV  (134)
#define _ICMPV6_NBR_SOL  (135)
#define _ICMPV6_NBR_ADV  (136)

/* ICMP frag for transmit */
#define _ICMPV6_SND_ERP  0x0001
#define _ICMPV6_SND_RTS  0x0002
#define _ICMPV6_SND_RTA  0x0004
#define _ICMPV6_SND_NBS  0x0008
#define _ICMPV6_SND_NBA  0x0010
#define _ICMPV6_SND_5  0x0020
#define _ICMPV6_SND_6  0x0040
#define _ICMPV6_SND_7  0x0080
#define _ICMPV6_SND_8  0x0100


/* multicast address */
#define _IP_TYPE_NON_MULTI   0
#define _IP_TYPE_MULTI_RESERVED  1
#define _IP_TYPE_MULTI_ALL_HOST  2
#define _IP_TYPE_MULTI_ANY   3
#define _IP_TYPE_MULTI_ALL_RTR  4

/* broadcast address */
#define _IP_TYPE_NON_BROAD   0
#define _IP_TYPE_BROADCAST   1
#define _IP_TYPE_DIRECTED_BROADCAST 2

/* address cache status */
#define _IS_FREE  0x00
#define _IS_INCOMPLETE 0x01
#define _IS_REACHABLE 0x02
#define _IS_STALE  0x04
#define _IS_DELAY  0x08
#define _IS_PROBE  0x10
#define _IS_TMOUT  0x20

#define _ICMP_MAX_MULTICAST_SOLICIT  2
#define _ICMP_MAX_UNICAST_SOLICIT  2
#define _ICMP_REACHABLE_TIME   (30*100)
#define _ICMP_RETRANS_TIMER    (1*100)
#define _ICMP_DELAY_FIRST_PROBE_TIME (5*100)

typedef struct
{
    uint8_t  ip_ver_len;
    uint8_t  ip_tos;
    uint16_t  ip_total_len;
    uint16_t  ip_id;
    uint16_t  ip_fragoff;
    uint8_t  ip_ttl;
    uint8_t  ip_proto_num;
    uint16_t  ip_chksum;
    IPaddr  ip_src;
    IPaddr  ip_dst;
} _IP_HDR;

typedef struct
{
    _IP_HDR  iph;   /* IP header */
    uint8_t  data[1];  /* DATA (upper protocol)*/
} _IP_PKT;


typedef struct
{
    uint32_t  rsvd;
    uint8_t  tgt_ip[16];
    uint8_t  opt_type;
    uint8_t  opt_len;
    uint8_t  opt[6];
} _ICMP_NBR_PKT;

typedef struct
{
    uint32_t  rsvd;
    uint8_t  opt_type;
    uint8_t  opt_len;
    uint8_t  opt[6];
} _ICMP_RTR_SOL_PKT;

typedef struct
{
    uint8_t  type;
    uint8_t  len;
    uint8_t  px_len;
    uint8_t  rsvd;
    uint32_t  valid_lt;
    uint32_t  pref_lt;
    uint32_t  rsvd2;
    uint8_t  prefix[16];
} _ICMP_OPT_PFX;

typedef struct
{
    uint8_t  hop_limit;
    uint8_t  flag;
    uint16_t  ef_time;
    uint32_t  reachable_time;
    uint32_t  reachable_rtx_timer; // 12
    uint8_t  opt_type;
    uint8_t  opt_len;
    uint8_t  opt[6];
    _ICMP_OPT_PFX pfx_opt;
} _ICMP_RTR_ADV_PKT;

typedef struct
{
    uint16_t id;     /* message id */
    uint16_t seq;    /* sequence number */
} _ICMP_ECHO_PKT;


typedef struct _icmp_hdr
{
    uint8_t type;    /* type of message */
    uint8_t code;    /* code (additional info. depend on type) */
    uint16_t chksum;    /* checksum of ICMP header+data   */
    uint16_t id;     /* message id */
    uint16_t seq;    /* sequence number */
} _ICMP_HDR;

typedef struct
{
    _ICMP_HDR icmph;
    uint8_t  data[1];
} _ICMP_PKT;

/* Channel information  for v.2.00 */
typedef struct _channel_info
{
    uint8_t  _ch_num;
    _P_RCV_BUF _p_rcv_buf;
    uint32_t _myipaddr[1];
    uint16_t _ip_id;
    uint16_t _rcvd;
    uint16_t flag;
#if defined(_ETHER)
    uint32_t _mymaskaddr[1];
    uint32_t _mygwaddr[1];
    uint8_t  arp_buff[28];
#endif
} _CH_INFO;

extern _CH_INFO *_ch_info_tbl;
extern _CH_INFO *_ch_info_head;


int16_t _ip_rcv_hdr(void);
int16_t _ip_snd(uint8_t *data, uint16_t len) ;
#if defined(_ETHER)
void _ip_icmp_init_tbl(void);
void _ip_icmp_update_tbl(void);
void _ip_icmp_clear_tbl(ARP_ENTRY *ae);
ARP_ENTRY *_ip_search_tbl(void);
#endif
void _ip_snd_icmp(void);

uint16_t _cksum(const uint8_t *data, uint16_t nbytes, uint16_t sum0);
#if defined(M16C) || defined(R8C)   /* assembler */
#pragma PARAMETER _cksum(a0, r0, r1);
#elif defined(M16C80) || defined(M32C80) /* assembler */
#pragma PARAMETER /C _cksum(a0, r0, r1);
#endif

int8_t _ip_check_ipadd_proto(_IP_HDR *piph);
uint8_t _ip_check_multicast(uint8_t *ipaddr);
uint8_t _ip_check_broadcast(uint8_t *ipaddr);
int16_t _ppp_snd_ip(uint8_t *data, uint16_t len);






