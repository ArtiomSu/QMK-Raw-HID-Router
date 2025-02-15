#include "qmk_hid_router.h"

void print_devices_struct(struct qmk_hid_device *ds){
	for(unsigned short i = 0; i < qmk_hid_device_count; i++){
		printf("vid: %x pid: %x path: %s\n", ds[i].vid, ds[i].pid, ds[i].path);
	}
}

void print_device(struct hid_device_info *cur_dev) {
	printf("[vid] %04hx [pid] %04hx [path] %s ", cur_dev->vendor_id, cur_dev->product_id, cur_dev->path);
	printf("[Manuf] %ls ", cur_dev->manufacturer_string);
	printf("[Prod] %ls ", cur_dev->product_string);
	printf("[Release] %hx ", cur_dev->release_number);
	printf("[Usage] 0x%hx [Page] 0x%hx", cur_dev->usage, cur_dev->usage_page);
	printf("\n\n");
}

void print_all_devices(){
	struct hid_device_info *cur_dev;
	cur_dev = hid_enumerate(0x0, 0x0);
	for (; cur_dev; cur_dev = cur_dev->next) {
		print_device(cur_dev);
	}
	hid_free_enumeration(cur_dev);
}

void print_packet(struct qmk_hid_packet *packet){
	printf("header: %X\n", packet->header);
	printf("vid: %X\n", packet->to_vid);
	printf("pid: %X\n", packet->to_pid);
	printf("operation: %X\n", packet->operation);
	printf("payload_length: %X\n", packet->payload_length);
	printf("payload: ");
	for (int i = 0; i < packet->payload_length; i++){
		printf("%02X", packet->payload[i]);
	}
	printf("\n\n");
}

void print_packet_compact(struct qmk_hid_packet *packet){
	printf("[header] %X ", packet->header);
	printf("[vid] %X ", packet->to_vid);
	printf("[pid] %X ", packet->to_pid);
	printf("[operation] %X ", packet->operation);
	printf("[payload_length] %X ", packet->payload_length);
	printf("[payload] ");
	for (int i = 0; i < packet->payload_length; i++){
		printf("%02X", packet->payload[i]);
	}
	printf("\n");
}

void cleanup() {
    for (int i = 0; i < qmk_hid_device_count; i++) {
        free(devs[i].path);
        if (devs[i].handle) {
            hid_close(devs[i].handle);
        }
    }
    free(devs);
    hid_exit();
    printf("Cleanup done. Exiting program.\n");
}

void signal_handler(int signum) {
    printf("Caught signal %d (Ctrl+C). Cleaning up...\n", signum);
	loop = false;
    cleanup();
    exit(0);
}

long long current_time_millis() {
#ifdef _WIN32
	return GetTickCount64();
#else
	struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000LL) + (tv.tv_usec / 1000);
#endif
}