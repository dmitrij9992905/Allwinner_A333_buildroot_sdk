// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007 Allwinner Technology Co., Ltd.
 *
 * Written by: fenglizhen <fenglizhen@allwinnertech.com>
 */

#include <config.h>
#include <common.h>
#include <errno.h>
#include <malloc.h>
#include <memalign.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/usb/composite.h>
#include <linux/compiler.h>
#include <version.h>
#include <g_dnl.h>
#include "../../sunxi_usb/usb_base.h"


// #define F_SUNXI_USB_DEBUG
#undef F_SUNXI_USB_DEBUG
#ifdef F_SUNXI_USB_DEBUG
#define __debug(fmt, args...)	printf(fmt, ##args)
#else
#define __debug(fmt, args...)
#endif

struct f_sunxi_usb {
	struct usb_function usb_function;
	struct usb_ep *in_ep, *out_ep;
	struct usb_request *in_req, *out_req;
};

extern sunxi_usb_setup_req_t *sunxi_udev_active;
static struct f_sunxi_usb *sunxi_usb_func;

static inline struct f_sunxi_usb *func_to_sunxi_usb(struct usb_function *f)
{
	return container_of(f, struct f_sunxi_usb, usb_function);
}

static struct usb_endpoint_descriptor fs_ep_in = {
	.bLength		= USB_DT_ENDPOINT_SIZE,
	.bDescriptorType	= USB_DT_ENDPOINT,
	.bEndpointAddress 	= USB_DIR_IN | 0x01,
	.bmAttributes		= USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize		= cpu_to_le16(64),
	.bInterval		= 0x00,
};

static struct usb_endpoint_descriptor fs_ep_out = {
	.bLength		= USB_DT_ENDPOINT_SIZE,
	.bDescriptorType	= USB_DT_ENDPOINT,
	.bEndpointAddress	= USB_DIR_OUT | 0x02,
	.bmAttributes		= USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize		= cpu_to_le16(64),
	.bInterval		= 0x00,
};

static struct usb_endpoint_descriptor hs_ep_in = {
	.bLength		= USB_DT_ENDPOINT_SIZE,
	.bDescriptorType	= USB_DT_ENDPOINT,
	.bEndpointAddress	= USB_DIR_IN | 0x01,
	.bmAttributes		= USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize		= cpu_to_le16(512),
	.bInterval		= 0x00,
};

static struct usb_endpoint_descriptor hs_ep_out = {
	.bLength		= USB_DT_ENDPOINT_SIZE,
	.bDescriptorType	= USB_DT_ENDPOINT,
	.bEndpointAddress	= USB_DIR_OUT | 0x02,
	.bmAttributes		= USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize		= cpu_to_le16(512),
	.bInterval		= 0x00,
};

static struct usb_interface_descriptor interface_desc = {
	.bLength		= USB_DT_INTERFACE_SIZE,
	.bDescriptorType	= USB_DT_INTERFACE,
	.bInterfaceNumber	= 0x00,
	.bAlternateSetting	= 0x00,
	.bNumEndpoints		= 0x02,
	.bInterfaceClass	= 0xff,
	.bInterfaceSubClass	= 0xff,
	.bInterfaceProtocol	= 0xff,
	.iInterface		= 0x00,
};

static struct usb_descriptor_header *sunxi_usb_fs_function[] = {
	(struct usb_descriptor_header *)&interface_desc,
	(struct usb_descriptor_header *)&fs_ep_in,
	(struct usb_descriptor_header *)&fs_ep_out,
};

static struct usb_descriptor_header *sunxi_usb_hs_function[] = {
	(struct usb_descriptor_header *)&interface_desc,
	(struct usb_descriptor_header *)&hs_ep_in,
	(struct usb_descriptor_header *)&hs_ep_out,
	NULL,
};

static const char sunxi_usb_name[] = "Allwinner USB GADGET";

static struct usb_string sunxi_usb_string_defs[] = {
	[0].s = sunxi_usb_name,
	{  }			/* end of list */
};

static struct usb_gadget_strings stringtab_sunxi_usb = {
	.language	= 0x0409,	/* en-us */
	.strings	= sunxi_usb_string_defs,
};

static struct usb_gadget_strings *sunxi_usb_strings[] = {
	&stringtab_sunxi_usb,
	NULL,
};

struct f_sunxi_usb *get_sunxi_usb(void)
{
	struct f_sunxi_usb *f_sunxi_usb = sunxi_usb_func;

	if (!f_sunxi_usb) {
		f_sunxi_usb = memalign(CONFIG_SYS_CACHELINE_SIZE, sizeof(*f_sunxi_usb));
		if (!f_sunxi_usb)
			return 0;

		sunxi_usb_func = f_sunxi_usb;
		memset(f_sunxi_usb, 0, sizeof(*f_sunxi_usb));
	}

