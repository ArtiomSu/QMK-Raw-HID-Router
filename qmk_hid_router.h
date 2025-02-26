#ifndef QMK_HID_ROUTER
#define QMK_HID_ROUTER

#include <stdio.h> // printf
#include <wchar.h> // wchar_t
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <hidapi.h>
#include <string.h>
#include <signal.h>

#ifdef _WIN32
	#include <windows.h>
#else
	#include <unistd.h>
	#include <sys/time.h>
#endif

#define DELAY_BETWEEN_LOOP 10 // ms
#define RAW_EPSIZE 32
#define QMK_RAW_USAGE_PAGE 0xFF60
#define QMK_RAW_USAGE 0x61
#define HID_PACKET_HEADER 0xfeed
#define HID_PACKET_PAYLOAD_LEN 23

struct qmk_hid_device
{
	unsigned short vid;
	unsigned short pid;
	char *path;
	hid_device *handle;
};

struct qmk_hid_packet
{
	uint16_t header; // 2 bytes now
	uint16_t to_vid; // 4 bytes now
	uint16_t to_pid; // 6 bytes now
	uint8_t operation; // 7 bytes now
	uint8_t payload_length; // 8 bytes now
	uint8_t payload[HID_PACKET_PAYLOAD_LEN]; 
};

enum operation {
    HID_RAW_OP_INFO = 0xee, // for sending info to pc
};

void print_devices_struct(struct qmk_hid_device *ds);
void print_device(struct hid_device_info *cur_dev);
void print_all_devices();
void find_devices(struct qmk_hid_device **ds);
void print_packet(struct qmk_hid_packet *packet);
void print_packet_compact(struct qmk_hid_packet *packet);
void cleanup();
void signal_handler(int signum);
long long current_time_millis();
void handle_packet(struct qmk_hid_packet *packet, uint16_t index_from);
void run_router();

extern uint16_t qmk_hid_device_count;
extern uint16_t qmk_hid_device_capacity;
extern struct qmk_hid_device *devs;
extern bool loop;
extern bool log_all_packets;
extern bool log_other_stuff;
extern bool allow_refresh;
extern uint32_t sleep_duration;

#endif