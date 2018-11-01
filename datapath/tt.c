#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include "datapath.h"
#include "tt.h"

bool udp_port_is_tt(__be16 port) {
    return port == htons(TT_PORT);
}

// 以太网协议头类型为TT_ETH_ETYPE为TT报文
bool eth_p_tt(__be16 eth_type) {
    return eth_type == htons(ETH_P_TT);
}

unsigned char *skb_tt_header(struct sk_buff *skb) {
    return skb_mac_header(skb) + skb->mac_len;
}

// 判断是否为TRDP报文
// note：传入的skb结构体中，其skb->h、skb->nh、skb->mac指针都已经被指向了正确的位置，
// 因为经过了extract_key的处理，所以我们只需要直接使用这些指针头，就可以定位到对应网络层的协议头
bool is_trdp_packet(struct sk_buff *skb) {
    struct ethhdr *eth;
    struct iphdr *nh;
    struct udphdr *udp;

    // 先检查是否为包含TT信息的TRDP报文
    // 特征：传输层UDP、网络层为ipv4、UDP目的端口TT_PORT
    eth = eth_hdr(skb);
    if (eth->h_proto != htons(ETH_P_IP))
        return 0;

    nh = ip_hdr(skb);
    if (nh->protocol != IPPROTO_UDP)
        return 0;

    udp = udp_hdr(skb);
    if (!udp_port_is_tt(udp->dest))
        return 0;

    return 1;
}

// 判断是否为TT报文
// note：传入的skb结构体中，其skb->h、skb->nh、skb->mac指针都已经被指向了正确的位置，
// 因为经过了extract_key的处理，所以我们只需要直接使用这些指针头，就可以定位到对应网络层的协议头
bool is_tt_packet(struct sk_buff *skb) {
    struct ethhdr *eth;
    eth = eth_hdr(skb);

    return eth_p_tt(eth->h_proto);
}

// 添加tt头
static int push_tt(struct sk_buff *skb, const __be16* flow_id) {
    struct tt_header *tt_hdr;
    struct ethhdr *eth;

    //skb_cow_head是看skb是否有足够的余量来装入tt头，不够会重新分配。
    if (skb_cow_head(skb, TT_HLEN) < 0) {
        printk(KERN_ALERT "DEBUG: push tt fail!  %s %d \n", __FUNCTION__, __LINE__);
        return -ENOMEM;
    }

    // 添加tt头的空间，然后将链路层协议头整体往前移动TT_HLEN个字节
    skb_push(skb, TT_HLEN);
    memmove(skb_mac_header(skb) - TT_HLEN, skb_mac_header(skb), skb->mac_len);
    skb_reset_mac_header(skb);

    //修改以太网报文类型
    eth = eth_hdr(skb);
    eth->h_proto = htons(ETH_P_TT);

    //填充tt报文头内容
    tt_hdr = (struct tt_header*)skb_tt_header(skb);
    tt_hdr->flow_id = *flow_id;
    tt_hdr->len = skb->len - 4; //？？？？这里可能存在的问题，tt中的len指的是报文的整体长度，但是不包括CRC，skb->data中是否有CRC待考究，除此之外还有一些padding的部分也待考究
    return 0;
}

// 删除tt头
static int pop_tt(struct sk_buff *skb) {
    struct ethhdr *hdr;
    int err;

    // 判断当前skb是否能写
    err = skb_ensure_writable(skb, skb->mac_len + TT_HLEN);
    if (unlikely(err))
        return err;

    memmove(skb_mac_header(skb) + TT_HLEN, skb_mac_header(skb), skb->mac_len);

    __skb_pull(skb, TT_HLEN);
    skb_reset_mac_header(skb);

    hdr = (struct ethhdr *)(skb_tt_header(skb) - ETH_HLEN);
    hdr->h_proto = htons(ETH_P_IP);

    return 0;
}

// 将TRDP报文转化为TT报文
// note：已经调用过is_trdp_packet函数，并确定通过了is_trdp_packet函数
int trdp_to_tt(struct sk_buff *skb) {
    //if (!is_trdp_packet(skb))
    //  return -EINVAL;

    // udp数据域的前四个字节为flow_id
    void* udp_data = skb_transport_header(skb) + sizeof(struct udphdr);
    __be16* flow_id = (__be16*)udp_data;
    printk(KERN_ALERT "DEBUG: cur tt flow_id is: %d %s %d \n", *flow_id,__FUNCTION__, __LINE__);

    return push_tt(skb, flow_id);
}

// 将TT报文转化为TRDP报文
// note：已经调用过is_tt_packet函数，并确定通过了is_tt_packet函数
int tt_to_trdp(struct sk_buff *skb) {
    //if (!is_tt_packet(skb))
    //  return -EINVAL;

    return pop_tt(skb);
}


//分配一个新的tt_table_item
struct tt_table_item *tt_table_item_alloc(void) {
	struct tt_table_item *item;

	item = kmalloc(sizeof(*item), GFP_KERNEL);
    if (!item) {
        printk(KERN_ALERT "DEBUG: tt_table_item alloc fail!  %s %d \n", __FUNCTION__, __LINE__);
        return NULL;
    }
	return item;
}

