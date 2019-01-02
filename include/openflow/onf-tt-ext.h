/*
 * Copyright (c) 2018 Tsinghua University, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OPENFLOW_ONF_TT_EXT_H
#define OPENFLOW_ONF_TT_EXT_H 1

#include <openflow/openflow.h>
#include <openvswitch/types.h>

/* The following experiment extensions, proposed by Tsinghua University, 
 * are not yet standardized, so they are not included in openflow.h. 
 * These extensions are based ONF experimenter mechanism. */

/* Tsinghua experiment Time-Triggered mechanism extension.
 *
 * +---------+---------------+--------+
 * | version | ONF_VENDOR_ID | length |   
 * +---------+---------------+--------+
 * |     struct onf_exp_header        |
 * +----------------------------------+
 */
 
/* Experiment extension message. */
struct onf_exp_header {
    struct ofp_header header; 
    ovs_be32 vendor;    /* ONF_VENDOR_ID */
    ovs_be32 subtype;   /* See the TXT numbers in ofp-mags.h. */
};
OFP_ASSERT(sizeof(struct onf_exp_header) == 16);


/* TT table commands 
enum tx_tt_table_mod_command {
    OFPFC_ADD,
};
*/

/* Message structure for ONF_ET_TT_FLOW_MDOD. */
struct onf_tt_flow_mod {
    /* Command type */
    uint8_t command; /* One of OFPFC_* */
    /* Entry field */
    uint8_t port; /* The entry related port. */
    uint8_t etype; /* Send entry or receive entry. */
    uint8_t flow_id; /* The identify of a flow. */
    ovs_be32 scheduled_time; /* The scheduled time that the flow packet is received or sent. */
    ovs_be32 period; /* The scheduling period. */
    ovs_be32 buffer_id; /* Buffered packet to apply to. */
    ovs_be32 pkt_size; /* The flow packet size. */
};
OFP_ASSERT(sizeof(struct onf_tt_flow_mod) == 20);

#endif /* openflow/onf-tt-ext.h */
