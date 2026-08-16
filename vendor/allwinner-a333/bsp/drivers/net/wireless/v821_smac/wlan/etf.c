/*
 * ETF interfaces for XRadio drivers
 *
 * Copyright (c) 2013
 * Xradio Technology Co., Ltd. <www.xradiotech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef XRADIO_ETF_C
#define XRADIO_ETF_C
#include <linux/module.h>
#include <linux/init.h>
#include <linux/vmalloc.h>
#include <linux/netlink.h>
#include <net/sock.h>
#include <linux/kthread.h>
#include <linux/wait.h>
#include <linux/firmware.h>
#include <uapi/linux/sched/types.h>
#ifdef CONFIG_DRIVER_V821
#include <sunxi-sid.h>
#include <sunxi-smc.h>
#include <linux/of.h>
#include <linux/of_address.h>
#endif

#include "etf.h"
#include "xradio.h"
#include "sbus.h"
#include "platform.h"
#include "hwio.h"

/********************* interfaces of adapter layer **********************/
static struct xradio_adapter adapter_priv;

#if (ETF_QUEUEMODE)
static void adapter_rec_handler(struct sk_buff *__skb)
{
	int i = 0;
	struct adapter_item *item;
	etf_printk(XRADIO_DBG_TRC, "%s\n", __func__);

	do {
		spin_lock_bh(&adapter_priv.recv_lock);
		if (likely(!list_empty(&adapter_priv.rx_pool))) {
			item = list_first_entry(&adapter_priv.rx_pool, struct adapter_item, head);
			item->skb = (void *)skb_get(__skb);
			list_move_tail(&item->head, &adapter_priv.rx_queue);
			spin_unlock_bh(&adapter_priv.recv_lock);
			etf_printk(XRADIO_DBG_NIY, "%s recv msg to rx_queue!\n", __func__);
			break;
		} else if (++i > ETF_QUEUE_TIMEOUT) { /* about 2s.*/
			spin_unlock_bh(&adapter_priv.recv_lock);
			etf_printk(XRADIO_DBG_ERROR,
				"%s driver is process timeout, drop msg!\n", __func__);
			return;
		} else {
			etf_printk(XRADIO_DBG_NIY,
				"%s rx_pool is empty, wait %d!ms\n", __func__, i);
		}
		spin_unlock_bh(&adapter_priv.recv_lock);
		msleep(i);
	} while (1);

	if (atomic_add_return(1, &adapter_priv.rx_cnt) == 1)
		up(&adapter_priv.proc_sem);
	return;
}
#else
static void adapter_rec_handler(struct sk_buff *skb)
{
	struct nlmsghdr *nlhdr = (struct nlmsghdr *)skb->data;
	etf_printk(XRADIO_DBG_TRC, "%s\n", __func__);

	if (nlhdr->nlmsg_len < sizeof(struct nlmsghdr)) {
		etf_printk(XRADIO_DBG_ERROR, "Corrupt netlink msg!\n");
	} else {
		int len = nlhdr->nlmsg_len - NLMSG_LENGTH(0);
		adapter_priv.send_pid = nlhdr->nlmsg_pid;
		adapter_priv.msg_len  = min(ADAPTER_RX_BUF_LEN, len);
		memcpy(adapter_priv.msg_buf, NLMSG_DATA(nlhdr), adapter_priv.msg_len);
		if (len != adapter_priv.msg_len)
			etf_printk(XRADIO_DBG_WARN, "%s recv len(%d), proc len=%d!\n",
					__func__, len, adapter_priv.msg_len);
		else
			etf_printk(XRADIO_DBG_NIY, "%s recv msg len(%d)!\n",
					__func__, adapter_priv.msg_len);
		if (adapter_priv.handler)
			adapter_priv.handler(adapter_priv.msg_buf, adapter_priv.msg_len);

	}
}
#endif

static int adapter_init_msgbuf(void)
{
#if (ETF_QUEUEMODE)
	int i = 0;
#endif
	etf_printk(XRADIO_DBG_NIY, "%s\n", __func__);
	if (adapter_priv.msg_buf) {
		etf_printk(XRADIO_DBG_ERROR, "%s msgbuf already init!\n", __func__);
		return -1;
	}

#if (ETF_QUEUEMODE)
	atomic_set(&adapter_priv.rx_cnt, 0);
	INIT_LIST_HEAD(&adapter_priv.rx_queue);
	INIT_LIST_HEAD(&adapter_priv.rx_pool);
	for (i = 0; i < ADAPTER_ITEM_MAX; i++)
		list_add_tail(&adapter_priv.rx_items[i].head, &adapter_priv.rx_pool);
#endif

	adapter_priv.msg_buf  = xr_kzalloc(ADAPTER_RX_BUF_LEN, false);
	if (!adapter_priv.msg_buf)
		return -ENOMEM;
	return 0;
}

static void adapter_deinit_msgbuf(void)
{
#if (ETF_QUEUEMODE)
	struct  adapter_item *item;
#endif
	etf_printk(XRADIO_DBG_NIY, "%s\n", __func__);
	if (adapter_priv.msg_buf) {
		kfree(adapter_priv.msg_buf);
		adapter_priv.msg_buf = NULL;
	}
#if (ETF_QUEUEMODE)
	spin_lock_bh(&adapter_priv.recv_lock);
	list_for_each_entry(item, &adapter_priv.rx_queue, head) {
		if (item->skb) {
			dev_kfree_skb((struct sk_buff *)item->skb);
			item->skb = NULL;
		}
	}
	spin_unlock_bh(&adapter_priv.recv_lock);
#endif
}

#if (ETF_QUEUEMODE)
static int adapter_handle_rx_queue(void)
{
	int len = 0;
	struct nlmsghdr *nlhdr = NULL;
	struct adapter_item *item;
	bool bHandler = false;
	while (1) {
		spin_lock_bh(&adapter_priv.recv_lock);
		if (!list_empty(&adapter_priv.rx_queue)) {
			item = list_first_entry(&adapter_priv.rx_queue, struct adapter_item, head);
			if (item->skb) {
				struct sk_buff *skb = (struct sk_buff *)(item->skb);
				nlhdr = (struct nlmsghdr *)skb->data;
				if (nlhdr->nlmsg_len < sizeof(struct nlmsghdr)) {
					etf_printk(XRADIO_DBG_ERROR, "Corrupt netlink msg!\n");
				} else {
					len = nlhdr->nlmsg_len - NLMSG_LENGTH(0);
					adapter_priv.send_pid = nlhdr->nlmsg_pid;
					adapter_priv.msg_len  = min(ADAPTER_RX_BUF_LEN, len);
					memcpy(adapter_priv.msg_buf, NLMSG_DATA(nlhdr), len);
					bHandler = true;
					if (len != adapter_priv.msg_len)
						etf_printk(XRADIO_DBG_WARN, "%s recv len(%d), proc len=%d!\n",
								__func__, len, adapter_priv.msg_len);
					else
						etf_printk(XRADIO_DBG_NIY, "%s recv msg len(%d)!\n",
								__func__, adapter_priv.msg_len);
				}
				dev_kfree_skb(skb);
				item->skb = NULL;
				list_move_tail(&item->head, &adapter_priv.rx_pool);
			} else {
				etf_printk(XRADIO_DBG_ERROR, "%s skb is NULL!\n", __func__);
			}
		} else {
			spin_unlock_bh(&adapter_priv.recv_lock);
			break;
		}
		spin_unlock_bh(&adapter_priv.recv_lock);
		if (bHandler && adapter_priv.handler) {
			adapter_priv.handler(adapter_priv.msg_buf, adapter_priv.msg_len);
			bHandler = false;
		}
	}
	return 0;
}

static int adapter_proc(void *param)
{
	int ret  = 0;
	int term = 0;
	int recv = 0;
	etf_printk(XRADIO_DBG_NIY, "%s\n", __func__);
	for (;;) {
		ret = down_interruptible(&adapter_priv.proc_sem);
		recv = atomic_xchg(&adapter_priv.rx_cnt, 0);
		term = kthread_should_stop();
		etf_printk(XRADIO_DBG_NIY, "%s term=%d!\n", __func__, term);
		if (term || adapter_priv.exit || ret < 0) {
			break;
		}
		if (recv) {
			adapter_handle_rx_queue();
		}
	}
	etf_printk(XRADIO_DBG_NIY, "%s: exit!\n", __func__);
	return 0;
}
static int adapter_start_thread(void)
{
	int ret = 0;
	struct sched_param param = {.sched_priority = 100 };
	etf_printk(XRADIO_DBG_NIY, "%s\n", __func__);
	adapter_priv.thread_tsk = NULL;
	adapter_priv.exit = 0;
	adapter_priv.thread_tsk = kthread_create(&adapter_proc, NULL, XRADIO_ADAPTER);
	if (IS_ERR(adapter_priv.thread_tsk)) {
		ret = PTR_ERR(adapter_priv.thread_tsk);
		adapter_priv.thread_tsk = NULL;
	} else {
		sched_setscheduler(adapter_priv.thread_tsk, SCHED_NORMAL, &param);
		wake_up_process(adapter_priv.thread_tsk);
	}
	return ret;
}
static void adapter_stop_thread(void)
{
	etf_printk(XRADIO_DBG_NIY, "%s\n", __func__);
	if (adapter_priv.thread_tsk != NULL) {
		adapter_priv.exit = 1;
		up(&adapter_priv.proc_sem);
		kthread_stop(adapter_priv.thread_tsk);
		adapter_priv.thread_tsk = NULL;
	} else {
		etf_printk(XRADIO_DBG_WARN, "%s: thread_tsk is NULL!\n", __func__);
	}
}
#endif /* ETF_QUEUEMODE */

static int xradio_adapter_send(void *data, int len)
{
	int ret = 0;
	struct sk_buff  *skb = NULL;
	struct nlmsghdr *nlhdr = NULL;
	u8  *payload  = NULL;
	if (0 == len || NULL == data)
		return -EINVAL;

	skb = xr_alloc_skb(NLMSG_SPACE(len + 1));
	if (!skb) {
		etf_printk(XRADIO_DBG_WARN, "%s:xr_alloc_skb failed!\n", __func__);
		return -ENOMEM;
	}
	/* '\0' safe for strcpy, compat for old etf apk, may be removed in furture.*/
	nlhdr = nlmsg_put(skb, 0, 0, 0, len + 1, 0); /* (len+1) is payload */
	payload = (u8 *)NLMSG_DATA(nlhdr);
	memcpy(payload, data, len);
	payload[len] = '\0';

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 7, 0)
	NETLINK_CB(skb).pid = 0;
#else
	NETLINK_CB(skb).portid = 0;
#endif
	NETLINK_CB(skb).dst_group = 0;

	down(&adapter_priv.send_lock);
	ret = netlink_unicast(adapter_priv.sock, skb, adapter_priv.send_pid, 0);
	if (ret < 0)
		etf_printk(XRADIO_DBG_ERROR, "%s:netlink_unicast failed(%d),pid=%d!\n",
					__func__, ret, adapter_priv.send_pid);
	else
		etf_printk(XRADIO_DBG_NIY, "%s:netlink_unicast len(%d),pid=%d!\n",
					__func__, ret, adapter_priv.send_pid);
	up(&adapter_priv.send_lock);

	return ret;
}

