#include "qmk_hid_router.h"

uint16_t qmk_hid_device_count = 0;
uint16_t qmk_hid_device_capacity = 2;
struct qmk_hid_device *devs = NULL;
bool loop = true;
bool log_all_packets = false;
bool log_other_stuff = false;

void find_devices(struct qmk_hid_device **ds){
	qmk_hid_device_count = 0;
	struct hid_device_info *cur_dev;

	cur_dev = hid_enumerate(0x0, 0x0);

	unsigned short num_found = 0;

	for (; cur_dev; cur_dev = cur_dev->next) {
		if(
			cur_dev->usage_page != QMK_RAW_USAGE_PAGE &&
			cur_dev->usage != QMK_RAW_USAGE 
		){
			continue;
		}

		for(uint16_t i = 0; i < qmk_hid_device_capacity; i++){
			if(
				!(*ds)[i].handle
			){
				(*ds)[i].path = malloc(strlen(cur_dev->path) + 1); // +1 for null terminator
                if ((*ds)[i].path == NULL) {
                    fprintf(stderr,"Failed to allocate memory for path");
					continue;
                }

                // Copy the path from cur_dev to ds[i]
                strcpy((*ds)[i].path, cur_dev->path);
				(*ds)[i].vid = cur_dev->vendor_id;
				(*ds)[i].pid = cur_dev->product_id;
				num_found++;
//#ifdef _WIN32
//				(*ds)[i].handle = hid_open((*ds)[i].vid, (*ds)[i].pid, NULL);
//#else
				(*ds)[i].handle = hid_open_path((*ds)[i].path);
//#endif
				if(!(*ds)[i].handle){
					fprintf(stderr,"Unable to open device %02X:%02X %s\n", (*ds)[i].pid, (*ds)[i].vid, cur_dev->product_string);
				}
				hid_set_nonblocking((*ds)[i].handle, 1);
				qmk_hid_device_count++;
				//printf("%d added %02X %02X\n", i, (*ds)[i].vid, (*ds)[i].pid);
				break;
			}
		}
		if(num_found >= qmk_hid_device_capacity){
			//printf("exceeded capacity will try to increase\n");
			qmk_hid_device_capacity = qmk_hid_device_capacity + 4;
			struct qmk_hid_device *temp = (struct qmk_hid_device *)realloc(*ds, qmk_hid_device_capacity * sizeof(struct qmk_hid_device));
			if(temp == NULL){
				fprintf(stderr, "failed to increase capacity, so no more new devices");
				qmk_hid_device_capacity = qmk_hid_device_capacity - 4;
				break;
			}
			//printf("pointer of ds is %p\n", (void *)*ds);
			//printf("pointer of temp is %p\n", (void *)temp);
			*ds = temp;
			memset(*ds + num_found, 0, (qmk_hid_device_capacity - num_found) * sizeof(struct qmk_hid_device));
			//printf("pointer of ds after is %p\n", (void *)*ds);
			//printDevices(*ds);
		}
	}
	//printf("num devices found %d\n", qmk_hid_device_count);
	hid_free_enumeration(cur_dev);
}

void handle_packet_for_pc(struct qmk_hid_packet *packet, uint16_t index_from){
	//info opeation
	if(packet->operation == HID_RAW_OP_INFO){
		printf("INFO: From %02X:%02X %s\n", devs[index_from].vid, devs[index_from].pid, packet->payload);
	}
}

