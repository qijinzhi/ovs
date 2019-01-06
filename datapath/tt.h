/*
  * Copyright (c) 2018-2019 Tsinghua University, Inc.
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of version 2 of the GNU General Public
  * License as published by the Free Software Foundation.
  *
  * This program is distributed in the hope that it will be useful, but
  * WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  * General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  * 02110-1301, USA
 */


#ifndef TT_H
#define TT_H 1

#include <linux/types.h>
#include <linux/skbuff.h>

#define TT_FLOW_ID_MAX  1024	/* max flow id */
#define TT_PORT 63000   /* tt flow dest port */
#define ETH_P_TT 0x88D7 /* tt flow ethernet type */

#define TT_HLEN 4   /* tt header length */
#define TT_TABLE_SIZE_MIN 16	/* minimum size of tt_table */
#define TT_BUFFER_SIZE 4096 /* one flow one buffer */
#define MAX_JITTER 100000
#define NSEC_PER_SECOND 1000000000
#define NSEC_PER_MSECOND 1000000
#define NSEC_PER_USECOND 1000

#define TIMESPEC_TO_NSEC(time_spec) \
	(time_spec.tv_sec * (u64)NSEC_PER_SECOND + time_spec.tv_nsec)

#define SWAP(x, y) x = x^y; y = x^y; x = x^y;

struct tt_header {
	u16 flow_id; /* tt flow_id */
	u16 len; /* tt packet's length */
};

/** ===>>>
	note:
	假设控制器给的tt报文的flow_id都是从1开始依次增长，所以下面的实现过程直接用flow_id-1作为数组的索引
	则在这种情况下，最大的flow_id即为tt_table->count
	当对表项无论进行删除还是修改，都应该保证表中的flow_id让仍然是从1开始
	之后开发的过程中，这部分可能需要进行修改
**/
/**
  struct tt_table_item - tt schedule table item, 
						 must be protected by rcu.
  @flow_id: tt flow identifier.
  @buffer_id: buffer to which this tt flow should be store.
  @rcu: Rcu callback head of deferred destruction.
  @packet_size: tt packet's length
  @period: the period of tt flow (ns).
  @base_offset: send time or receive time in current period (ns).
 */
struct tt_table_item {
	u16 flow_id;
	u16 buffer_id;
	struct rcu_head rcu;
	u16 packet_size;
	u64 period;
	u64 base_offset;
};

/**
  struct tt_table - tt schedule table, 
					must be protected by rcu.
  @rcu: Rcu callback head of deferred destruction.
  @count: total number of tt flow in this tt_table.
  @max: max tt_table size.
  @tt_items: tt flow items.
 */
struct tt_table {
	struct rcu_head rcu;
	u16 count, max;
	struct tt_table_item* __rcu tt_items[];
};

struct tt_send_cache {
	u64 *send_times;
	u16 *flow_ids;
	u16 size;
};

struct tt_send_info {
	u64 macro_period;
	u64 advance_time;
	struct tt_send_cache send_cache; 
};

/* tt operation */
bool udp_port_is_tt(__be16 port);
bool eth_p_tt(__be16 eth_type);
unsigned char *skb_tt_header(struct sk_buff *skb);
bool is_trdp_packet(struct sk_buff *skb);
bool is_tt_packet(struct sk_buff *skb);
int trdp_to_tt(struct sk_buff *skb);
int tt_to_trdp(struct sk_buff *skb);

/* tt_table operation */
struct tt_table_item *tt_table_item_alloc(void);
void rcu_free_tt_table(struct rcu_head *rcu);
struct tt_table *tt_table_alloc(int size);
struct tt_table_item* tt_table_lookup(const struct tt_table* cur_tt_table, const __be16 flow_id);
int tt_table_num_items(const struct tt_table* cur_tt_table);
struct tt_table* tt_table_delete_item(struct tt_table* cur_tt_table, __be16 flow_id);
struct tt_table* tt_table_item_insert(struct tt_table *cur_tt_table, const struct tt_table_item *new);

/* tt send info */
u64 global_time_read(void);
int dispatch(struct vport* vport);
void get_next_time(struct vport *vport, u64 cur_time, u64 *wait_time, u16 *flow_id, u64 *send_time);
#endif