static int xradio_adapter_send_pkg(void *data1, int len1, void *data2, int len2)
{
	int ret = 0;
	struct sk_buff  *skb = NULL;
	struct nlmsghdr *nlhdr = NULL;
	u8  *payload  = NULL;
	int total_len = len1 + len2;
	if (len1 <= 0 || len2 < 0 || NULL == data1)
		return -EINVAL;

	skb = xr_alloc_skb(NLMSG_SPACE(total_len + 1));
	if (!skb) {
		etf_printk(XRADIO_DBG_WARN, "%s:xr_alloc_skb failed!\n", __func__);
		return -ENOMEM;
	}
	/* '\0' safe for strcpy, compat for old etf apk, may be removed in furture.*/
	nlhdr = nlmsg_put(skb, 0, 0, 0, total_len + 1, 0);
	payload = (u8 *)NLMSG_DATA(nlhdr);
	memcpy(payload, data1, len1);
	if (data2 && len2)
		memcpy((payload + len1), data2, len2);
	payload[total_len] = '\0';

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 7, 0)
	NETLINK_CB(skb).pid = 0;
#else
	NETLINK_CB(skb).portid = 0;
#endif
	NETLINK_CB(skb).dst_group = 0;

	down(&adapter_priv.send_lock);
	ret = netlink_unicast(adapter_priv.sock, skb, adapter_priv.send_pid, 0);
	if (ret < 0)
		etf_printk(XRADIO_DBG_ERROR, "%s:netlink_unicast failed(%d),pid=%d!\n",
					__func__, ret, adapter_priv.send_pid);
	else
		etf_printk(XRADIO_DBG_NIY, "%s:netlink_unicast len(%d),pid=%d!\n",
					__func__, ret, adapter_priv.send_pid);
	up(&adapter_priv.send_lock);

	return ret;
}

struct xradio_adapter *xradio_adapter_init(msg_proc func)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0))
	struct netlink_kernel_cfg cfg = {
		.input	= adapter_rec_handler,
	};
#endif
	if (func == NULL) {
		etf_printk(XRADIO_DBG_ERROR, "%s msg_proc is NULL\n", __func__);
		return NULL;
	}
	adapter_priv.handler = func;
	sema_init(&adapter_priv.send_lock, 1);
	if (adapter_init_msgbuf()) {
		etf_printk(XRADIO_DBG_ERROR, "%s adapter_init_msgbuf failed\n", __func__);
		return NULL;
	}

#if (ETF_QUEUEMODE)
	sema_init(&adapter_priv.proc_sem, 0);
	if (adapter_start_thread()) {
		adapter_deinit_msgbuf();
		etf_printk(XRADIO_DBG_ERROR, "%s adapter_start_thread failed\n", __func__);
		return NULL;
	}
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 6, 0))
	adapter_priv.sock = netlink_kernel_create(&init_net, NL_FOR_XRADIO, 0,
			adapter_rec_handler, NULL, THIS_MODULE);
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(3, 7, 0))
	adapter_priv.sock = netlink_kernel_create(&init_net, NL_FOR_XRADIO,
			THIS_MODULE, &cfg);
#else
	adapter_priv.sock = netlink_kernel_create(&init_net, NL_FOR_XRADIO, &cfg);
#endif

	if (adapter_priv.sock == NULL) {
#if (ETF_QUEUEMODE)
		adapter_stop_thread();
#endif
		adapter_deinit_msgbuf();
		etf_printk(XRADIO_DBG_ERROR, "%s netlink_kernel_create failed\n", __func__);
		return NULL;
	}

	return &adapter_priv;
}

void xradio_adapter_deinit(void)
{
	if (adapter_priv.sock != NULL) {
		netlink_kernel_release(adapter_priv.sock);
		adapter_priv.sock = NULL;
	}
#if (ETF_QUEUEMODE)
	adapter_stop_thread();
#endif
	adapter_deinit_msgbuf();
}


/********************* interfaces of etf core **********************/
static struct xradio_etf etf_priv;
static struct etf_api_context context_save;

int wsm_etf_cmd(struct xradio_common *hw_priv, struct wsm_hdr *arg);
int xradio_bh_suspend(struct xradio_common *hw_priv);
int xradio_bh_resume(struct xradio_common *hw_priv);

void etf_adapter_version_init(struct xradio_etf *priv)
{
	priv->version = 0;
	priv->version |= ADAPTER_MAIN_VER << 24;
	priv->version |= ADAPTER_SUB_VER  << 16;
	priv->version |= ADAPTER_REV_VER  << 0;
}

static int  etf_check_version(u32 ver)
{
	u16 rev_ver  = (u16)((ver >> 0)  & 0xffff);
	u8  sub_ver  = (u8)((ver >> 16) & 0xff);
	u8  main_ver = (u8)((ver >> 24) & 0xff);

	etf_printk(XRADIO_DBG_NIY, "[%s] version: %d.%d.%d\n",
				    __func__, main_ver, sub_ver, rev_ver);
	if (main_ver >= 128 || sub_ver >= 200 || rev_ver >= 512)
		goto out;

	if (main_ver > REF_ETF_MAIN_VER) {
		return 1;
	} else if (main_ver == REF_ETF_MAIN_VER && sub_ver > REF_ETF_SUB_VER) {
		return 1;
	} else if (main_ver == REF_ETF_MAIN_VER && sub_ver >= REF_ETF_SUB_VER &&
				    rev_ver >= REF_ETF_REV_VER) {
		return 1;
	}

out:
	etf_printk(XRADIO_DBG_WARN, "*************************************\n");
	etf_printk(XRADIO_DBG_WARN, "     etf cli version is too low\n");
	etf_printk(XRADIO_DBG_WARN, "    %d.%d.%d or above is recommended.\n",
			    REF_ETF_MAIN_VER, REF_ETF_SUB_VER, REF_ETF_REV_VER);
	etf_printk(XRADIO_DBG_WARN, "*************************************\n");

	return 0;
}

static int etf_alloc_cli_buffer(u32 req_len, struct xradio_etf *priv)
{
	int ret = 0;

	if (req_len && req_len > priv->cli_data_len) {
		if (priv->cli_data) {
			kfree(priv->cli_data);
			priv->cli_data = NULL;
		}

		priv->cli_data = kmalloc(req_len, GFP_KERNEL);
		if (!priv->cli_data) {
			ret = EBUSY;
			etf_printk(XRADIO_DBG_ERROR,
			    "%s: can't allocate cli_data buffer!\n", __func__);
		}
		memset(priv->cli_data, 0, req_len);
		priv->cli_data_len = req_len;
		ret = 0;
	} else {
		if (req_len && req_len <= priv->cli_data_len)
			etf_printk(XRADIO_DBG_ALWY,
				"%s: cli_data buffer is ready.\n", __func__);
		else
			etf_printk(XRADIO_DBG_WARN,
				"%s: req_len = %d, priv->cli_data_len = %d.\n",
				__func__, req_len, priv->cli_data_len);
	}

	return ret;
}

static void etf_free_cli_buffer(struct xradio_etf *priv)
{
	if (priv->cli_data) {
		kfree(priv->cli_data);
		priv->cli_data = NULL;
		priv->cli_data_len = 0;
	}
}

bool etf_is_connect(void)
{
	return (bool)(etf_priv.etf_state != ETF_STAT_NULL);
}

void etf_set_core(void *core_priv)
{
	etf_priv.core_priv = core_priv;
}

const char *etf_get_fwpath(void)
{
	return (const char *)etf_priv.fw_path;
}

const char *etf_get_sddpath(void)
{
	return (const char *)etf_priv.sdd_path;
}

void etf_set_fwpath(const char *fw_path)
{
	etf_priv.fw_path = fw_path;
}

void etf_set_sddpath(const char *sdd_path)
{
	etf_priv.sdd_path = sdd_path;
}

u32 etf_set_freq_offset(u32 freq_offset)
{
	u8 dcxo_from_a_die = 0;
#if defined(CONFIG_DRIVER_V821)
	u32 val = REG_READ_U32(CCU_AON_DCXO_CFG);

	if (val & CCU_AON_DCXO_FLAG_BIT) {
		u8 enhance_rf_clk = 0x1;

		val &= ~(CCU_AON_DCXO_DIE_FREQ_OFFSET_MASK | CCU_AON_DCXO_ENHANCE_RFCLK_OUTV9_MASK);
		val |= (freq_offset << CCU_AON_DCXO_DIE_FREQ_OFFSET_SHIFT) & CCU_AON_DCXO_DIE_FREQ_OFFSET_MASK;
		val |= (enhance_rf_clk << CCU_AON_DCXO_ENHANCE_RFCLK_OUTV9_SHIFT) & CCU_AON_DCXO_ENHANCE_RFCLK_OUTV9_MASK;
		REG_WRITE_U32(CCU_AON_DCXO_CFG, val);
	} else {
		dcxo_from_a_die = 1;
	}
#endif

	return dcxo_from_a_die;
}

u32 etf_get_freq_offset(u32 *freq_offset)
{
	u8 dcxo_from_a_die = 0;
#if defined(CONFIG_DRIVER_V821)
	u32 val = REG_READ_U32(CCU_AON_DCXO_CFG);

	if (val & CCU_AON_DCXO_FLAG_BIT) {
		*freq_offset = (val & CCU_AON_DCXO_DIE_FREQ_OFFSET_MASK) >> CCU_AON_DCXO_DIE_FREQ_OFFSET_SHIFT;
	} else {
		dcxo_from_a_die = 1;
	}
#endif

	return dcxo_from_a_die;
}

static int etf_get_sdd_param(u8 *sdd, int sdd_len, u8 ies, void **data)
{
	int ie_len = 0;
	int parsedLength = 0;
	struct xradio_sdd *pElement = NULL;
	etf_printk(XRADIO_DBG_TRC, "%s\n", __func__);

	/*parse SDD config.*/
	pElement = (struct xradio_sdd *)sdd;

	while (parsedLength < sdd_len) {
		if (pElement->id == ies) {
			*data = (void *)pElement->data;
			ie_len = pElement->length;
		}
		parsedLength += (FIELD_OFFSET(struct xradio_sdd, data) + \
						pElement->length);
		pElement = FIND_NEXT_ELT(pElement);
	}

	xradio_dbg(XRADIO_DBG_MSG, "parse len=%d, ie_len=%d.\n",
				parsedLength, ie_len);
	return ie_len;
}

static int etf_driver_resp(int state, int result)
{
	struct drv_resp drv_ret;
	drv_ret.len    = sizeof(drv_ret);
	drv_ret.id     = ETF_DRIVER_IND_ID;
	drv_ret.state  = state;
	drv_ret.result = result;
	return xradio_adapter_send(&drv_ret, drv_ret.len);
}

#ifdef CONFIG_DRIVER_V821
//#define ETF_EFUSE_DBG

#define EFPG_DCXO_TRIM_FLAG_START       (1216)
#define EFPG_DCXO_TRIM_FLAG_NUM         (2)
#define EFPG_DCXO_TRIM_LEN              (8)
#define EFPG_DCXO_TRIM_START            (352)
#define EFPG_DCXO_TRIM_NUM              (EFPG_DCXO_TRIM_LEN)
#define EFPG_DCXO_TRIM1_START           (1218)
#define EFPG_DCXO_TRIM1_NUM             (EFPG_DCXO_TRIM_LEN)
#define EFPG_DCXO_TRIM2_START           (EFPG_DCXO_TRIM1_START + EFPG_DCXO_TRIM1_NUM)
#define EFPG_DCXO_TRIM2_NUM             (EFPG_DCXO_TRIM_LEN)

#define EFPG_WLAN_POUT_FLAG_START       (1234)
#define EFPG_WLAN_POUT_FLAG_NUM         (2)
#define EFPG_WLAN_POUT_LEN              (7)
#define EFPG_WLAN_POUT1_START           (1236)
#define EFPG_WLAN_POUT1_NUM             (EFPG_WLAN_POUT_LEN * 3)
#define EFPG_WLAN_POUT2_START           (EFPG_WLAN_POUT1_START + EFPG_WLAN_POUT1_NUM)
#define EFPG_WLAN_POUT2_NUM             (EFPG_WLAN_POUT_LEN * 3)

