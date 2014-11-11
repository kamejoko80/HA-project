/**
 * @file ha_sixlowpan.cpp
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.0
 * @date 08-Nov-2014
 * @brief This is header holds common function implementations for 6lowpan network
 * of home automation nodes and cc.
 */

extern "C" {
#include "net_if.h"
#include "rpl.h"
#include "socket_base/socket.h"
}

#include "ha_sixlowpan.h"
#include "ff.h"

#define HA_NOTIFICATION (1)
#define HA_DEBUG_EN (1)
#include "ha_debug.h"

#ifdef HA_CC
#include "cc_slp_sender.h"
#include "cc_slp_receiver.h"
#endif

namespace ha_ns {
/* 6LoWPAN sender and receiver threads */
#ifdef HA_CC
kernel_pid_t *sixlowpan_sender_pid = &ha_cc_ns::slp_sender_pid;
kernel_pid_t *sixlowpan_receiver_pid = &ha_cc_ns::slp_receiver_pid;

cir_queue *slp_sender_gff_queue_p = &ha_cc_ns::slp_sender_gff_queue;
#else
kernel_pid_t *sixlowpan_sender_pid = &ha_node_ns::slp_sender_pid;
#endif

/* Communications global vars */
ipv6_addr_t sixlowpan_ipaddr;
uint16_t sixlowpan_node_id = 0;
char sixlowpan_netdev_type = '0';
}

/*----------------------------------------------------------------------------*/
int16_t ha_slp_readconfig(const char* path, const char* pattern, uint16_t pattern_size,
        uint16_t* prefixes_p, uint16_t &node_id, char &netdev_type, uint16_t &channel)
{
    FIL config_file;
    char config_string[pattern_size];
    UINT byte_read = 0;
    uint16_t allzero_prefixes[4];

    uint32_t prefixes32[4];
    uint32_t node_id32, channel32;

    uint16_t count;

    /* read configurations from file */
    if (f_open(&config_file, path, FA_READ | FA_OPEN_EXISTING) != FR_OK) {
        HA_DEBUG("ha_slp_rdconf: Can't open %s\n", path);
        return -1;
    }
    HA_DEBUG("ha_slp_rdconf: %s opened \n", path);

    if (f_read(&config_file, config_string, pattern_size, &byte_read) !=  FR_OK) {
        HA_DEBUG("ha_slp_rdconf: error while reading configurations, bytes read: %u.\n", byte_read);

        /* close file */
        if (f_close(&config_file) != FR_OK) {
            HA_DEBUG("ha_slp_rdconf: error when close %s\n", path);
        }
        return -1;
    }

    /* close file */
    if (f_close(&config_file) != FR_OK) {
        HA_DEBUG("ha_slp_rdconf: error when close %s\n", ha_ns::sixlowpan_config_file);
    }

    HA_DEBUG("ha_slp_rdconf: bytes read %u.\n", byte_read);
    config_string[byte_read] = '\0';

    memset(prefixes32, 0, sizeof(prefixes32));
    sscanf(config_string, pattern,
            &prefixes32[3], &prefixes32[2], &prefixes32[1], &prefixes32[0],
            &node_id32,
            &netdev_type,
            &channel32);

    for (count = 0; count < 4; count++) {
        prefixes_p[count] = (uint16_t) prefixes32[count];
    }
    node_id = (uint16_t) node_id32;
    channel = (uint16_t) channel32;

    HA_DEBUG("ha_slp_rdconf: configurations\n"
            "\tprefixes: %x:%x:%x:%x\n"
            "\tnode_id: %x\n"
            "\tnetdev_type: %c\n"
            "\tchanel: %d\n",
            prefixes_p[3], prefixes_p[2], prefixes_p[1], prefixes_p[0],
            node_id, netdev_type, channel);

    memset(allzero_prefixes, 0, sizeof(allzero_prefixes));
    if (memcmp(prefixes_p, allzero_prefixes, sizeof(prefixes_p)) == 0) {
        HA_DEBUG("ha_slp_rdconf: prefixes are all zeros.\n");
        return -1;
    }
    if (node_id == 0) {
        HA_DEBUG("ha_slp_rdconf: node id = 0.\n");
        return -1;
    }
    if (netdev_type != 'h' && netdev_type != 'r' && netdev_type != 'n') {
        HA_DEBUG("ha_slp_rdconf: net type is %c (%d).\n", netdev_type, netdev_type);
        return -1;
    }

    return 0;
}

