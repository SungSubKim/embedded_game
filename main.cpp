#include <vector>
#include "rpi_logo.h"
#include "skku.h"
#define S_WIDTH 128
#define S_HEIGHT 64
#define S_PAGES (S_HEIGHT/8)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include "gpio.h" 
#include "ssd1306.h"
using namespace std;

int main_init();
void main_finalize();
void handler(int sig);
//global variable
int level=1, day=0, score=10, phaze=1;//day 0 : before start

int main(int argc, char* argv[]) {
	if(main_init()==-1) {
		printf("main_init_panic\n");
		return -1;
	}
    signal(SIGALRM,handler);
    ualarm(120000,120000);

    while(1) {
        sleep(1);
    }
	return 0;
}
int main_init() {//peripheral
	if(ssd1306_main_init()==-1) {
		printf("ssd_1306_second_init_panic\n");
		free(frames); close(i2c_fd);
		return -1;
	}
	if(frame_init()==-1) {
		printf("frame_init_panic\n");
		free(frames); close(i2c_fd);
		return -1;
	}
	if(gpio_init()==-1) {
		printf("led_init_panic\n");
		return -1;
	}
	return 0;
}
void main_finalize() {
	//return 0;
}
void process() {
	return;
}
void display(int i) {
    printf("Goes for %d / %d\n", i, total_frames);
    update_frame_area(i2c_fd, &frames[i], rpi);
	return;
}
void handler(int sig) {
	
    static int i = 0;
	process();
	display(i);
    
    i = (i+1)%total_frames;
}