#define EFPG_MAC_WLAN_ADDR_NUM          (2)
#define EFPG_MAC_WLAN_ADDR_FLAG_START   (510)
#define EFPG_MAC_WLAN_ADDR_FLAG_NUM     (EFPG_MAC_WLAN_ADDR_NUM)
#define EFPG_MAC_WLAN_ADDR_LEN          (48)
#if (EFPG_MAC_WLAN_ADDR_NUM >= 1)
#define EFPG_MAC_WLAN_ADDR1_START       (EFPG_MAC_WLAN_ADDR_FLAG_START + EFPG_MAC_WLAN_ADDR_FLAG_NUM)
#define EFPG_MAC_WLAN_ADDR1_NUM         (EFPG_MAC_WLAN_ADDR_LEN)
#endif
#if (EFPG_MAC_WLAN_ADDR_NUM >= 2)
#define EFPG_MAC_WLAN_ADDR2_START       (EFPG_MAC_WLAN_ADDR1_START + EFPG_MAC_WLAN_ADDR1_NUM)
#define EFPG_MAC_WLAN_ADDR2_NUM         (EFPG_MAC_WLAN_ADDR_LEN)
#endif

#define EFPG_WLAN_OEM_START             (1216)
#define EFPG_WLAN_OEM_LEN               (448)

#define EFUSE_WMAC_SEL_FLAG_NAME        "wmac_sel"
#define EFUSE_WMAC_ADDR1_NAME           "wmac_addr1"
#define EFUSE_WMAC_ADDR2_NAME           "wmac_addr2"

#define EFUSE_TEMP_DATA_BYTE  EFPG_BITS_TO_BYTE_CNT(EFPG_WLAN_OEM_LEN)
#define EFPG_BITS_TO_BYTE_CNT(bits)        (((bits) + 7) / 8)

typedef struct efpg_region_info {
	u16 flag_start;       /* bit */
	u16 flag_bits;        /* MUST less than 32-bit */
	u16 data_start;       /* bit */
	u16 data_bits;
	u8  *buf;             /* temp buffer for write to save read back data */
	u8  buf_len;          /* byte, MUST equal to ((data_bits + 7) / 8) */
	// key info
	u16 key_start;       /* bit */
	u16 key_len;         /* byte */
	char *key_name;
	u16 key_flag_start;  /* bit */
	u16 key_flag_len;    /* byte */
	char *key_flag_name;
} efpg_region_info_t;

struct etf_efuse_data {
	sunxi_efuse_key_info_t temp_key;
	u8 temp_data[EFUSE_TEMP_DATA_BYTE];
};

static struct etf_efuse_data *efuse_data_info;
static void __iomem *efuse_addr;
static u8 efuse_area_in_use;

static int etf_call_efuse_write(char *name, u8 *data, u8 len)
{
	int ret;

	if ((sizeof(name) > sizeof(efuse_data_info->temp_key.name)) || (len > sizeof(efuse_data_info->temp_data)))
		return -EINVAL;

	//strncpy(efuse_data_info->temp_key.name, name, sizeof(efuse_data_info->temp_key.name));
	snprintf(efuse_data_info->temp_key.name, sizeof(efuse_data_info->temp_key.name), "%s", name);
	efuse_data_info->temp_key.len = len;
	efuse_data_info->temp_key.offset = 0;
	memcpy(efuse_data_info->temp_data, data, len);
	efuse_data_info->temp_key.key_data = virt_to_phys((void *)efuse_data_info->temp_data);

#ifdef ETF_EFUSE_DBG
	etf_printk(XRADIO_DBG_ALWY, "%s %d the virt addr is 0x%x\n", __func__, __LINE__,
		(u32)&efuse_data_info->temp_key);
	etf_printk(XRADIO_DBG_ALWY, "%s %d the phy  addr is 0x%lx\n", __func__, __LINE__,
		(unsigned long)virt_to_phys(&efuse_data_info->temp_key));
	etf_printk(XRADIO_DBG_ALWY, "key name %s %s len %d\n", name,
		efuse_data_info->temp_key.name, efuse_data_info->temp_key.len);
	print_hex_dump(KERN_ERR, "data: ", DUMP_PREFIX_OFFSET, 16, 1,
		efuse_data_info->temp_data, efuse_data_info->temp_key.len, 0);
#endif

	ret = sbi_efuse_write(virt_to_phys((volatile void *)&efuse_data_info->temp_key));
#ifdef ETF_EFUSE_DBG
	etf_printk(XRADIO_DBG_ALWY, "%s after write dump all efuse\n", __func__);
	print_hex_dump(KERN_ERR, "efsue: ", DUMP_PREFIX_OFFSET, 16, 1, efuse_addr, 256, 0);
#endif

	return ret;
}

#if 0
static int etf_call_efuse_read(char *name, u8 *buf, u8 len)
{
	return sunxi_efuse_readn(name, buf, len);
}
#endif

static __inline void xradio_efuse_sram_read(u8 index, u32 *pData)
{
	u32 *VALUE = efuse_addr;

	*pData = VALUE[index];
}

static int xradio_efuse_read(u32 start_bit, u32 bit_num, u8 *data)
{
	u32 start_word = start_bit / 32;
	u8 word_shift = start_bit % 32;
	u8 word_cnt = (start_bit + bit_num) / 32 - start_word;
	u32 *sid_data;
	int i;

	if ((start_bit + bit_num) % 32)
		word_cnt++;

	if (!efuse_addr) {
		etf_printk(XRADIO_DBG_WARN, "xradio_efuse_read: base addr is 0.\n");
		return -1;
	}

	sid_data = kzalloc(word_cnt * 4, GFP_KERNEL);
	if (sid_data == NULL)
		return -1;

	for (i = 0; i < word_cnt; i++) {
		xradio_efuse_sram_read(start_word + i, sid_data + i);
	}
	if (word_shift) {
		for (i = 0; i < (word_cnt - 1); i++) {
			sid_data[i] = (sid_data[i] >> word_shift) | (sid_data[i + 1] << (32 - word_shift));
		}
		sid_data[i] = (sid_data[i] >> word_shift);
	}
	((u8 *)sid_data)[(bit_num - 1) / 8] &= ((1 << (bit_num % 8 == 0 ? 8 : bit_num % 8)) - 1);

	memcpy(data, (u8 *)sid_data, (bit_num + 7) / 8);
	kfree(sid_data);

	return 0;
}

#if 0
static u16 xradio_efuse_read_region(efpg_region_info_t *info, u8 *data)
{
	u8 idx = 0;
	u8 flag;
	u32 start_bit;

	/* flag */
	if (xradio_efuse_read(info->flag_start, info->flag_bits, (u8 *)&flag)) {
		return EFUSE_STATUS_CODE__RW_ERR;
	}
	flag &= ((1 << info->flag_bits) - 1);
	etf_printk(XRADIO_DBG_ALWY, "r start %d, bits %d, flag 0x%x\n", info->flag_start, info->flag_bits, flag);
	if ((flag == 0) || (flag > ((1 << info->flag_bits) - 1))) {
		etf_printk(XRADIO_DBG_WARN, "%s(), flag (%d, %d) = 0x%x is invalid\n", __func__,
			info->flag_start, info->flag_bits, flag);
		return EFUSE_STATUS_CODE__NODATA_ERR;
	}
	while ((flag & 0x1) != 0) {
		flag = flag >> 1;
		idx++;
	}
	start_bit = info->data_start + (idx - 1) * info->data_bits;

	/* data */
	etf_printk(XRADIO_DBG_ALWY, "r data, start %d, bits %d\n", start_bit, info->data_bits);
	if (xradio_efuse_read(start_bit, info->data_bits, data)) {
		return EFUSE_STATUS_CODE__RW_ERR;
	}

	return EFUSE_STATUS_CODE__SUCCESS;
}
#endif

static u16 xradio_efuse_write(u32 start_bit, u32 bit_num, u8 *data, char *key_name, u8 key_len)
{
	u8 *p_data = data;
	u32 bit_shift = start_bit & (32 - 1);
	u32 word_idx = start_bit >> 5;

	u64 buf = 0;
	u32 *efuse_word = (u32 *)&buf;
	u32 bit_cnt = bit_num;
	u32 *efuse = (void *)efuse_data_info->temp_data;
	int ret;

	if ((data == NULL) || (bit_num == 0) || (bit_num > EFUSE_TEMP_DATA_BYTE) ||
		(EFPG_BITS_TO_BYTE_CNT(start_bit + bit_num) > EFUSE_TEMP_DATA_BYTE)) {
		etf_printk(XRADIO_DBG_ERROR, "%s check fail: start bit %u, bit num %u, data %p\n", __func__,
			start_bit, bit_num, data);
		return EFUSE_STATUS_CODE__RW_ERR;
	}

#ifdef ETF_EFUSE_DBG
	etf_printk(XRADIO_DBG_ALWY, "pre change key %s len %d:\n", key_name, key_len);
	print_hex_dump(KERN_ERR, "efsue: ", DUMP_PREFIX_OFFSET, 16, 1, efuse, key_len, 0);
#endif

	data[(bit_num - 1) / 8] &= ((1 << (bit_num % 8 == 0 ? 8 : bit_num % 8)) - 1);
	memcpy(&efuse_word[1], p_data, sizeof(efuse_word[1]));
	if (bit_cnt < 32)
		efuse_word[1] &= (1U << bit_cnt) - 1;
	efuse_word[1] = efuse_word[1] << bit_shift;

	efuse[word_idx] |= efuse_word[1];

	word_idx++;
	bit_cnt -= (bit_cnt <= 32 - bit_shift) ? bit_cnt : 32 - bit_shift;

	while (bit_cnt > 0) {
		memcpy(&buf, p_data, sizeof(buf));
		buf = buf << bit_shift;
		if (bit_cnt < 32)
			efuse_word[1] &= (1U << bit_cnt) - 1;

		efuse[word_idx] |= efuse_word[1];

		word_idx++;
		p_data += 4;
		bit_cnt -= (bit_cnt <= 32) ? bit_cnt : 32;
	}

#ifdef ETF_EFUSE_DBG
	etf_printk(XRADIO_DBG_ALWY, "pre write key %s len %d:\n", key_name, key_len);
	print_hex_dump(KERN_ERR, "key  : ", DUMP_PREFIX_OFFSET, 16, 1, efuse, key_len, 0);
#endif

	ret = etf_call_efuse_write(key_name, (u8 *)efuse, key_len);
	if (ret) {
		etf_printk(XRADIO_DBG_ERROR, "etf_call_efuse_write key %s len %d fail %d\n",
			key_name, key_len, ret);
		return EFUSE_STATUS_CODE__RW_ERR;
	}

	return EFUSE_STATUS_CODE__SUCCESS;
}

