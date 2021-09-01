#pragma once

#include <linux/list.h>
#include <linux/usb/ch9.h>

typedef enum ep0_state ep0_state_t;
typedef struct udc udc_t;
typedef struct udc_ep udc_ep_t;
typedef struct udc_req udc_req_t;
typedef struct udc_ops udc_ops_t;
typedef struct udc_gadget udc_gadget_t;

enum ep0_state {
    EP0_WAIT_FOR_SETUP,
    EP0_DATA_STATE_XMIT,
    EP0_DATA_STATE_RECV,
    EP0_WAIT_FOR_COMPLETE,
    EP0_WAIT_FOR_OUT_COMPLETE,
    EP0_WAIT_FOR_IN_COMPLETE,
    EP0_WAIT_FOR_NULL_COMPLETE,
};

struct udc_ep {
    udc_t *udc;
    uint8_t addr;
    uint32_t max_packet;
    int len;
    void *buf;

    unsigned stopped : 1;

    const struct usb_endpoint_descriptor *desc;

    struct list_head queue;
};

struct udc {
    udc_ep_t ep[16];
    ep0_state_t ep0_state;
    struct usb_ctrlrequest *ctrl;

    enum usb_device_speed speed;
    uint16_t ep0_fifo_size;
    uint16_t ep_fifo_size;

    uint8_t clear_feature_num;
    unsigned clear_feature_flag : 1;

    const udc_ops_t *ops;
    udc_gadget_t *gadget;
    void *gadget_data;
};

struct udc_req {
    void *buf;
    size_t length;
    size_t actual;

    void (*complete)(udc_ep_t *ep, udc_req_t *req);
    int status;

    struct list_head queue;
};

struct udc_ops {
    int (*ep_enable)(udc_ep_t *ep, const struct usb_endpoint_descriptor *);
    int (*ep_disable)(udc_ep_t *ep);
    udc_req_t *(*alloc_req)(udc_ep_t *ep);
    void (*free_req)(udc_ep_t *ep, udc_req_t *req);
    int (*queue)(udc_ep_t *ep, udc_req_t *req);
    int (*dequeue)(udc_ep_t *ep, udc_req_t *req);
    int (*set_halt)(udc_ep_t *ep, int value);
};

struct udc_gadget {
    int (*bind)(udc_t *udc);
    void (*unbind)(udc_t *udc);
    int (*setup)(udc_t *udc, struct usb_ctrlrequest *ctrl);
};

int udc_probe(void);
int udc_register_gadget(udc_gadget_t *gadget);