void rcu_free_tt_table_item(struct rcu_head *rcu) {
    struct tt_table_item* item = container_of(rcu, struct tt_table_item, rcu);
    
    kfree(item);
}

//释放内存rcu_head* rcu所在的结构体
void rcu_free_tt_table(struct rcu_head *rcu) {
	struct tt_table *tt = container_of(rcu, struct tt_table, rcu);  //container_of：通过结构体变量中某个成员的首地址进而获得整个结构体变量的首地址

	kfree(tt);
}

// 分配tt_table空间
struct tt_table *tt_table_alloc(int size) {
	struct tt_table *new;

	size = max(TT_TABLE_SIZE_MIN, size); //size+1是因为0没有使用，从1开始
	new = kzalloc(sizeof(struct tt_table) +
		sizeof(struct tt_table_item *) * size, GFP_KERNEL);
	if (!new) {
		printk(KERN_ALERT "DEBUG: tt_table_alloc fail!  %s %d \n", __FUNCTION__, __LINE__);
        return NULL;
    }

	new->count = 0;
	new->max = size;

	return new;
}

// tt_table重新分配空间
static struct tt_table* tt_table_realloc(struct tt_table *old, int size) {
	struct tt_table *new;

	new = tt_table_alloc(size);
	if (!new) {
		printk(KERN_ALERT "DEBUG: tt_table realloc fail!  %s %d \n", __FUNCTION__, __LINE__);
        return NULL;
    }

	if (old) {
		int i;

		for (i = 0; i < old->max; i++) {
			if (ovsl_dereference(old->tt_items[i]))
				new->tt_items[i] = old->tt_items[i];
		}

		new->count = old->count;
	}

	if (old)
		call_rcu(&old->rcu, rcu_free_tt_table);

	return new;
}

// 根据flow_id查找tt_table
struct tt_table_item* tt_table_lookup(const struct tt_table* cur_tt_table, const __be16 flow_id) {
	struct tt_table_item* tt_item;
	if (!cur_tt_table || flow_id > cur_tt_table->max) return NULL;

	tt_item = ovsl_dereference(cur_tt_table->tt_items[flow_id - 1]);
	return tt_item;
}

// 获得当前tt表中的表项个数
int tt_table_num_items(const struct tt_table* cur_tt_table) {
	return cur_tt_table->count;
}

// 删除指定flow_id的表项
// 错误返回NULL
struct tt_table* tt_table_delete_item(struct tt_table* cur_tt_table, __be16 flow_id) {
	struct tt_table_item* tt_item;
	if (!cur_tt_table || flow_id > cur_tt_table->max) {
        printk(KERN_ALERT "DEBUG: flow_id to big! tt_table_item delete fail!  %s %d \n", __FUNCTION__, __LINE__);
        return NULL;
    }

	tt_item = ovsl_dereference(cur_tt_table->tt_items[flow_id - 1]);
	if (tt_item) {
		RCU_INIT_POINTER(cur_tt_table->tt_items[flow_id - 1], NULL);
		cur_tt_table->count--;
        call_rcu(&tt_item->rcu, rcu_free_tt_table_item);
	}

	//必要时缩小flow_id
	if (cur_tt_table->max >= (TT_TABLE_SIZE_MIN * 2) &&
		cur_tt_table->count <= (cur_tt_table->max / 3)) {
		struct tt_table* res_tt_table;
		res_tt_table = tt_table_realloc(cur_tt_table, cur_tt_table->max / 2);
		if (!res_tt_table) {
            printk(KERN_ALERT "DEBUG: tt_table_item delete fail!  %s %d \n", __FUNCTION__, __LINE__);
            return cur_tt_table;
        }
        
        return res_tt_table;
	}
	return cur_tt_table;
}


//插入tt_table_item
//错误返回NULL
struct tt_table* tt_table_item_insert(struct tt_table *cur_tt_table, const struct tt_table_item *new) {

	__be16 flow_id = new->flow_id;
	struct tt_table_item *item;

	item = tt_table_item_alloc(); //申请一个新的tt表项
	if (!item) {
		printk(KERN_ALERT "DEBUG: tt_table_item insert fail!  %s %d \n", __FUNCTION__, __LINE__);
        return NULL;
    }

	item->flow_id = new->flow_id;
	item->buffer_id = new->buffer_id;
	item->circle = new->circle;
	item->len = new->len;
	item->time = new->time;
    
	//mask_array中实际长度大于最大长度，则重新分配mask_array空间（扩容）
	if (!cur_tt_table || flow_id > cur_tt_table->max) {
	    struct tt_table* res_tt_table;
		res_tt_table = tt_table_realloc(cur_tt_table, flow_id + TT_TABLE_SIZE_MIN); // 给tbl->mask_array重新分配
		if (!res_tt_table) {
            printk(KERN_ALERT "DEBUG: tt_table_item insert fail!  %s %d \n", __FUNCTION__, __LINE__);
			kfree(item);
            return NULL;
		}

		rcu_assign_pointer(cur_tt_table, res_tt_table);
	}
    
    rcu_assign_pointer(cur_tt_table->tt_items[flow_id - 1], item);
	return cur_tt_table;
}