static u16 xradio_efuse_write_region(efpg_region_info_t *info, u8 *data)
{
	u8 idx = 0;
	u8 flag;
	u32 start_bit;
	u8 w_tmp;

	if ((info->flag_start < info->key_flag_start) ||
		(info->data_start < info->key_start)) {
		etf_printk(XRADIO_DBG_ERROR, "check fail: flag:start %u key start %u, data:start %u key start %u\n",
			info->flag_start, info->key_flag_start, info->data_start, info->key_start);
		return EFUSE_STATUS_CODE__INVALID_PARAMS;
	}
	if (info->key_len > sizeof(efuse_data_info->temp_data)) {
		etf_printk(XRADIO_DBG_ERROR, "check fail: key len %u, buf size %u\n",
			info->key_len, sizeof(efuse_data_info->temp_data));
		return EFUSE_STATUS_CODE__INVALID_PARAMS;
	}

	/* read flag */
	if (xradio_efuse_read(info->flag_start, info->flag_bits, (u8 *)&flag)) {
		return EFUSE_STATUS_CODE__RW_ERR;
	}
	flag &= ((1 << info->flag_bits) - 1);
	etf_printk(XRADIO_DBG_NIY, "w start %d, bits %d, flag 0x%x\n", info->flag_start, info->flag_bits, flag);
	memset(info->buf, 0, info->buf_len);
	memset(efuse_data_info->temp_data, 0, sizeof(efuse_data_info->temp_data));

	if (flag < (1 << info->flag_bits)) {
		if (flag == 0) {
			idx = 0;
			w_tmp = 1 << idx;

			if (xradio_efuse_read(info->key_flag_start, info->key_flag_len * 8, (u8 *)efuse_data_info->temp_data)) {
				return EFUSE_STATUS_CODE__RW_ERR;
			}
			if ((xradio_efuse_write(info->flag_start - info->key_flag_start, info->flag_bits,
				&w_tmp, info->key_flag_name, info->key_flag_len))) {
				return EFUSE_STATUS_CODE__RW_ERR;
			}
		} else {
			while (flag != 0) {
				flag = flag >> 1;
				idx++;
			}
			idx--;
		}

		etf_printk(XRADIO_DBG_NIY, "idx:%d flag_bits:%d\n", idx, info->flag_bits);

		while (idx < info->flag_bits) {
			etf_printk(XRADIO_DBG_NIY, "idx:%d\n", idx);
			/* will try compare old data with new */
			start_bit = info->data_start + idx * info->data_bits;
			memset(info->buf, 0, info->buf_len);
			if (xradio_efuse_read(start_bit, info->data_bits, info->buf) == 0) {
				int i;

				if (info->data_bits % 8) {
					info->buf[info->buf_len - 1] &= ((1 << (info->data_bits % 8)) - 1);
				}
				for (i = 0; i < info->buf_len; i++) {
					if (((info->buf[i] ^ data[i]) & info->buf[i]) != 0) {
						etf_printk(XRADIO_DBG_NIY, "compare fail i:%d data %x %x\n", i, info->buf[i], data[i]);
						break;
					}
				}
				if (i == info->buf_len) {
					memset(info->buf, 0, info->buf_len);
					etf_printk(XRADIO_DBG_NIY, "w start %d, bits %d\n", start_bit, info->data_bits);
					if (xradio_efuse_read(info->key_start, info->key_len, (u8 *)efuse_data_info->temp_data)) {
						return EFUSE_STATUS_CODE__RW_ERR;
					}
					if ((xradio_efuse_write(start_bit - info->key_start, info->data_bits, data,
						info->key_name, info->key_len) == 0)) {
						return EFUSE_STATUS_CODE__SUCCESS;
					}
				}
			}

			if (++idx > info->flag_bits - 1) {
				etf_printk(XRADIO_DBG_WARN, "region has no space\n");
				return EFUSE_STATUS_CODE__DI_ERR;
			}
			/* update flag as next */
			w_tmp = 1 << idx;
			memset(info->buf, 0, info->buf_len);
			etf_printk(XRADIO_DBG_NIY, "w start %d, bits %d, w_tmp 0x%x\n", info->flag_start, info->flag_bits, w_tmp);
			if (xradio_efuse_read(info->key_flag_start, info->key_flag_len, (u8 *)efuse_data_info->temp_data)) {
				return EFUSE_STATUS_CODE__RW_ERR;
			}
			if ((xradio_efuse_write(info->flag_start - info->key_flag_start, info->flag_bits,
				&w_tmp, info->key_flag_name, info->key_flag_len))) {
				etf_printk(XRADIO_DBG_WARN, "fail to write flag\n");
				return EFUSE_STATUS_CODE__RW_ERR;
			}
		}
	}

	etf_printk(XRADIO_DBG_WARN, "flag (%d, %d) = 0x%x is invalid, idx:%d\n", info->flag_start, info->flag_bits, flag, idx);
	return EFUSE_STATUS_CODE__NODATA_ERR;
}

static int xradio_rw_efuse_init(void)
{
	int ret = 1;
	struct device_node *dev_node = NULL;
	struct resource efuse_ram_res = {0};

	efuse_addr = NULL;
	efuse_data_info = kzalloc(sizeof(struct etf_efuse_data), GFP_KERNEL);
	if (!efuse_data_info) {
		etf_printk(XRADIO_DBG_ERROR, "no mem.\n");
		return -ENOMEM;
	}

	dev_node = of_find_compatible_node(NULL, NULL, EFUSE_SID_BASE);
	if (IS_ERR_OR_NULL(dev_node)) {
		etf_printk(XRADIO_DBG_ERROR, "Failed to find \"%s\" in dts.\n", EFUSE_SID_BASE);
		kfree(efuse_data_info);
		efuse_data_info = NULL;
		return -ENOMEM;
	}
	/* get efuse_ram_reserved from dts for efuse sram read */
	efuse_ram_res.start = 0;
	dev_node = of_parse_phandle(dev_node, "memory-region", 0);
	if (dev_node) {
		ret = of_address_to_resource(dev_node, 0, &efuse_ram_res);
	}
	if (efuse_ram_res.start) {
		void __iomem *start;

		start = ioremap(efuse_ram_res.start, resource_size(&efuse_ram_res));
		if (IS_ERR_OR_NULL(start))
			return -ENOMEM;
		efuse_addr = start;
#ifdef ETF_EFUSE_DBG
		etf_printk(XRADIO_DBG_ALWY, "%s dump efuse, addr 0x%x\n", __func__, (u32)efuse_ram_res.start);
		print_hex_dump(KERN_ERR, "efsue: ", DUMP_PREFIX_OFFSET, 16, 1, start, 256, 0);
#endif
		return 0;
	}
	etf_printk(XRADIO_DBG_ERROR, "Failed to get efuse %d.\n", ret);
	kfree(efuse_data_info);
	efuse_data_info = NULL;

	return ret;
}

static void xradio_rw_efuse_deinit(void)
{
	if (efuse_addr) {
		iounmap(efuse_addr);
		efuse_addr = NULL;
	}
	if (efuse_data_info) {
		kfree(efuse_data_info);
		efuse_data_info = NULL;
	}
}

static u16 xradio_efuse_read_dcxo(u8 *freq_offset)
{
	u8 flag, data, idx = 0;
	u32 start_bit;

	/* flag */
	if (xradio_efuse_read(EFPG_DCXO_TRIM_FLAG_START, EFPG_DCXO_TRIM_FLAG_NUM, (u8 *)&flag)) {
		return EFUSE_STATUS_CODE__RW_ERR;
	}
	flag &= ((1 << EFPG_DCXO_TRIM_FLAG_NUM) - 1);
	etf_printk(XRADIO_DBG_NIY, "r start %d, bits %d, flag 0x%x\n", EFPG_DCXO_TRIM_FLAG_START, EFPG_DCXO_TRIM_FLAG_NUM, flag);

	if (flag == 0)
		return EFUSE_STATUS_CODE__NODATA_ERR;
	else if (flag == 1)
		start_bit = EFPG_DCXO_TRIM1_START;
	else if (flag == 3)
		start_bit = EFPG_DCXO_TRIM2_START;
	else {
		etf_printk(XRADIO_DBG_ERROR, "%s(), flag (%d, %d) = 0x%x is invalid\n",
			__func__, EFPG_DCXO_TRIM_FLAG_START, EFPG_DCXO_TRIM_FLAG_NUM, flag);
		return EFUSE_STATUS_CODE__NOTMATCH_ERR;
	}

	while (flag) {
		flag = flag >> 1;
		idx++;
	}
	efuse_area_in_use = idx;
	etf_printk(XRADIO_DBG_NIY, "flag 0x%x idx:%d\n", flag, idx);

	/* data */
	data = 0;
	etf_printk(XRADIO_DBG_NIY, "r data, start %d, bits %d\n", start_bit, EFPG_DCXO_TRIM_LEN);
	if (xradio_efuse_read(start_bit, EFPG_DCXO_TRIM_LEN, &data)) {
		return EFUSE_STATUS_CODE__RW_ERR;
	}
	etf_printk(XRADIO_DBG_NIY, "efuse dcxo:%d\n", data);
	if (data == 0)
		return EFUSE_STATUS_CODE__NODATA_ERR;
	*freq_offset = data;

	return EFUSE_STATUS_CODE__SUCCESS;
}

static u16 xradio_efuse_write_dcxo(u8 *data)
{
	efpg_region_info_t info;
	u8 buf[EFPG_BITS_TO_BYTE_CNT(EFPG_DCXO_TRIM_LEN)] = {0};
	char key_name[] = {EFUSE_OEM_NAME};

	info.flag_start = EFPG_DCXO_TRIM_FLAG_START;
	info.flag_bits = EFPG_DCXO_TRIM_FLAG_NUM;
	info.data_start = EFPG_DCXO_TRIM1_START;
	info.data_bits = EFPG_DCXO_TRIM1_NUM;
	info.buf = buf;
	info.buf_len = sizeof(buf);

	info.key_start = EFPG_WLAN_OEM_START;
	info.key_len = EFPG_BITS_TO_BYTE_CNT(EFPG_WLAN_OEM_LEN);
	info.key_name = key_name;

	info.key_flag_start = EFPG_WLAN_OEM_START;
	info.key_flag_len = EFPG_BITS_TO_BYTE_CNT(EFPG_WLAN_OEM_LEN);
	info.key_flag_name = key_name;

	etf_printk(XRADIO_DBG_NIY, "%s Flag: key name %s len %d start %d\n", __func__,
		info.key_flag_name, info.key_flag_len, info.key_flag_start);
	etf_printk(XRADIO_DBG_NIY, "%s Data: key name %s len %d start %d\n", __func__,
		info.key_name, info.key_len, info.key_start);
	etf_printk(XRADIO_DBG_NIY, "%s Data: %d\n", __func__, data[0]);

	return xradio_efuse_write_region(&info, data);
}

static u16 xradio_efuse_read_wlan_pout(u8 *pout)
{
	u8 flag, data[3] = {0}, idx = 0;
	u32 start_bit;

	/* flag */
	if (xradio_efuse_read(EFPG_WLAN_POUT_FLAG_START, EFPG_WLAN_POUT_FLAG_NUM, (u8 *)&flag)) {
		return EFUSE_STATUS_CODE__RW_ERR;
	}
	flag &= ((1 << EFPG_WLAN_POUT_FLAG_NUM) - 1);
	etf_printk(XRADIO_DBG_MSG, "r start %d, bits %d, flag 0x%x\n", EFPG_WLAN_POUT_FLAG_START, EFPG_WLAN_POUT_FLAG_NUM, flag);

	if (flag == 0) {
		return EFUSE_STATUS_CODE__NODATA_ERR;
	} else if (flag == 1) {
		start_bit = EFPG_WLAN_POUT1_START;
	} else if (flag == 3) {
		start_bit = EFPG_WLAN_POUT2_START;
	} else {
		etf_printk(XRADIO_DBG_ERROR, "%s(), flag (%d, %d) = 0x%x is invalid\n",
		      __func__, EFPG_WLAN_POUT_FLAG_START, EFPG_WLAN_POUT_FLAG_NUM, flag);
		return EFUSE_STATUS_CODE__NODATA_ERR;
	}

	while (flag) {
		flag = flag >> 1;
		idx++;
	}
	efuse_area_in_use = idx;
	etf_printk(XRADIO_DBG_NIY, "flag 0x%x idx:%d\n", flag, idx);

	/* data */
	etf_printk(XRADIO_DBG_NIY, "r data, start %d, bits %d\n", start_bit, EFPG_WLAN_POUT1_NUM);
	if (xradio_efuse_read(start_bit, EFPG_WLAN_POUT1_NUM, data)) {
		return EFUSE_STATUS_CODE__RW_ERR;
	}
	etf_printk(XRADIO_DBG_NIY, "efuse wlan pout:0x%x 0x%x 0x%x\n", data[0], data[1], data[2]);

	pout[0] = data[0];
	pout[1] = data[1];
	pout[2] = data[2];

	return EFUSE_STATUS_CODE__SUCCESS;
}


