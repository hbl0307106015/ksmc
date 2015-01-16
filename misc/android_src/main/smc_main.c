/*
*    smart machine controller
*    Copyright (C) <2015>  <VIA>
*
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <errno.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <libusb.h>

#define RC1180_VENDOR_ID	0x0403
#define RC1180_PRODUCT_ID	0x6001

#define ENDPOINT_IN     (LIBUSB_ENDPOINT_IN|0x01)
#define ENDPOINT_OUT    (LIBUSB_ENDPOINT_OUT|0x02)

#define MAX_APPL_DATA_SIZE 126

struct appl_data_header_t {
    uint16_t length; /* totoal length of data frame without its self bytes */
    uint8_t knx_ctrl; /* KNX control */
    uint16_t src; /* source address */
    uint16_t dst; /* destination address */
    uint8_t lnpci; /* link/network layer protocol control information */
    uint16_t tpci_apci; /* transport layer protocol & application layer protocol control information */
};

struct appl_data_t {
    struct appl_data_header_t hdr; /* application data header */
    unsigned char user_data[MAX_APPL_DATA_SIZE]; /* user data */
};

static struct libusb_device_handle *devh = NULL;

static int find_radiocrafts_device(void)
{
	devh = libusb_open_device_with_vid_pid(NULL, RC1180_VENDOR_ID, RC1180_PRODUCT_ID);
	return (devh ? (0) : (-EIO));
}

static int assemble_appl_data_header_test(struct appl_data_t *data, int len)
{
    if (len <= 0) {printf("len error.\n"); return -1;}

    data->hdr.length = 0xB; /* pre-edit length */
    data->hdr.knx_ctrl = 0x0; /* knx controll */
    data->hdr.src = 0x1; /* src address for test */
    data->hdr.dst = 0x2; /* dst address for test */
    data->hdr.lnpci = 0x61; /* l/npci */
    data->hdr.tpci_apci = 0x0; /* tpci/apci */

    printf("data header assembled\n");
    return 0;
}

static int assemble_appl_data_msg_test(struct appl_data_t *data, int len)
{
    if (len <= 0) {printf("len error.\n"); return -1;}
    
    data->user_data[0] = 0xAA;
    data->user_data[1] = 0xBB;
    data->user_data[2] = 0xCC;
    data->user_data[3] = 0x3F;

    printf("user data assembled\n");
    return 0;
}

static int assemble_transmit_data_test(struct appl_data_t *data, int len)
{
    if (len <= 0) {printf("len error.\n"); return -1;}

    assemble_appl_data_header_test(data, len);
    assemble_appl_data_msg_test(data, len);

    return 0;
}

static void store_byte8(unsigned char **dst, unsigned char src)
{ 
    memcpy(*dst, &src, 1);
    (*dst)++;
}

static void store_byte16(unsigned char **dst, uint16_t src)
{ 
    memcpy(*dst, &src, 2);
    (*dst) += 2;
}

static void store_byte32(unsigned char **dst, uint32_t src)
{
    memcpy(*dst, &src, 4);
    (*dst) += 4;
}

static void store_raw_bytes(unsigned char **dst, unsigned char *src, int len)
{
    memcpy(*dst, src, len);
    (*dst) += len;
}

static int transfer_appl_data_to_buffer(struct appl_data_t *data, unsigned char *buffer, int len)
{
    unsigned char *pos = NULL;
    pos = buffer;
    
    /* header */
    printf("transfering appl_data...\n");
    #if 0
    store_byte8(&pos, data->hdr.length);
    printf("length:%02x\n", data->hdr.length);
    store_byte8(&pos, data->hdr.knx_ctrl);
    printf("knx_ctrl:%02x\n", data->hdr.knx_ctrl);
    #endif
    store_byte16(&pos, htons(data->hdr.src));
    printf("src:%04x\n", data->hdr.src);
    store_byte16(&pos, htons(data->hdr.dst));
    printf("dst:%04x\n", data->hdr.dst);
    store_byte8(&pos, data->hdr.lnpci);
    printf("lnpci:%02x\n", data->hdr.lnpci);
    store_byte16(&pos, htons(data->hdr.tpci_apci));
    printf("tpci_apci:%04x\n", data->hdr.tpci_apci);

    /* user data */
    store_raw_bytes(&pos, data->user_data, 4); /* make 10 bytes transfered for test */

    printf("user_data:%02x %02x %02x\n", \
           data->user_data[0], \
           data->user_data[1], \
           data->user_data[2]);

    printf("%d data bytes appl_data transfered to buffer\n", pos - buffer);
    return 0;
}

int main(int argc, char *argv[])
{
	int r = 1;

	r = libusb_init(NULL);
	if (r < 0) {
		fprintf(stderr, "failed to initialise libusb\n");
		exit(1);
	}

	r = find_radiocrafts_device();
	if (r < 0) {
		fprintf(stderr, "Could not find/open device\n");
		goto out;
	}

    r = libusb_kernel_driver_active(devh, 0);
    if (r == 1) {
        r = libusb_detach_kernel_driver(devh, 0);
	    if (r < 0) {
		    fprintf(stderr, "libusb_detach_kernel_driver error %d\n", r);
	    	goto out;
        }
    }

	r = libusb_claim_interface(devh, 0);
	if (r < 0) {
		fprintf(stderr, "usb_claim_interface error %d\n", r);
		goto out;
	}
	printf("claimed interface\n");

    int actual_length = 0;
    struct appl_data_t data;
    unsigned char buffer_w[MAX_APPL_DATA_SIZE] = {0};
    unsigned char buffer_r[MAX_APPL_DATA_SIZE] = {0};
    unsigned char endc[2] = {0x0, 0x3f};

    assemble_transmit_data_test(&data, sizeof(data));
    transfer_appl_data_to_buffer(&data, buffer_w, sizeof(data));

    int k = 0;
    for (k = 0; k < 32; k++)
        printf("%02x ", buffer_w[k]);
    printf("\n");

	/* begin I/O */
	while (1) {
		/* write something to the device */
		r = libusb_bulk_transfer(devh,
			LIBUSB_ENDPOINT_OUT | 0x02,
			buffer_w,
			sizeof(buffer_w),
            &actual_length,
			0);
		printf("return type=%d. bytes written=%d.\n", r, actual_length);
        #if 0
		/* write something to the device */
		r = libusb_bulk_transfer(devh,
			LIBUSB_ENDPOINT_OUT | 0x02,
			endc,
			sizeof(endc),
            &actual_length,
			0);
		printf("return type=%d. bytes written=%d.\n", r, actual_length);
        #endif
        #if 0
		/* read something from the device */
		r = libusb_bulk_transfer(devh,
			LIBUSB_ENDPOINT_IN | 0x00,
			buffer_r,
			sizeof(buffer_r),
            &actual_length,
			0);
		printf("return type=%d. bytes read=%d content=%d.\n", r, actual_length, buffer_r[0]);
        #endif
		/* have a bit break */
		sleep(2.1);
	}
	
out_release:
    libusb_attach_kernel_driver(devh, 0);
	libusb_release_interface(devh, 0);
out:
	libusb_close(devh);
	libusb_exit(NULL);
	return r >= 0 ? r : -r;
}