void handle_packet(struct qmk_hid_packet *packet, uint16_t index_from){
	if(log_all_packets){
		printf("Received Packet:");
		print_packet_compact(packet);    
	}
	
	if(packet->header != HID_PACKET_HEADER){
		return;
	}

	if(packet->to_pid == 0x00 && packet->to_vid == 0x00){
		return handle_packet_for_pc(packet, index_from);
	}

	bool found = false;
	bool is_broadcast = packet->to_pid == 0xffff && packet->to_vid == 0xffff;
	for(int i=0; i<qmk_hid_device_count; i++){
		if(
			is_broadcast ||
			(devs[i].pid == packet->to_pid && devs[i].vid == packet->to_vid )
		){
			if(i == index_from && is_broadcast){
				continue;
			}
			if(!devs[i].handle){
				if(is_broadcast){
					continue;
				}
				break;	
			}
			found = true;

			if(!is_broadcast){
				packet->to_pid = devs[index_from].pid;
				packet->to_vid = devs[index_from].vid;
			}
#ifdef _WIN32
			struct qmk_hid_packet pack;
			pack.header = packet->header;
			pack.to_pid = packet->to_pid;
			pack.to_vid = packet->to_vid; 
			pack.operation = packet->operation;
			pack.payload_length = packet->payload_length;

			for(int i=0; i<HID_PACKET_PAYLOAD_LEN -2; i++){
				//buf[i] = packet->payload[i];
				pack.payload[i] = packet->payload[i];
			}

			uint8_t buff[RAW_EPSIZE+1];
			uint8_t *squash = (uint8_t *)&pack;

			buff[0]=0x0; // report id must be the very first thing. only for windows I think. This was the main problem for me anyway..
			for(int i=1; i<RAW_EPSIZE+1; i++){
				buff[i] = squash[i-1];
			}

	
			//printf("Sending Packet:\n");
			//print_packet(&pack);    
			int res = hid_write(devs[i].handle, (unsigned char *)&buff, RAW_EPSIZE+1);
#else

			//printf("Sending Packet from %02X:%02X\n", devs[index_from].pid, devs[index_from].vid);
			//print_packet(packet);    
			int res = hid_write(devs[i].handle, (unsigned char *)packet, RAW_EPSIZE);
#endif

			if (res < 0) {
				fprintf(stderr, "Unable to write(): %ls\n", hid_error(devs[i].handle));
			}
			if(!is_broadcast){
				break;
			}
		}
	}

	if(!found && !is_broadcast){
		fprintf(stderr,"Cannot write to device %02X:%02X as it is closed\n", packet->to_vid, packet->to_pid);
	}
}

void run_router(){
    int i, res;
	uint16_t refresh = 0;
	unsigned char buf[RAW_EPSIZE];
	devs = malloc(qmk_hid_device_capacity * sizeof(struct qmk_hid_device));
	memset(devs, 0, qmk_hid_device_capacity * sizeof(struct qmk_hid_device));


	find_devices(&devs);
	if(log_other_stuff){
		print_devices_struct(devs);
	}

	while (loop){
		for(i=0; i<qmk_hid_device_count; i++){
			if(!loop){
				break;
			}
			if(devs[i].handle){
				res = hid_read(devs[i].handle, buf, RAW_EPSIZE);
				if (res < 0) {
					fprintf(stderr, "Unable to read(): %ls\n", hid_error(devs[i].handle));
					hid_close(devs[i].handle);
					devs[i].handle = NULL;
				}
				else if (res > 0) {
					struct qmk_hid_packet *packet = (struct qmk_hid_packet *)buf;
					handle_packet(packet, i);
				}
			}
		}
#ifdef _WIN32
		Sleep(DELAY_BETWEEN_LOOP);
#else
		usleep(DELAY_BETWEEN_LOOP*1000);
#endif
		refresh++;
		if(refresh > REFRESH_DEVICES_INTERVAL){
			if(log_other_stuff){
				printf("Refreshing devices\n");
			}
			refresh = 0;
			for (int i = 0; i < qmk_hid_device_count; i++) {
				free(devs[i].path);
				if (devs[i].handle) {
					hid_close(devs[i].handle);
				}
			}
			memset(devs, 0, qmk_hid_device_capacity * sizeof(struct qmk_hid_device));
			find_devices(&devs);
			//printDevices(devs);
		}
	}
}