	return f_sunxi_usb;
}

static struct usb_endpoint_descriptor *rkusb_ep_desc(
struct usb_gadget *g,
struct usb_endpoint_descriptor *fs,
struct usb_endpoint_descriptor *hs)
{
	if (gadget_is_dualspeed(g) && g->speed == USB_SPEED_HIGH)
		return hs;
	return fs;
}

static void sunxi_usb_print(struct usb_ep *ep, struct usb_request *req)
{
#ifdef F_SUNXI_USB_DEBUG
	int i	  = 0;
	int is_in = usb_endpoint_dir_in(ep->desc);

	__debug("\n\e[;34m%s! ep: %x len:%d actual:%d \e[0m \n",
		is_in ? "IN" : "OUT", usb_endpoint_num(ep->desc), req->length, req->actual);

	for (i = 0; i < req->actual; i++) {
		if (((i + 1) % 32) && ((i + 1) != req->actual))
			__debug("%.2x ", ((unsigned char *)req->buf)[i]);
		else
			__debug("%.2x \n", ((unsigned char *)req->buf)[i]);
	}
#endif
}

int sunxi_usb_tx_write(const char *buffer, unsigned int buffer_size)
{
	int ret = 0;
	struct usb_request *in_req = sunxi_usb_func->in_req;

	if (in_req->status == -EINPROGRESS)
		usb_ep_dequeue(sunxi_usb_func->in_ep, in_req);

	memcpy(in_req->buf, buffer, buffer_size);
	in_req->length = buffer_size;

	ret = usb_ep_queue(sunxi_usb_func->in_ep, in_req, 0);
	if (ret)
		printf("[%s] Error %d on queue\n", __func__, ret);

	return ret;
}

static int sunxi_usb_rx_read(const char *buffer, unsigned int buffer_size)
{
	int ret = 0;
	struct usb_request *out_req = sunxi_usb_func->out_req;

	// out_req已经在初始化的时候写死，所以这里不用复原
	// out_req->buf = buffer;
	// out_req->length = buffer_size;
	out_req->actual = 0;
	memset(out_req->buf, 0, out_req->length);

	ret = usb_ep_queue(sunxi_usb_func->out_ep, out_req, 0);
	if (ret)
		printf("[%s] Error %d on queue\n", __func__, ret);

	return ret;
}

static void sunxi_usb_req_tx_complete(struct usb_ep *ep, struct usb_request *req)
{
	if (!req->status) {
		// send data success
		sunxi_usb_print(ep, req);
		sunxi_udev_active->dma_tx_isr(req);
		return;
	}

	__debug("status: %d ep '%s' trans: %d\n", req->status, ep->name, req->actual);
}

static void sunxi_usb_req_rx_complete(struct usb_ep *ep,
				      struct usb_request *req)
{
	if (req->status || req->length == 0) {
		__debug("\n\e[;34m[%s][%d] ERROR status:%d length:%d\e[0m \n",
			__func__, __LINE__, req->status, req->length);
		return;
	}

	if (req->actual <= req->length) {
		sunxi_usb_print(ep, req);
		sunxi_udev_active->dma_rx_isr(req);
	} else {
		__debug("\n\e[;34m[%s][%d] ERROR req->actual:%d req->length:%d\e[0m \n",
			__func__, __LINE__, req->actual, req->length);
	}

	// enqueue request to make rx-data continous
	sunxi_usb_rx_read(req->buf, req->length);
}

/* config the sunxi_usb device*/
static int sunxi_usb_bind(struct usb_configuration *c, struct usb_function *f)
{
	int id;
	struct usb_gadget *gadget = c->cdev->gadget;
	struct f_sunxi_usb *f_sunxi_usb = func_to_sunxi_usb(f);
	// const char *s;

	id = usb_interface_id(c, f);
	if (id < 0)
		return id;
	interface_desc.bInterfaceNumber = id;

	id = usb_string_id(c->cdev);
	if (id < 0)
		return id;

	sunxi_usb_string_defs[0].id = id;
	interface_desc.iInterface = id;

	f_sunxi_usb->in_ep = usb_ep_autoconfig(gadget, &fs_ep_in);
	if (!f_sunxi_usb->in_ep)
		return -ENODEV;
	f_sunxi_usb->in_ep->driver_data = c->cdev;

	f_sunxi_usb->out_ep = usb_ep_autoconfig(gadget, &fs_ep_out);
	if (!f_sunxi_usb->out_ep)
		return -ENODEV;
	f_sunxi_usb->out_ep->driver_data = c->cdev;

	f->descriptors = sunxi_usb_fs_function;

	if (gadget_is_dualspeed(gadget)) {
		hs_ep_in.bEndpointAddress = fs_ep_in.bEndpointAddress;
		hs_ep_out.bEndpointAddress = fs_ep_out.bEndpointAddress;
		f->hs_descriptors = sunxi_usb_hs_function;
	}

	// s = env_get("serial#");
	// if (s)
	// 	g_dnl_set_serialnumber((char *)s);

	return 0;
}