/*----------------------------------------------------------------------------*/
int16_t ha_slp_init(uint8_t interface, transceiver_type_t transceiver,
        uint16_t* prefixes_p, uint16_t node_id, char netdev_type, uint16_t channel)
{
    uint16_t ret_hwaddr = 0;
    uint8_t ret_valu8;
    ipv6_addr_t ipaddr;
    char addr_str[IPV6_MAX_ADDR_STR_LEN];
    transceiver_command_t tcmd;
    msg_t m;

    /* Use short addresses */
    net_if_set_src_address_mode(interface, NET_IF_TRANS_ADDR_M_SHORT);

    /* Set hardware address */
    ret_hwaddr = net_if_set_hardware_address(interface, node_id);
    if (ret_hwaddr > 0) {
        HA_DEBUG("ha_slp_init: hardware address set to %u.\n", ret_hwaddr);
    }
    else {
        HA_DEBUG("ha_slp_init: failed to set hardware address.\n");
        return -1;
    }

    /* Init RPL */
    ret_valu8 = rpl_init(interface);
    if (ret_valu8 != SIXLOWERROR_SUCCESS) {
        HA_DEBUG("ha_slp_init: error initializing RPL\n");
        return -1;
    }
    else {
        HA_DEBUG("ha_slp_init: RPL initialized\n");
    }

    /* Init network device */
    switch (netdev_type) {
    case 'r':
        rpl_init_root();
        HA_DEBUG("ha_slp_init: initialized as root\n");
        break;
    case 'n':
        ipv6_iface_set_routing_provider(rpl_get_next_hop);
        HA_DEBUG("ha_slp_init: initialized as node router\n");
        break;
    case 'h':
        /* TODO: need to be implemented when RIOT support host */
        HA_DEBUG("ha_slp_init: initialized as host (do nothing for now)\n");
        break;
    default:
        HA_DEBUG("ha_slp_init: unknow netdev_type %c(%d)\n", netdev_type, netdev_type);
        return -1;
    }

    /* add a global address */
    ipv6_addr_init(&ipaddr, prefixes_p[3], prefixes_p[2], prefixes_p[1], prefixes_p[0],
            0x0, 0x0, 0x0, node_id);
    ipv6_addr_set_by_eui64(&ipaddr, interface, &ipaddr);
    if (ipv6_net_if_add_addr(interface, &ipaddr,
            NDP_ADDR_STATE_PREFERRED, 0, 0, 0) != 1) {
        HA_DEBUG("ha_slp_init: failed to add global address to interface %d\n",
                interface);
        return -1;
    }
    else {
        HA_DEBUG("ha_slp_init: %s is added to interface %d\n",
                ipv6_addr_to_str(addr_str, IPV6_MAX_ADDR_STR_LEN, &ipaddr),
                interface);
    }

    if (netdev_type == 'r' || netdev_type == 'n') {
        ipv6_init_as_router();
    }

    /* Set channel */
    tcmd.transceivers = transceiver;
    tcmd.data = &channel;
    m.type = SET_CHANNEL;
    m.content.ptr = (char *) &tcmd;

    msg_send_receive(&m, &m, transceiver_pid);
    HA_DEBUG("ha_slp_init: channel is set to %u\n", channel);

    /* Save configurations to global vars */
    memcpy(&ha_ns::sixlowpan_ipaddr, &ipaddr, 16);
    ha_ns::sixlowpan_node_id = node_id;
    ha_ns::sixlowpan_netdev_type = netdev_type;

    return 0;
}