static u16 xradio_efuse_write_wlan_pout(u8 *data)
{
	efpg_region_info_t info;
	u8 buf[EFPG_BITS_TO_BYTE_CNT(EFPG_WLAN_POUT1_NUM)] = {0};
	char key_name[] = {EFUSE_OEM_NAME};

	info.flag_start = EFPG_WLAN_POUT_FLAG_START;
	info.flag_bits = EFPG_WLAN_POUT_FLAG_NUM;
	info.data_start = EFPG_WLAN_POUT1_START;
	info.data_bits = EFPG_WLAN_POUT1_NUM;
	info.buf = buf;
	info.buf_len = sizeof(buf);

	info.key_start = EFPG_WLAN_OEM_START;
	info.key_len = EFPG_BITS_TO_BYTE_CNT(EFPG_WLAN_OEM_LEN);
	info.key_name = key_name;

	info.key_flag_start = EFPG_WLAN_OEM_START;
	info.key_flag_len = EFPG_BITS_TO_BYTE_CNT(EFPG_WLAN_OEM_LEN);
	info.key_flag_name = key_name;

	etf_printk(XRADIO_DBG_NIY, "%s Flag: key name %s len %d start %d\n", __func__,
		info.key_flag_name, info.key_flag_len, info.key_flag_start);
	etf_printk(XRADIO_DBG_NIY, "%s Data: key name %s len %d start %d\n", __func__,
		info.key_name, info.key_len, info.key_start);
	etf_printk(XRADIO_DBG_NIY, "%s Data: %x %x %x\n", __func__,
		data[0], data[1], data[2]);

	return xradio_efuse_write_region(&info, data);
}