static void sunxi_usb_unbind(struct usb_configuration *c, struct usb_function *f)
{
	/* clear the configuration*/
	memset(sunxi_usb_func, 0, sizeof(*sunxi_usb_func));
}

static void sunxi_usb_disable(struct usb_function *f)
{
	struct f_sunxi_usb *f_sunxi_usb = func_to_sunxi_usb(f);

	usb_ep_disable(f_sunxi_usb->out_ep);
	usb_ep_disable(f_sunxi_usb->in_ep);

	if (f_sunxi_usb->out_req) {
		free(f_sunxi_usb->out_req->buf);
		usb_ep_free_request(f_sunxi_usb->out_ep, f_sunxi_usb->out_req);
		f_sunxi_usb->out_req = NULL;
	}
	if (f_sunxi_usb->in_req) {
		free(f_sunxi_usb->in_req->buf);
		usb_ep_free_request(f_sunxi_usb->in_ep, f_sunxi_usb->in_req);
		f_sunxi_usb->in_req = NULL;
	}
}

static struct usb_request *sunxi_usb_start_ep(struct usb_ep *ep)
{
	struct usb_request *req;

	req = usb_ep_alloc_request(ep, 0);
	if (!req)
		return NULL;

	req->length = SUNXI_USB_REQ_BUFFER_LEN;
	req->buf = memalign(CONFIG_SYS_CACHELINE_SIZE, SUNXI_USB_REQ_BUFFER_LEN);
	if (!req->buf) {
		usb_ep_free_request(ep, req);
		return NULL;
	}
	memset(req->buf, 0, req->length);

	return req;
}

static int sunxi_usb_set_alt(struct usb_function *f, unsigned int interface,
			   unsigned int alt)
{
	int ret;
	struct usb_composite_dev *cdev = f->config->cdev;
	struct usb_gadget *gadget = cdev->gadget;
	struct f_sunxi_usb *f_sunxi_usb = func_to_sunxi_usb(f);
	const struct usb_endpoint_descriptor *d;

	__debug("%s: func: %s intf: %d alt: %d\n",
	      __func__, f->name, interface, alt);

	// init out ep
	d = rkusb_ep_desc(gadget, &fs_ep_out, &hs_ep_out);
	ret = usb_ep_enable(f_sunxi_usb->out_ep, d);
	if (ret) {
		printf("failed to enable out ep\n");
		return ret;
	}

	f_sunxi_usb->out_req = sunxi_usb_start_ep(f_sunxi_usb->out_ep);
	if (!f_sunxi_usb->out_req) {
		printf("failed to alloc out req\n");
		ret = -EINVAL;
		goto err;
	}
	f_sunxi_usb->out_req->complete = sunxi_usb_req_rx_complete;

	// init in ep
	d = rkusb_ep_desc(gadget, &fs_ep_in, &hs_ep_in);
	ret = usb_ep_enable(f_sunxi_usb->in_ep, d);
	if (ret) {
		printf("failed to enable in ep\n");
		goto err;
	}

	f_sunxi_usb->in_req = sunxi_usb_start_ep(f_sunxi_usb->in_ep);
	if (!f_sunxi_usb->in_req) {
		printf("failed alloc req in\n");
		ret = -EINVAL;
		goto err;
	}
	f_sunxi_usb->in_req->complete = sunxi_usb_req_tx_complete;

	// enqueue first request to start rx-data from host
	ret = sunxi_usb_rx_read(f_sunxi_usb->out_req->buf,
				f_sunxi_usb->out_req->length);
	if (ret)
		goto err;

	return 0;
err:
	sunxi_usb_disable(f);
	return ret;
}

static int sunxi_usb_add(struct usb_configuration *c)
{
	struct f_sunxi_usb *f_sunxi_usb = get_sunxi_usb();
	int status;

	debug("%s: cdev: 0x%p\n", __func__, c->cdev);

	f_sunxi_usb->usb_function.name = "f_sunxi_usb";
	f_sunxi_usb->usb_function.bind = sunxi_usb_bind;
	f_sunxi_usb->usb_function.unbind = sunxi_usb_unbind;
	f_sunxi_usb->usb_function.set_alt = sunxi_usb_set_alt;
	f_sunxi_usb->usb_function.disable = sunxi_usb_disable;
	f_sunxi_usb->usb_function.strings = sunxi_usb_strings;

	status = usb_add_function(c, &f_sunxi_usb->usb_function);
	if (status) {
		free(f_sunxi_usb);
		sunxi_usb_func = f_sunxi_usb;
	}
	return status;
}

DECLARE_GADGET_BIND_CALLBACK(usb_dnl_sunxi_usb, sunxi_usb_add);

