/*
 * Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015 Nicira, Inc.
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

#ifndef OPENFLOW_TSINGHUA_EXT_H
#define OPENFLOW_TSINGHUA_EXT_H 1

#include <openflow/openflow.h>
#include <openvswitch/types.h>

/* tsinghua-ext.h
 * this is openflow interface between SDN controller and ovs, which 
 * acts as a openflow switch. 
 * the tt(time trigger) openflow message format looks like:
 * 
 * +---------+--------------------------+--------+
 * | version | OFPTYPE_TXT_TT_TABLE_MOD | length |   
 * +---------+--------------------------+--------+
 * |           struct tx_tt_table_mod           |
 * +---------------------------------------------+
 *
 */

/* TT table commands */
enum tx_tt_table_mod_command {
    TXTTMC_ADD,          /* New mappings (fails if an option is already
                            mapped). */
};

/* tt table item */
struct tx_tt_flow {
	ovs_be16 flow_id;
	ovs_be16 pad;
    ovs_be32 cycle;
};

#define MAX_TT_TABLE_SIZE 1024

/* tt table mod (controller -> datapath). */
struct tx_tt_table_mod {
	ovs_be16 command;
    ovs_be16 tt_table_size;
	struct tx_tt_flow tt_table[MAX_TT_TABLE_SIZE];
};

#endif /* openflow/tsinghua-ext.h */
