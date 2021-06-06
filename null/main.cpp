#include <vector>
#include <iostream>
using namespace std;
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
#include "font.h"

int main_init();
void main_finalize();
void handler(int sig);
//global variable
int level=1, day=0, score=10, phaze=0;//day 0 : before start
int old_level = level, old_day=day, old_score=score, old_phaze =phaze;
uint8_t *background[10];
char day_str[20],phaze_str[20],score_str[20]="score",score_str2[10];

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
	if(animation_init()==-1) {
		printf("frame_init_panic\n");
		free(frames); close(i2c_fd);
		return -1;
	}
	if(gpio_init()==-1) {
		printf("led_init_panic\n");
		return -1;
	}
	font_rotate();
	background[0] = (uint8_t*)&level0[0];
	background[1] = (uint8_t*)&level1[0];
	background[2] = (uint8_t*)&level2[0];
	background[3] = (uint8_t*)&level3[0];
	background[4] = (uint8_t*)&level4[0];
	printf("main_init complete\n");
	return 0;
}
void main_finalize() {
	//return 0;
}
void process() {
	if(day==0)
		day++;
	sprintf(day_str,"day %d",day);
	switch(phaze) {
		case 1:
			sprintf(phaze_str,"morning");
			break;
		case 2:
			sprintf(phaze_str,"daytime");
			break;
		default:
			sprintf(phaze_str,"night");
			break;
	}
	sprintf(score_str,"score %d",score);

	return;
}
void save_old_val() {
	old_level = level, old_day= day, old_score = score, old_phaze=phaze;
	return;
}
void display(int i) {
    //printf("Goes for %d / %d\n", i, total_frames);
	update_full(i2c_fd,background[level]);
	write_str(i2c_fd, day_str, S_WIDTH-4, 0);
	write_str(i2c_fd, phaze_str, S_WIDTH-4-8, 0);
	write_str(i2c_fd, score_str, S_WIDTH-4-8*2, 0);
//    update_frame_area(i2c_fd, &frames[i], rpi);
	return;
}
void handler(int sig) {
	
    static int i = 0;
	process();
	display(i);
    
    i = (i+1)%total_frames;
}
