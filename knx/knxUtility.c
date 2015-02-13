#include "knxUtility.h"

bool is_valid_length(ssize_t ret, int8_t len_byte)
{
	return ((ret == KNX_LENGTH_BYTES) && (len_byte > 0));
}

bool is_valid_knx_buffer(ssize_t ret)
{
	return (ret > 0);
}

ssize_t knxRead(int fd, void *b, size_t l)
{
	return read(tty_fd, b, l);
}

ssize_t knxReadLengthByte(int fd, void *b)
{
	return read(tty_fd, b, KNX_LENGTH_BYTES);
}

void knxHandleUARTBuffer(int fd)
{
	int16_t cnt = 0;
	int8_t len_byte = 0;;
	uint8_t buffer[KNX_BUFFER_SIZE] = {0};
	ssize_t buffer_len = 0;
	
	ret = knxReadLengthByte(fd, (void *)&len_byte);
	
	if (is_valid_length(ret, len_byte)) {
	// first byte's value, which indicates the length of the buffer, is valid.

		fprintf(stdout, "knx buffer from uart length=%d, count=%d\n", len_byte, (++cnt));	

		buffer_len = knxRead(fd, (void *)buffer, len_byte);
		
		if (is_valid_knx_buffer(buffer_len)) {
			fprintf(stdout, "knx buffer from uart is handled correctly\n");
			knx_protocol_store_packet(QUEUE_INDEX_KNX_RX, buffer, buffer_len);
		} else {
			fprintf(stdout, "read knx buffer error\n");
			goto out;
		}

	// first byte is invalid value
	} else {
		fprintf(stdout, "length byte error, ret=%d val=%d\n", ret, len_byte);
		goto out;
	}
	
out:
	return;
}