static u16 xradio_efuse_read_wlan_mac(u8 *buf)
{
	u8 flag, wmac1[6] = {0}, idx = 0;
	u32 start_bit;

	/* flag */
	if (xradio_efuse_read(EFPG_MAC_WLAN_ADDR_FLAG_START, EFPG_MAC_WLAN_ADDR_FLAG_NUM, (u8 *)&flag)) {
		return EFUSE_STATUS_CODE__RW_ERR;
	}
	flag &= ((1 << EFPG_MAC_WLAN_ADDR_FLAG_NUM) - 1);
	etf_printk(XRADIO_DBG_NIY, "r start %d, bits %d, flag 0x%x\n", EFPG_MAC_WLAN_ADDR_FLAG_START, EFPG_MAC_WLAN_ADDR_FLAG_NUM, flag);

	while (flag) {
		flag = flag >> 1;
		idx++;
	}
	efuse_area_in_use = idx;
	etf_printk(XRADIO_DBG_NIY, "flag 0x%x idx:%d\n", flag, idx);

	if (flag == 0) {
		u8 wmac2[6] = {0};

		start_bit = EFPG_MAC_WLAN_ADDR1_START;
		/* data */
		etf_printk(XRADIO_DBG_NIY, "r data, start %d, bits %d\n", start_bit, EFPG_MAC_WLAN_ADDR1_NUM);
		if (xradio_efuse_read(start_bit, EFPG_MAC_WLAN_ADDR1_NUM, wmac1)) {
			return EFUSE_STATUS_CODE__RW_ERR;
		}

		start_bit = EFPG_MAC_WLAN_ADDR2_START;
		/* data */
		etf_printk(XRADIO_DBG_NIY, "r data, start %d, bits %d\n", start_bit, EFPG_MAC_WLAN_ADDR1_NUM);
		if (xradio_efuse_read(start_bit, EFPG_MAC_WLAN_ADDR1_NUM, wmac2)) {
			return EFUSE_STATUS_CODE__RW_ERR;
		}

		if ((wmac2[0] == 0) && (wmac2[1] == 0) && (wmac2[2] == 0) && (wmac2[3] == 0) &&
			(wmac2[4] == 0) && (wmac2[5] == 0)) {

			if ((wmac1[0] == 0) && (wmac1[1] == 0) && (wmac1[2] == 0) && (wmac1[3] == 0) &&
				(wmac1[4] == 0) && (wmac1[5] == 0)) {
				return EFUSE_STATUS_CODE__NODATA_ERR;
			} else {
				// sel MAC addr1
				memcpy(buf, wmac1, sizeof(wmac1));
				etf_printk(XRADIO_DBG_NIY, "%02x:%02x:%02x:%02x:%02x:%02x", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);

				return EFUSE_STATUS_CODE__SUCCESS;
			}
		} else {
			// sel MAC addr2
			memcpy(buf, wmac2, sizeof(wmac2));
			etf_printk(XRADIO_DBG_NIY, "%02x:%02x:%02x:%02x:%02x:%02x", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);

			return EFUSE_STATUS_CODE__SUCCESS;
		}

	} else if (flag == 1) {
		start_bit = EFPG_MAC_WLAN_ADDR1_START;
	} else if (flag & 0x2) {
		start_bit = EFPG_MAC_WLAN_ADDR2_START;
	} else {
		etf_printk(XRADIO_DBG_ERROR, "%s(), flag (%d, %d) = 0x%x is invalid\n",
			__func__, EFPG_MAC_WLAN_ADDR_FLAG_START, EFPG_MAC_WLAN_ADDR_FLAG_NUM, flag);
		return EFUSE_STATUS_CODE__NODATA_ERR;
	}

	/* data */
	etf_printk(XRADIO_DBG_NIY, "r data, start %d, bits %d\n", start_bit, EFPG_MAC_WLAN_ADDR1_NUM);
	if (xradio_efuse_read(start_bit, EFPG_MAC_WLAN_ADDR1_NUM, wmac1)) {
		return EFUSE_STATUS_CODE__RW_ERR;
	}

	if ((wmac1[0] == 0) && (wmac1[1] == 0) && (wmac1[2] == 0) && (wmac1[3] == 0) &&
		(wmac1[4] == 0) && (wmac1[5] == 0)) {
		return EFUSE_STATUS_CODE__NODATA_ERR;
	} else {
		memcpy(buf, wmac1, sizeof(wmac1));
		etf_printk(XRADIO_DBG_NIY, "%02x:%02x:%02x:%02x:%02x:%02x", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
		return EFUSE_STATUS_CODE__SUCCESS;
	}
}

static u16 xradio_efuse_write_mac_region(efpg_region_info_t *info, u8 *data)
{
	u8 idx = 0;
	u8 flag;
	u32 start_bit;
	u8 w_tmp;
	int ret;

	if ((info->flag_start < info->key_flag_start) ||
		(info->data_start < info->key_start)) {
		etf_printk(XRADIO_DBG_ERROR, "flag:start %u key start %u, data:start %u key start %u\n",
			info->flag_start, info->key_flag_start, info->data_start, info->key_start);
		return EFUSE_STATUS_CODE__INVALID_PARAMS;
	}
	if (info->key_len > sizeof(efuse_data_info->temp_data)) {
		etf_printk(XRADIO_DBG_ERROR, "key len %u, buf size %u\n",
			info->key_len, sizeof(efuse_data_info->temp_data));
		return EFUSE_STATUS_CODE__INVALID_PARAMS;
	}

	/* read flag */
	if (xradio_efuse_read(info->flag_start, info->flag_bits, (u8 *)&flag)) {
		return EFUSE_STATUS_CODE__RW_ERR;
	}
	flag &= ((1 << info->flag_bits) - 1);
	etf_printk(XRADIO_DBG_NIY, "w start %d, bits %d, flag 0x%x\n", info->flag_start, info->flag_bits, flag);
	memset(info->buf, 0, info->buf_len);
	memset(efuse_data_info->temp_data, 0, sizeof(efuse_data_info->temp_data));

	if (flag < (1 << info->flag_bits)) {
		if (flag == 0) {
			idx = 0;
			w_tmp = 1 << idx;

			if (xradio_efuse_read(info->key_flag_start, info->key_flag_len * 8, (u8 *)efuse_data_info->temp_data)) {
				return EFUSE_STATUS_CODE__RW_ERR;
			}
			if ((xradio_efuse_write(info->flag_start - info->key_flag_start, info->flag_bits,
				&w_tmp, info->key_flag_name, info->key_flag_len))) {
				return EFUSE_STATUS_CODE__RW_ERR;
			}
		} else {
			while (flag != 0) { //while ((flag & 0x1) != 0) {
				flag = flag >> 1;
				idx++;
			}
			idx--;
		}

		etf_printk(XRADIO_DBG_NIY, "idx:%d flag_bits:%d\n", idx, info->flag_bits);

		while (idx < info->flag_bits) {
			etf_printk(XRADIO_DBG_NIY, "idx:%d\n", idx);
			/* will try compare old data with new */
			start_bit = info->data_start + idx * info->data_bits;
			memset(info->buf, 0, info->buf_len);
			ret = xradio_efuse_read(start_bit, info->data_bits, info->buf);
			if (ret == 0) {
				int i;

				if (info->data_bits % 8) {
					info->buf[info->buf_len - 1] &= ((1 << (info->data_bits % 8)) - 1);
				}
				for (i = 0; i < info->buf_len; i++) {
					if (((info->buf[i] ^ data[i]) & info->buf[i]) != 0) {
						etf_printk(XRADIO_DBG_NIY, "compare fail i:%d data %x %x\n", i, info->buf[i], data[i]);
						break;
					}
				}
				if (i == info->buf_len) {
					memset(info->buf, 0, info->buf_len);
					if (idx == 1) {
						char key_name_2[] = {EFUSE_WMAC_ADDR2_NAME};

						// update to MAC addr 2 key
						info->key_start = EFPG_MAC_WLAN_ADDR2_START;
						info->key_len = EFPG_BITS_TO_BYTE_CNT(EFPG_MAC_WLAN_ADDR2_NUM);
						info->key_name = key_name_2;
						etf_printk(XRADIO_DBG_NIY, "MAC region update to addr 2, now idx %d\n", idx);
					} else if (idx > 1) {
						etf_printk(XRADIO_DBG_ERROR, "MAC max region is 2, now idx %d\n", idx);
						return EFUSE_STATUS_CODE__DI_ERR;
					}

					etf_printk(XRADIO_DBG_NIY, "w start %d, bits %d\n", start_bit, info->data_bits);
					if (xradio_efuse_read(info->key_start, info->key_len, (u8 *)efuse_data_info->temp_data)) {
						return EFUSE_STATUS_CODE__RW_ERR;
					}
					if ((xradio_efuse_write(start_bit - info->key_start, info->data_bits,
						data, info->key_name, info->key_len) == 0)) {
						return EFUSE_STATUS_CODE__SUCCESS;
					}
				}
			} else {
				return EFUSE_STATUS_CODE__RW_ERR;
			}

			if (++idx > info->flag_bits - 1) {
				etf_printk(XRADIO_DBG_WARN, "region has no space\n");
				return EFUSE_STATUS_CODE__DI_ERR;
			}
			/* update flag as next */
			w_tmp = 1 << idx;
			memset(info->buf, 0, info->buf_len);
			etf_printk(XRADIO_DBG_NIY, "w start %d, bits %d, w_tmp 0x%x\n", info->flag_start, info->flag_bits, w_tmp);
			if (xradio_efuse_read(info->key_flag_start, info->key_flag_len, (u8 *)efuse_data_info->temp_data)) {
				return EFUSE_STATUS_CODE__RW_ERR;
			}
			if ((xradio_efuse_write(info->flag_start - info->key_flag_start, info->flag_bits,
				&w_tmp, info->key_flag_name, info->key_flag_len))) {
				etf_printk(XRADIO_DBG_WARN, "fail to write flag\n");
				return EFUSE_STATUS_CODE__RW_ERR;
			}
		}
	}

	etf_printk(XRADIO_DBG_WARN, "flag (%d, %d) = 0x%x is invalid, idx:%d\n", info->flag_start, info->flag_bits, flag, idx);
	return EFUSE_STATUS_CODE__NODATA_ERR;
}

u16 xradio_efuse_write_wlan_mac(u8 *data)
{
	efpg_region_info_t info;
	int ret;
	u8 buf[EFPG_BITS_TO_BYTE_CNT(EFPG_MAC_WLAN_ADDR1_NUM)] = {0};
	char key_name_1[] = {EFUSE_WMAC_ADDR1_NAME};
	char key_flag_name[] = {EFUSE_WMAC_SEL_FLAG_NAME};

	info.flag_start = EFPG_MAC_WLAN_ADDR_FLAG_START;
	info.flag_bits = EFPG_MAC_WLAN_ADDR_FLAG_NUM;
	info.data_start = EFPG_MAC_WLAN_ADDR1_START;
	info.data_bits = EFPG_MAC_WLAN_ADDR1_NUM;
	info.buf = buf;
	info.buf_len = sizeof(buf);
	info.key_start = EFPG_MAC_WLAN_ADDR1_START;
	info.key_len = EFPG_BITS_TO_BYTE_CNT(EFPG_MAC_WLAN_ADDR1_NUM);
	info.key_name = key_name_1;

	info.key_flag_start = EFPG_MAC_WLAN_ADDR_FLAG_START / 8 * 8;
	info.key_flag_len = EFPG_BITS_TO_BYTE_CNT(EFPG_MAC_WLAN_ADDR_FLAG_NUM);
	info.key_flag_name = key_flag_name;

	etf_printk(XRADIO_DBG_NIY, "%s Flag: key name %s len %d start %d\n", __func__,
		info.key_flag_name, info.key_flag_len, info.key_flag_start);
	etf_printk(XRADIO_DBG_NIY, "%s Data: key name %s len %d start %d\n", __func__,
		info.key_name, info.key_len, info.key_start);
	etf_printk(XRADIO_DBG_NIY, "%s Data: %x:%x:%x:%x:%x:%x\n", __func__,
		data[0], data[1], data[2], data[3], data[4], data[5]);

	ret = xradio_efuse_write_mac_region(&info, data);

	return ret;
}

static u8 etf_efuse_reqcksum(u8 data, u8 cksum)
{
	u8 res;

	res = data + cksum;

	return res;
}

void xradio_efuse_info(void *data)
{
	if (xradio_rw_efuse_init()) {
		etf_printk(XRADIO_DBG_ERROR, "%s rw efuse init fail!\n", __func__);
		return;
	}

	memcpy(data, efuse_addr, EFUSE_AREA_SIZE);
	xradio_rw_efuse_deinit();
}

static int etf_read_efuse(void *data)
{
	ETF_READ_EFUSE_ELEMENT_REQ *req = data;
	ETF_READ_EFUSE_ELEMENT_CNF *cnf = data;
	u32 status = EFUSE_STATUS_CODE__DEFAULT;

	u16 type = RW_EFUSE_NONE;
	u32 addr = 0, len = 0;
	u8 *pdata = cnf->EfuseData;

	efuse_area_in_use = 0;

	if (ADAPTER_RX_BUF_LEN < sizeof(ETF_READ_EFUSE_ELEMENT_CNF)) {
		etf_printk(XRADIO_DBG_ERROR, "%s ADAPTER_RX_BUF_LEN %d < efuse cnf %d\n", __func__,
			ADAPTER_RX_BUF_LEN, sizeof(ETF_READ_EFUSE_ELEMENT_CNF));
		return -1;
	}
	if (xradio_rw_efuse_init()) {
		etf_printk(XRADIO_DBG_ERROR, "%s read efuse fail!\n", __func__);
		return -1;
	}

	if (etf_efuse_reqcksum(req->Type, req->CheckSum) != 0xFF) {
		status = EFUSE_STATUS_CODE__CS_ERR;
		goto cnf;
	}

	switch (req->Type) {
	case RW_EFUSE_CHIPID:
	{
		type = RW_EFUSE_CHIPID;
		status = EFUSE_STATUS_CODE__INVALID_PARAMS;
		break;
	}
	case RW_EFUSE_DCXO_TRIM:
	{
		type = RW_EFUSE_DCXO_TRIM;
		status = xradio_efuse_read_dcxo(pdata);
		len = 1;
		break;
	}
	case RW_EFUSE_POUT_CAL:
	{
		type = RW_EFUSE_POUT_CAL;
		status = xradio_efuse_read_wlan_pout(pdata);
		len = 3;
		break;
	}
	case RW_EFUSE_MAC:
	{
		type = RW_EFUSE_MAC;
		status = xradio_efuse_read_wlan_mac(pdata);
		len = 6;
		break;
	}
	case RW_EFUSE_USER_AREA:
	{
		type = RW_EFUSE_USER_AREA;
		addr = req->Addr;
		len = req->Len;
		status = EFUSE_STATUS_CODE__INVALID_PARAMS;
		break;
	}
	case RW_EFUSE_POUT_CAL_BT:
	{
		type = RW_EFUSE_POUT_CAL_BT;
		status = EFUSE_STATUS_CODE__INVALID_PARAMS;
		break;
	}
	default:
		etf_printk(XRADIO_DBG_ERROR, "%s req type %d not found.\n", __func__, req->Type);
	}

cnf:
	cnf->Type = type;
	cnf->MsgLen = EFUSE_READ_CNF_DEFAULT_LEN + len;
	cnf->CheckSum = (u8)(0xFF - (u8)cnf->MsgLen);
	cnf->AreaInUse = efuse_area_in_use;
	cnf->MsgId = ETF_READ_EFUSE_ELEMENT_CNF_ID;
	cnf->Result = status;
	xradio_rw_efuse_deinit();

	return xradio_adapter_send(cnf, cnf->MsgLen);
}

static int etf_write_efuse(void *data)
{
	ETF_WRITE_EFUSE_ELEMENT_REQ *req = data;
	ETF_WRITE_EFUSE_ELEMENT_CNF *cnf = data;
	u32 status = EFUSE_STATUS_CODE__DEFAULT;

	u16 type = RW_EFUSE_NONE;
	u32 addr = 0, len = 0;
	u8 *pdata = req->EfuseData;

	efuse_area_in_use = 0;

	if (ADAPTER_RX_BUF_LEN < sizeof(ETF_WRITE_EFUSE_ELEMENT_CNF)) {
		etf_printk(XRADIO_DBG_ERROR, "%s ADAPTER_RX_BUF_LEN %d < efuse cnf %d\n", __func__,
			ADAPTER_RX_BUF_LEN, sizeof(ETF_WRITE_EFUSE_ELEMENT_CNF));
		return -1;
	}
	if (xradio_rw_efuse_init()) {
		etf_printk(XRADIO_DBG_ERROR, "%s read efuse fail!\n", __func__);
		return -1;
	}

	if (etf_efuse_reqcksum(req->Type, req->CheckSum) != 0xFF) {
		status = EFUSE_STATUS_CODE__CS_ERR;
		goto cnf;
	}

	switch (req->Type) {
	case RW_EFUSE_CHIPID:
	{
		type = RW_EFUSE_CHIPID;
		status = EFUSE_STATUS_CODE__INVALID_PARAMS;
		break;
	}
	case RW_EFUSE_DCXO_TRIM:
	{
		type = RW_EFUSE_DCXO_TRIM;
		status = xradio_efuse_write_dcxo(pdata);
		len = 0;
		break;
	}
	case RW_EFUSE_POUT_CAL:
	{
		type = RW_EFUSE_POUT_CAL;
		status = xradio_efuse_write_wlan_pout(pdata);
		len = 0;
		break;
	}
	case RW_EFUSE_MAC:
	{
		type = RW_EFUSE_MAC;
		status = xradio_efuse_write_wlan_mac(pdata);
		len = 0;
		break;
	}
	case RW_EFUSE_USER_AREA:
	{
		type = RW_EFUSE_USER_AREA;
		addr = req->Addr;
		len = req->Len;
		status = EFUSE_STATUS_CODE__INVALID_PARAMS;
		break;
	}
	case RW_EFUSE_POUT_CAL_BT:
	{
		type = RW_EFUSE_POUT_CAL_BT;
		status = EFUSE_STATUS_CODE__INVALID_PARAMS;
		break;
	}
	default:
		etf_printk(XRADIO_DBG_ERROR, "%s req type %d not found.\n", __func__, req->Type);
	}

cnf:
	cnf->Type = type;
	cnf->MsgLen = EFUSE_WRITE_CNF_DEFAULT_LEN + len;
	cnf->CheckSum = (u8)(0xFF - (u8)cnf->MsgLen);
	cnf->MsgId = ETF_WRITE_EFUSE_ELEMENT_CNF_ID;
	cnf->Result = status;
	xradio_rw_efuse_deinit();

	return xradio_adapter_send(cnf, cnf->MsgLen);
}

#endif // V821 EFUSE

static int xradio_etf_connect(void)
{
	int ret = BOOT_SUCCESS;
	etf_printk(XRADIO_DBG_TRC, "%s\n", __func__);

	if (etf_priv.etf_state == ETF_STAT_NULL) {
		etf_priv.etf_state = ETF_STAT_CONNECTING;
		ret = xradio_core_init();
		if (ret) {
			etf_printk(XRADIO_DBG_ERROR, "%s:xradio_core_init failed(%d).\n",
						__func__, ret);
			goto err;
		}
		etf_priv.etf_state = ETF_STAT_CONNECTED;
	} else {
		etf_printk(XRADIO_DBG_WARN, "%s etf is already connect.\n", __func__);
		return ETF_ERR_CONNECTED;
	}
	etf_printk(XRADIO_DBG_ALWY, "%s:ETF connect success.\n", __func__);
	return 0;

err:
	etf_priv.etf_state = ETF_STAT_NULL;
	return ret;
}

static void xradio_etf_disconnect(void)
{
	etf_printk(XRADIO_DBG_TRC, "%s\n", __func__);
	if (etf_priv.etf_state != ETF_STAT_NULL) {
		xradio_core_deinit();
		etf_priv.etf_state = ETF_STAT_NULL;
	}
	etf_printk(XRADIO_DBG_ALWY, "%s end.\n", __func__);
}

static int xradio_etf_proc(void *data, int len)
{
	int ret = BOOT_SUCCESS;
	struct wsm_hdr *hdr_rev = (struct wsm_hdr *)data;
	struct drv_download *download;
	int    datalen;
	etf_printk(XRADIO_DBG_TRC, "%s\n", __func__);

	down(&etf_priv.etf_lock);
	/* Don't process ETF cmd in wlan mode */
	if (etf_priv.is_wlan) {
		etf_printk(XRADIO_DBG_WARN, "%s is in wlan mode.\n", __func__);
		ret = ETF_ERR_WLAN_MODE;
		etf_driver_resp(BOOT_STATE_NULL, ret);
		goto out;
	}

	etf_printk(XRADIO_DBG_NIY, "%s: passed cmd=0x%04x, len=%d",
				__func__, MSG_ID(hdr_rev->id), hdr_rev->len);

	/* Process msg from ETF API */
	switch (MSG_ID(hdr_rev->id)) {
	case ETF_SOFT_RESET_REQ_ID:
		etf_printk(XRADIO_DBG_ALWY, "SOFT_RESET\n");
		xradio_etf_disconnect();
		etf_priv.fw_path   = NULL;
		etf_priv.sdd_path  = NULL;
		etf_priv.seq_send  = 0;
		etf_priv.seq_recv  = 0;
		memset(&context_save, 0, sizeof(context_save));
		etf_driver_resp(BOOT_STATE_NULL, BOOT_SUCCESS);
		break;
	case ETF_GET_API_CONTEXT_ID: /* for ETF tools which cannot save context*/
		{
			struct etf_api_context_result *ctxt_ret = NULL;
			struct xradio_common *hw_priv = etf_priv.core_priv;
			ctxt_ret = (struct etf_api_context_result *)hdr_rev;
			if (hw_priv) {
				ctxt_ret->is_etf_fw_run = hw_priv->wsm_caps.firmwareReady;
				memcpy(ctxt_ret->fw_label, hw_priv->wsm_caps.fw_label,
					min(HI_SW_LABEL_MAX, WSM_FW_LABEL));
				ctxt_ret->fw_api_ver = hw_priv->wsm_caps.firmwareApiVer;
				ctxt_ret->mib_baseaddr = context_save.config[1];
			} else {
				ctxt_ret->is_etf_fw_run = 0;
				memset(ctxt_ret->fw_label, 0, HI_SW_LABEL_MAX);
			}
			ctxt_ret->len = sizeof(*ctxt_ret);
			ctxt_ret->id |= ETF_CNF_BASE;
			ctxt_ret->result = 0;
			xradio_adapter_send(ctxt_ret, sizeof(*ctxt_ret));
		}
		break;
	case ETF_DOWNLOAD_ID:
		etf_printk(XRADIO_DBG_NIY, "DOWNLOAD_FW, size=%d\n", hdr_rev->len);
		download = (struct drv_download *)(hdr_rev + 1);
		datalen = hdr_rev->len - sizeof(*download) - sizeof(*hdr_rev);
		if (datalen <= 0) {
			/*it means old version APK, TODO:remove it in future */
			if (hdr_rev->len <= sizeof(*hdr_rev)) {
				etf_driver_resp(BOOT_COMPLETE, BOOT_SUCCESS);
				ret = xradio_etf_connect();
				if (ret)
					etf_driver_resp(BOOT_STATE_NULL, ret);
			}
			break;
		}
		if (download->flags & DOWNLOAD_F_PATH_ONLY) {
			etf_printk(XRADIO_DBG_WARN, "fw path=%s\n", (char *)(download+1));
			/*TODO: set etf_priv.fw_path*/

		} else if (download->flags & DOWNLOAD_F_START) {
			etf_printk(XRADIO_DBG_NIY, "fw first block\n");
			/*TODO: open a tmp file to save firmware */
		}

		/*TODO: put data into a tmp file */

		/*TODO: put data into a tmp file */
		if ((download->flags & DOWNLOAD_F_END)) {
			etf_printk(XRADIO_DBG_NIY, "fw last block\n");
			etf_driver_resp(BOOT_COMPLETE, BOOT_SUCCESS);
			/*TODO: save data tmp file and update etf_priv.fw_path*/
			ret = xradio_etf_connect();
			if (ret)
				etf_driver_resp(BOOT_STATE_NULL, ret);
		}
		break;
	case ETF_DOWNLOAD_SDD_ID:
		etf_printk(XRADIO_DBG_NIY, "DOWNLOAD_SDD, size=%d\n", hdr_rev->len);
		download = (struct drv_download *)(hdr_rev + 1);
		datalen = hdr_rev->len - sizeof(*download) - sizeof(*hdr_rev);
		if (datalen <= 0) {
			/*it means old version APK, TODO:remove it in future */
			if (hdr_rev->len <= sizeof(*hdr_rev)) {
				etf_driver_resp(BOOT_SDD_COMPLETE, BOOT_SUCCESS);
			}
			break;
		}

		if (download->flags & DOWNLOAD_F_PATH_ONLY) {
			etf_printk(XRADIO_DBG_WARN, "sdd path=%s\n", (char *)(download+1));
			/*TODO: set etf_priv.fw_path*/

		} else if ((download->flags & DOWNLOAD_F_START)) {
			etf_printk(XRADIO_DBG_NIY, "sdd first block\n");
			/*TODO: open a tmp file to save firmware */
		}

		/*TODO: put data into a tmp file */

		if ((download->flags & DOWNLOAD_F_END)) {
			etf_printk(XRADIO_DBG_NIY, "sdd last block\n");
			etf_driver_resp(BOOT_SDD_COMPLETE, BOOT_SUCCESS);
			/*TODO: save data tmp file and update etf_priv.sdd_path*/
		}
		break;
	case ETF_CONNECT_ID:
		{
			ETF_CONNECT_REQ_T *connect_req = (ETF_CONNECT_REQ_T *)hdr_rev;

			etf_printk(XRADIO_DBG_ALWY, "ETF_CONNECT_ID!\n");
			etf_driver_resp(BOOT_COMPLETE, BOOT_SUCCESS);
			ret = xradio_etf_connect();
			if (ret)
				etf_driver_resp(BOOT_STATE_NULL, ret);

			if (etf_check_version(connect_req->version)) {
				ret = etf_alloc_cli_buffer(connect_req->data_len, &etf_priv);
				if (ret)
					etf_driver_resp(BOOT_STATE_NULL, ret);
			}
		}
		break;
	case ETF_DISCONNECT_ID:
		etf_printk(XRADIO_DBG_ALWY, "ETF_DISCONNECT_ID!\n");
		xradio_etf_disconnect();
		etf_free_cli_buffer(&etf_priv);
		etf_driver_resp(BOOT_STATE_NULL, BOOT_SUCCESS);
		break;
	case ETF_RECONNECT_ID:
		{
			ETF_CONNECT_REQ_T *reconnect_req = (ETF_CONNECT_REQ_T *)hdr_rev;

			etf_printk(XRADIO_DBG_ALWY, "ETF_RECONNECT_ID!\n");
			xradio_etf_disconnect();
			etf_driver_resp(BOOT_COMPLETE, BOOT_SUCCESS);
			ret = xradio_etf_connect();
			if (ret)
				etf_driver_resp(BOOT_STATE_NULL, ret);

			if (etf_check_version(reconnect_req->version)) {
				ret = etf_alloc_cli_buffer(reconnect_req->data_len, &etf_priv);
				if (ret)
					etf_driver_resp(BOOT_STATE_NULL, ret);
			}
		}
		break;
	case ETF_GET_SDD_POWER_DEFT:  /* get default tx power */
		{
#ifdef USE_VFS_FIRMWARE
			const struct xr_file  *sdd = NULL;
#else
			const struct firmware *sdd = NULL;
#endif
			etf_printk(XRADIO_DBG_NIY, "%s ETF_GET_SDD_POWER_DEFT!\n", __func__);

#ifdef USE_VFS_FIRMWARE
			sdd = xr_request_file(etf_get_sddpath());
#else
			ret = request_firmware(&sdd, etf_get_sddpath(), NULL);
#endif
			/* get sdd data */
			if (likely(sdd) && likely(sdd->data)) {
				int i;
				u16  *outpower = (u16 *)(hdr_rev + 1);
				void *ie_data = NULL;
				int ie_len = etf_get_sdd_param((u8 *)sdd->data, sdd->size,
								SDD_MAX_OUTPUT_POWER_2G4_ELT_ID, &ie_data);
				if (ie_len > 0 && ie_data &&
				   ie_len < (ADAPTER_RX_BUF_LEN - sizeof(*hdr_rev))) {
					memcpy(outpower, ie_data, ie_len);
					for (i = 0; i < (ie_len>>1); i++)
						outpower[i] -= 48; /*default pwr = max power - 48*/
				}
				hdr_rev->len = sizeof(*hdr_rev) + ie_len;
				xradio_adapter_send(hdr_rev, hdr_rev->len);
#ifdef USE_VFS_FIRMWARE
				xr_fileclose(sdd);
#else
				release_firmware(sdd);
#endif
			} else {
				etf_printk(XRADIO_DBG_ERROR, "%s: can't load sdd file %s.\n",
							__func__, etf_get_sddpath());
				ret = -ENOENT;
				hdr_rev->len = sizeof(*hdr_rev);
				xradio_adapter_send(hdr_rev, sizeof(*hdr_rev));
			}
		}
		break;
	case ETF_GET_SDD_PARAM_ID:
		{
			struct xradio_common *hw_priv = etf_priv.core_priv;
			struct get_sdd_param_req *sddreq = (struct get_sdd_param_req *)hdr_rev;
			struct get_sdd_result *sddret = (struct get_sdd_result *)sddreq;
			void *ie_data = NULL;
			int ie_len = 0;
#ifdef USE_VFS_FIRMWARE
			const struct xr_file  *sdd = NULL;
#else
			const struct firmware *sdd = NULL;
#endif
			etf_printk(XRADIO_DBG_NIY, "%s ETF_GET_SDD_PARAM_ID!\n", __func__);

			if (hw_priv && hw_priv->sdd) {
				etf_printk(XRADIO_DBG_NIY, "%s use xradio_common sdd!\n", __func__);
				sdd = hw_priv->sdd;
			} else {
#ifdef USE_VFS_FIRMWARE
				sdd = xr_request_file(etf_get_sddpath());
#else
				ret = request_firmware(&sdd, etf_get_sddpath(), NULL);
#endif
			}

			hdr_rev->id  = hdr_rev->id + ETF_CNF_BASE;
			if (likely(sdd) && likely(sdd->data)) {
				if (sddreq->flags & FLAG_GET_SDD_ALL) {
					sddret->result  = 0;
					sddret->length  = sdd->size;
					xradio_adapter_send_pkg(sddret, sizeof(*sddret), (u8 *)sdd->data,
							sddret->length);
				} else {
					ie_len = etf_get_sdd_param((u8 *)sdd->data, sdd->size,
								sddreq->ies, &ie_data);
					if (ie_data && ie_len > 0) {
						sddret->result  = 0;
						sddret->length  = ie_len;
						hdr_rev->len = sizeof(*sddret) + sddret->length;
						xradio_adapter_send_pkg(sddret, sizeof(*sddret), ie_data,
								sddret->length);
					} else {
						sddret->result = -ENOMSG;
						sddret->length = 0;
						hdr_rev->len = sizeof(*sddret) + sddret->length;
						xradio_adapter_send(sddret, sizeof(*sddret));
					}
				}
				if (hw_priv && hw_priv->sdd) {
					sdd = NULL;
				} else {
#ifdef USE_VFS_FIRMWARE
					xr_fileclose(sdd);
#else
					release_firmware(sdd);
#endif
				}
			} else {
				etf_printk(XRADIO_DBG_ERROR, "%s: can't load sdd file %s.\n",
							__func__, etf_get_sddpath());
				sddret->result = -ENOENT;
				sddret->length = 0;
				hdr_rev->len = sizeof(*sddret) + sddret->length;
				xradio_adapter_send(sddret, sizeof(*sddret));
			}
		}
		break;
	case ETF_SET_CLI_PAR_DEFT:
		{
			CLI_PARAM_SAVE_T *hdr_data = (CLI_PARAM_SAVE_T *)hdr_rev;
			ETF_PARAM0 hdr_ret;

			etf_printk(XRADIO_DBG_NIY, "%s ETF_SET_CLI_PAR_DEFT!\n", __func__);
			hdr_ret.MsgId = hdr_rev->id + ETF_CNF_BASE;
			hdr_ret.MsgLen = sizeof(ETF_PARAM0);

			if (etf_check_version(hdr_data->version)) {
				if (etf_priv.cli_data && hdr_data->data_len == etf_priv.cli_data_len) {
					memset(etf_priv.cli_data, 0, hdr_data->data_len);
					memcpy(etf_priv.cli_data,
						    hdr_data->data, hdr_data->data_len);
					hdr_ret.result = BOOT_SUCCESS;
				} else {
					etf_printk(XRADIO_DBG_ALWY,
						"%s support_len = %d req_len = %d\n",
						__func__, etf_priv.cli_data_len, hdr_data->data_len);
					hdr_ret.result = BOOT_ERR_BAD_OP;
				}
			} else {
					hdr_ret.result = ETF_ERR_CHECK_VERSION;
			}

			xradio_adapter_send(&hdr_ret, hdr_ret.MsgLen);
		}
		break;
	case ETF_GET_CLI_PAR_DEFT:
		{
			struct get_cli_data_s *hdr_data;

			hdr_data = (struct get_cli_data_s *)hdr_rev;
			etf_printk(XRADIO_DBG_NIY, "%s ETF_GET_CLI_PAR_DEFT!\n", __func__);
			hdr_rev->id  = hdr_rev->id + ETF_CNF_BASE;
			if (etf_check_version(hdr_data->version)) {
				hdr_data->result   = BOOT_SUCCESS;
				hdr_data->data_len = etf_priv.cli_data_len;
				hdr_data->version  = etf_priv.version;
				hdr_data->msg_len  = sizeof(struct get_cli_data_s) + hdr_data->data_len;
				xradio_adapter_send_pkg(hdr_data, sizeof(struct get_cli_data_s),
						(void *)etf_priv.cli_data, hdr_data->data_len);
			} else {
				hdr_data->result = ETF_ERR_CHECK_VERSION;
				hdr_data->data_len = 0;
				hdr_data->version  = etf_priv.version;
				hdr_data->msg_len  = sizeof(struct get_cli_data_s);
				xradio_adapter_send(hdr_data, hdr_data->msg_len);
			}
		}
		break;
	case ETF_GET_SDD_PATH_ID:
		{
			struct get_sdd_patch_req *param_req;
			struct get_sdd_patch_ret *param_ret;
			const char *sdd_path = etf_get_sddpath();

			param_req = (struct get_sdd_patch_req *)hdr_rev;
			param_ret = (struct get_sdd_patch_ret *)param_req;
			etf_printk(XRADIO_DBG_NIY, "%s ETF_GET_SDD_PATH_ID!\n", __func__);

			hdr_rev->id  = hdr_rev->id + ETF_CNF_BASE;
			param_ret->result = BOOT_SUCCESS;
			param_ret->length = strlen(sdd_path);
			hdr_rev->len = sizeof(*param_ret) + param_ret->length;
			xradio_adapter_send_pkg(param_ret, sizeof(*param_ret),
					(void *)sdd_path, param_ret->length);
		}
		break;
	default:
		etf_printk(XRADIO_DBG_NIY, "%s: passed cmd=0x%04x, len=%d",
					__func__, MSG_ID(hdr_rev->id), hdr_rev->len);
		if (etf_priv.etf_state != ETF_STAT_CONNECTED) {
			etf_printk(XRADIO_DBG_WARN, "%s etf Not connect.\n", __func__);
			ret = ETF_ERR_NOT_CONNECT;
			etf_driver_resp(BOOT_STATE_NULL, ret);
			goto out;
		}
		if (SEQ_MASK(etf_priv.seq_send) != MSG_SEQ(hdr_rev->id)) {
			etf_printk(XRADIO_DBG_NIY, "%s:seq err, drv_seq=%d, seq=%d\n",
						__func__, etf_priv.seq_send, MSG_SEQ(hdr_rev->id));
			etf_priv.seq_send = MSG_SEQ(hdr_rev->id);
		}
		etf_priv.seq_send++;
		if (ETF_HWT_REQ == MSG_ID(hdr_rev->id)) {
			/*TODO: HW test*/
			etf_printk(XRADIO_DBG_ERROR, "%s HW test unsupport\n", __func__);
		} else if (ETF_READ_PRCM_REG_REQ_ID == MSG_ID(hdr_rev->id)) {
			struct ETF_PARAM2 *hdr_data;

#if defined(CONFIG_DRIVER_V821)
			u8 dcxo_from_a_die;
			u32 freq_offset;

			dcxo_from_a_die = etf_get_freq_offset(&freq_offset);
			if (dcxo_from_a_die)
				freq_offset = 0xffff;
#endif
			hdr_data = (struct ETF_PARAM2 *)hdr_rev;
			etf_printk(XRADIO_DBG_NIY, "%s ETF_READ_PRCM_REG_REQ_ID!\n", __func__);
			hdr_data->MsgId = hdr_rev->id + ETF_CNF_BASE;
			hdr_data->result = BOOT_SUCCESS;
			hdr_data->MsgLen = sizeof(struct ETF_PARAM2);
			hdr_data->param2 = freq_offset;
			xradio_adapter_send_pkg(hdr_data, sizeof(struct get_cli_data_s),
					(void *)etf_priv.cli_data, hdr_data->MsgLen);
		} else if (ETF_WRITE_PRCM_REG_REQ_ID == MSG_ID(hdr_rev->id)) {
			struct ETF_PARAM2 *hdr_data;
			struct ETF_PARAM2 *freq_offset_data = (struct ETF_PARAM2 *)data;

			etf_set_freq_offset(freq_offset_data->param2);
			hdr_data = (struct ETF_PARAM2 *)hdr_rev;
			etf_printk(XRADIO_DBG_NIY, "%s ETF_WRITE_PRCM_REG_REQ_ID!\n", __func__);
			hdr_data->MsgId = hdr_rev->id + ETF_CNF_BASE;
			hdr_data->result = BOOT_SUCCESS;
			hdr_data->MsgLen = sizeof(struct ETF_PARAM2);
			xradio_adapter_send_pkg(hdr_data, sizeof(struct get_cli_data_s),
					(void *)etf_priv.cli_data, hdr_data->MsgLen);
#ifdef CONFIG_DRIVER_V821
		} else if (ETF_READ_EFUSE_ELEMENT_REQ_ID == MSG_ID(hdr_rev->id)) {
			etf_printk(XRADIO_DBG_NIY, "%s read efuse\n", __func__);
			ret = etf_read_efuse(data);
		} else if (ETF_WRITE_EFUSE_ELEMENT_REQ_ID == MSG_ID(hdr_rev->id)) {
			etf_printk(XRADIO_DBG_NIY, "%s write efuse\n", __func__);
			ret = etf_write_efuse(data);
#endif
		} else if (etf_priv.core_priv) {
			if (ETF_DOWNLOAD_SDD == MSG_ID(hdr_rev->id) &&
			    hdr_rev->len <= sizeof(struct etf_sdd_req)) {
				struct xradio_common *hw_priv = etf_priv.core_priv;
				struct etf_sdd_req *sdd_req = (struct etf_sdd_req *)hdr_rev;
				u32 sdd_cmd = sdd_req->sdd_cmd;

				etf_printk(XRADIO_DBG_NIY, "%s add sdd data, cmd=0x%08x!\n",
							__func__, sdd_cmd);
				if (hw_priv->sdd) {
					u8 *sdd_data = (u8 *)(hdr_rev + 1);
					int sdd_len  = hw_priv->sdd->size;
					if (sdd_len + sizeof(*hdr_rev) + 4 < ADAPTER_RX_BUF_LEN) {
						memcpy(sdd_data, hw_priv->sdd->data, sdd_len);
						memcpy(sdd_data + sdd_len, &sdd_cmd, 4);
						hdr_rev->len += sdd_len;
					} else {
						etf_printk(XRADIO_DBG_ERROR,
									"ADAPTER_RX_BUF_LEN=%d, sdd_len=%d\n",
									 ADAPTER_RX_BUF_LEN, sdd_len);
					}
				} else
					etf_printk(XRADIO_DBG_ERROR, "%s core_priv sdd is NULL\n",
							__func__);
			}
			/* send to device*/
			ret = wsm_etf_cmd(etf_priv.core_priv, (struct wsm_hdr *)data);
			if (ret < 0) {
				etf_driver_resp(BOOT_STATE_NULL, ret);
				etf_printk(XRADIO_DBG_ERROR, "%s wsm_etf_cmd failed(%d)\n",
							__func__, ret);
			}
		} else {
			etf_printk(XRADIO_DBG_ERROR, "%s core_priv is NULL\n",
						__func__);
		}
		break;
	}
	up(&etf_priv.etf_lock);
	etf_printk(XRADIO_DBG_NIY, "%s success\n", __func__);
	return 0;

out:
	up(&etf_priv.etf_lock);
	if (ret)
		etf_printk(XRADIO_DBG_ERROR, "%s proc failed(%d)\n", __func__, ret);
	return ret;
}

int xradio_etf_from_device(struct sk_buff **skb)
{
	int ret = 0;
	struct wsm_hdr *wsm = (struct wsm_hdr *)((*skb)->data);
	etf_printk(XRADIO_DBG_NIY, "%s MsgId=0x%04x, len=%d\n", __func__,
			   wsm->id, wsm->len);
	ret = xradio_adapter_send((void *)wsm, wsm->len);
	if (ret < 0) {
		etf_printk(XRADIO_DBG_ERROR, "%s xradio_adapter_send failed(%d).\n",
					__func__, ret);
	} else {
		etf_priv.seq_recv++;
	}
	return 0;
}

void xradio_etf_save_context(void *buf, int len)
{
	etf_printk(XRADIO_DBG_TRC, "%s\n", __func__);
	if (len == sizeof(context_save)) {
		etf_printk(XRADIO_DBG_NIY, "%s, len=%d\n", __func__, len);
		memcpy(&context_save, buf, sizeof(context_save));
	} else if (len > sizeof(context_save)) {
		etf_printk(XRADIO_DBG_WARN, "%s, len=%d\n", __func__, len);
		memcpy(&context_save, buf, sizeof(context_save));
	} else {
		etf_printk(XRADIO_DBG_ERROR, "%s,len too short=%d\n", __func__, len);
	}
}

void xradio_etf_to_wlan(u32 change)
{
	down(&etf_priv.etf_lock);
	etf_priv.is_wlan = change;
	if (change && etf_is_connect()) {
		etf_printk(XRADIO_DBG_NIY, "%s change to wlan.\n", __func__);
		xradio_etf_disconnect();
	}
	up(&etf_priv.etf_lock);
}
EXPORT_SYMBOL_GPL(xradio_etf_to_wlan);

int xradio_etf_suspend(void)
{
	etf_printk(XRADIO_DBG_NIY, "%s\n", __func__);
	if (down_trylock(&etf_priv.etf_lock)) {
		etf_printk(XRADIO_DBG_WARN,
			"%s etf_lock is locked\n", __func__);
		return -EBUSY; /*etf is locked.*/
	}
	if (etf_is_connect()) {
		etf_printk(XRADIO_DBG_WARN,
			"%s etf is running, don't suspend!\n", __func__);
		up(&etf_priv.etf_lock);
		return -EBUSY; /*etf is running*/
	}
	up(&etf_priv.etf_lock);
	return 0;
}

int xradio_etf_resume(void)
{
	etf_printk(XRADIO_DBG_NIY, "%s\n", __func__);
	return 0;
}

int xradio_etf_init(void)
{
	int ret = 0;
	etf_printk(XRADIO_DBG_TRC, "%s\n", __func__);
	sema_init(&etf_priv.etf_lock, 1);
	etf_priv.etf_state = ETF_STAT_NULL;
	etf_priv.fw_path  = NULL;
	etf_priv.sdd_path = NULL;
	etf_priv.adapter = xradio_adapter_init(&xradio_etf_proc);
	etf_adapter_version_init(&etf_priv);
	return ret;
}

void xradio_etf_deinit(void)
{
	etf_printk(XRADIO_DBG_TRC, "%s\n", __func__);
	/*ensure in disconnect state when etf deinit*/
	xradio_etf_to_wlan(1);
	etf_free_cli_buffer(&etf_priv);
	xradio_adapter_deinit();
	memset(&etf_priv, 0, sizeof(etf_priv));
}

#endif /*XRADIO_ETF_C*/
