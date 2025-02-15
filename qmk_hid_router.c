#include "qmk_hid_router.h"

void print_help(){
	printf("QMK RAW HID ROUTER\n\n");
	printf("USAGE:\n");
	printf("--show shows all raw hid devices. Try this first to make sure this program can see them all\n");
	printf("-v shows all packets\n");
	printf("-vv shows all packets and other info\n");
	printf("-r 100 how often to run the loop, 100 means every 100 milliseconds (default 10)\n");
	printf("--no-refresh do not refresh the device list, this causes memory leaks on windows\n");
	printf("--help | -h prints this page\n\n");
	printf("By default will just run the router\n\n");
}

int main(int argc, char* argv[])
{
	signal(SIGINT, signal_handler);
	if (hid_init() < 0) {
        fprintf(stderr, "Failed to initialize HID API\n");
        return 1;
    }

	sleep_duration = DELAY_BETWEEN_LOOP;

 	for (int i = 1; i < argc; i++) {
    	if (strcmp(argv[i], "--show") == 0) {
            print_all_devices();
			hid_exit();
            exit(0);
        }
    	if (strcmp(argv[i], "-v") == 0) {
			log_all_packets=true;
		}
    	if (strcmp(argv[i], "-vv") == 0) {
			log_all_packets=true;
			log_other_stuff=true;
		}
    	if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0) ) {
			print_help();
			hid_exit();
            exit(0);
		}
		if (strcmp(argv[i], "-r") == 0){
			i++;
			if (i < argc) {
                sleep_duration = atoi(argv[i]);
                if (sleep_duration <= 0) {
                    fprintf(stderr, "Error: Sleep duration must be a positive integer.\n");
                    hid_exit();
                    exit(1);
                }
            } else {
                fprintf(stderr, "Error: No value provided for -r option.\n");
                hid_exit();
                exit(1);
            }
		}
    	if (strcmp(argv[i], "--no-refresh") == 0) {
			allow_refresh = false;
		}

    }

	run_router();
	cleanup();
}