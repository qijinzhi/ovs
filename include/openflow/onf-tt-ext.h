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

/* TT flow control message type */
enum onf_tt_flow_ctrl_type {
    ONF_TFCT_DOWNLOAD_START_REQUEST = 0,
    ONF_TFCT_DOWNLOAD_START_REPLY = 1,
    ONF_TFCT_DOWNLOAD_END_REQUEST = 2,
    ONF_TFCT_DOWNLOAD_END_REPLY	= 3,
    ONF_TFCT_CLEAR_OLD_REQUEST = 4,
    ONF_TFCT_CLEAR_OLD_REPLY = 5,
    ONF_TFCT_QUERY_TABLE_REQUEST = 6,
    ONF_TFCT_QUERY_TABLE_REPLY = 7,
};

/* Message structure for ONF_ET_TT_FLOW_CONTROL. */
struct onf_tt_flow_ctrl {
    ovs_be32 type; /* ONF_TFCT_*. */
    ovs_be32 flow_count; /* The count of flow. */
};
OFP_ASSERT(sizeof(struct onf_tt_flow_ctrl) == 8);

/* Message structure for ONF_ET_TT_FLOW_MDOD. */
struct onf_tt_flow_mod {
    /* Entry field */
    ovs_be32 port; /* The entry related port. */
    ovs_be32 etype; /* Send entry or receive entry. */
    ovs_be32 flow_id; /* The identify of a flow. */
    uint8_t pad[4];
    ovs_be64 base_offset; /* The scheduled time that the flow packet is received or sent. */
    ovs_be64 period; /* The scheduling period. */
    ovs_be32 buffer_id; /* Buffered packet to apply to. */
    ovs_be32 packet_size; /* The flow packet size. */
    ovs_be64 execute_time; /* The time this entry take effect. */
};
OFP_ASSERT(sizeof(struct onf_tt_flow_mod) == 48);

#endif /* openflow/onf-tt-ext.h */
