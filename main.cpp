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
#include "led.h" 
#define SSD1306_I2C_DEV 0x3c
using namespace std;
void ssd1306_Init(int i2c_fd);
void ssd1306_command(int i2c_fd,uint8_t cmd);
void ssd1306_data(int i2c_fd,const uint8_t* data,size_t size);
void update_full(int i2c_fd,uint8_t* data);
void update_area(int i2c_fd, const uint8_t* data, int x, int y, int x_len, int y_len); 
void update_area_x_wrap(int i2c_fd, const uint8_t* data, int x, int y, int x_len, int y_len);
void handler(int sig);

#define LOGO_WIDTH 50
#define LOGO_HEIGHT 48
#define LOGO_POS_INIT_X 0
#define LOGO_POS_INIT_Y 8
#define LOGO_VELO_INIT_X 3
#define LOGO_VELO_INIT_Y 1
#define LOGO_AXCEL_X 0
#define LOGO_AXCEL_Y 1
#define LOGO_VELO_MAX_X 3
#define LOGO_VELO_MAX_Y 8
#define LOGO_VELO_MIN_X 3
#define LOGO_VELO_MIN_Y 0
// X: 1 ~ 20, accel 1
// Y: 2, no accel

#define MAX_FRAME 999999

typedef struct frame{
    uint8_t x_off, y_off;
    uint8_t from_x, from_y, to_x, to_y;
    uint8_t *data;
	uint8_t *ld, *lu, *rd, *ru; // left, right, up, down
	//uint8_t* frame_data;
} Frame;

int i2c_fd;
Frame* frames;
int total_frames = -1;

void shift_n_bits(uint8_t* target, int x_len, int y_len, int bits){
	int bytes = (bits+7)/8;
	bits = 8*bytes-bits;
	uint8_t mask = 0x00;
	for(int i=0; i<bits; i++){
		mask <<= 1;
		mask |= 1; 
	}
	//printf("%d bytes %d bit, mask %u\n", bytes, bits, mask);

	for(int x=x_len-1; x>=0; x--){
		uint8_t prev = 0;
		for(int y=y_len-1; y>=0; y--){
			uint8_t cur_page = (y>=bytes) ? target[(y-bytes)*x_len+x] : 0;
			uint8_t up_page = (y>=bytes-1) ? prev : 0;
			target[y*x_len+x] = cur_page >> bits;
			target[y*x_len+x] |= (up_page&mask) << (8-bits);
			prev = cur_page;
		}
	}
	/*
	Moving 3 bit
	the last 5 bit of second page should be first 5 bit of second page
	the first 3 bit of [second page] should be the last 3 bit of first page
	the last 5 bit of [first page] should be the first 5 bit of first page
	the first 3 bit of first page should be 0
	*/
}

int pageof(int target){
    return target/8;
}

int main(int argc, char* argv[]) {
    i2c_fd = open("/dev/i2c-1",O_RDWR);
    if(i2c_fd < 0) {
        printf("err opening device\n");
        return -1;
    }
    if(ioctl(i2c_fd, I2C_SLAVE, SSD1306_I2C_DEV)<0) {
        printf("err setting i2c slave address\n");
        return -1;
    }
    ssd1306_Init(i2c_fd);
	
	uint8_t* data_clear = (uint8_t*)calloc(S_WIDTH*S_HEIGHT/8, sizeof(uint8_t));
	update_full(i2c_fd, data_clear);
	printf("Done clear the screen!\n");
	free(data_clear);

	frames = (Frame*) malloc(sizeof(Frame) * MAX_FRAME);
	int cur_x = LOGO_POS_INIT_X;
	int cur_y = LOGO_POS_INIT_Y;
	int cur_vel_x = LOGO_VELO_INIT_X;
	int cur_vel_y = LOGO_VELO_INIT_Y;

	for(int i=0; i<MAX_FRAME; i++) {
        frames[i].x_off = cur_vel_x;
        frames[i].y_off = cur_vel_y + (cur_y) % 8;
        frames[i].from_x = cur_x;
        frames[i].from_y = cur_y;
        cur_x += cur_vel_x;
        cur_y += cur_vel_y;
        cur_vel_x += LOGO_AXCEL_X;
        cur_vel_y += LOGO_AXCEL_Y;
        if (cur_vel_x > LOGO_VELO_MAX_X) cur_vel_x = LOGO_VELO_MIN_X;
        if (cur_vel_y > LOGO_VELO_MAX_Y) cur_vel_y = LOGO_VELO_MIN_Y;

        frames[i].to_x = cur_x + LOGO_WIDTH;
        frames[i].to_y = cur_y + LOGO_HEIGHT;

        cur_x %= S_WIDTH;
        cur_y %= S_HEIGHT;

        if (i &&
            frames[i].x_off == frames[0].x_off &&
            frames[i].y_off == frames[0].y_off &&
            frames[i].from_x == frames[0].from_x &&
            frames[i].from_y == frames[0].from_y &&
            frames[i].to_x == frames[0].to_x &&
            frames[i].to_y == frames[0].to_y
                ) {
            total_frames = i;
            break;
        }

        int now_xlen = (frames[i].to_x-frames[i].from_x);
        int now_ylen = (pageof(frames[i].to_y)-pageof(frames[i].from_y));
        frames[i].data = (uint8_t*) malloc(sizeof(uint8_t) * now_xlen * now_ylen);

        for(int x=0; x<now_xlen; x++){
            for(int y=0; y<now_ylen; y++){
                frames[i].data[x+y*now_xlen] = 0;
            }
        }
        for(int x=0; x<LOGO_WIDTH; x++){
            for(int y=0; y<LOGO_HEIGHT; y++){
                frames[i].data[frames[i].x_off + x+y*now_xlen] = rpi[x+y*LOGO_WIDTH];
            }
        }
        shift_n_bits(frames[i].data, now_xlen, now_ylen, frames[i].y_off);
	}
	if(total_frames == -1){ // could not found total frames
		printf("Cannot calculate #frames in %d!\n", MAX_FRAME);
		goto SAFE_EXIT;
	}
	else{
		printf("#frames: %d\n", total_frames);
		printf("Start to animate...\n");
	}
	
    signal(SIGALRM,handler);
    ualarm(120000,120000);


    while(1) {
        sleep(1);
    }
	
SAFE_EXIT:
	free(frames);
    close(i2c_fd);
    return 0;
}
void ssd1306_command(int i2c_fd, uint8_t cmd) {
    uint8_t buffer[2];
    buffer[0] = (0<<7) | (0<<6); //Co = 0, D/C# = 0
    buffer[1] = cmd;
    if (write(i2c_fd, buffer, 2) != 2) {
        printf("i2c write failed!\n");
    }
}
void ssd1306_Init(int i2c_fd){
    ssd1306_command(i2c_fd, 0xA8);//Set Mux Ratio
    ssd1306_command(i2c_fd, 0x3f);
    ssd1306_command(i2c_fd, 0xD3);//Set Display Offset
    ssd1306_command(i2c_fd, 0x00);
    ssd1306_command(i2c_fd, 0x40);//Set Display Start Line
    ssd1306_command(i2c_fd, 0xA0);//Set Segment re map
                        //0xA1 for vertical inversion
    ssd1306_command(i2c_fd, 0xC0);//Set COM Output Scan Direction
                        //0xC8 for horizontal inversion
    ssd1306_command(i2c_fd, 0xDA);//Set COM Pins hardware configuration
    ssd1306_command(i2c_fd, 0x12);//Manual says 0x2, but 0x12 is required
    ssd1306_command(i2c_fd, 0x81);//Set Contrast Control
    ssd1306_command(i2c_fd, 0x7F);//0:min, 0xFF:max
    ssd1306_command(i2c_fd, 0xA4);//Disable Entire Display On
    ssd1306_command(i2c_fd, 0xA6);//Set Normal Display
    ssd1306_command(i2c_fd, 0xD5);//Set Osc Frequency
    ssd1306_command(i2c_fd, 0x80);
    ssd1306_command(i2c_fd, 0x8D);//Enable charge pump regulator
    ssd1306_command(i2c_fd, 0x14);
    ssd1306_command(i2c_fd, 0xAF);//Display ON
}
void ssd1306_data(int i2c_fd, const uint8_t* data, size_t size) {
    uint8_t* buffer=(uint8_t*)malloc(size+1);
    buffer[0] = (0<<7) | (1<<6); //Co = 0 , D/C# = 1
    memcpy(buffer+1,data,size);
    if (write(i2c_fd, buffer, size+1) != size+1) {
        printf("i2c write failed!\n");
    }
    free(buffer);
}
void update_full(int i2c_fd,uint8_t* data) {
    ssd1306_command(i2c_fd,0x20); //addressing mode
    ssd1306_command(i2c_fd,0x0); //horizontal addressing mode
    ssd1306_command(i2c_fd,0x21); //set column start/end address
    ssd1306_command(i2c_fd,0);
    ssd1306_command(i2c_fd,S_WIDTH-1);
    ssd1306_command(i2c_fd,0x22); //set page start/end address
    ssd1306_command(i2c_fd,0);
    ssd1306_command(i2c_fd,S_PAGES-1);
    ssd1306_data(i2c_fd,data,S_WIDTH*S_PAGES);
}
void update_area(int i2c_fd, const uint8_t* data, int x, int y, int x_len, int y_len) {
    ssd1306_command(i2c_fd,0x20); //addressing mode
    ssd1306_command(i2c_fd,0x0); //horizontal addressing mode
    ssd1306_command(i2c_fd,0x21); //set column start/end address
    ssd1306_command(i2c_fd,x);
    ssd1306_command(i2c_fd,x+x_len-1);
    ssd1306_command(i2c_fd,0x22); //set page start/end address
    ssd1306_command(i2c_fd,y);
    ssd1306_command(i2c_fd,y+y_len-1);
    printf("area %d~%d %d~%d\n", x,x+x_len-1, y,y+y_len-1);

    ssd1306_data(i2c_fd, data, x_len * y_len);
}
void update_frame_area(int i2c_fd, Frame* cur, const unsigned char* logo){
    if(cur->to_x <= S_WIDTH){
        if(pageof(cur->to_y) <= pageof(S_HEIGHT)){
            update_area(i2c_fd, cur->data, cur->from_x, pageof(cur->from_y),
                        cur->to_x-cur->from_x, pageof(cur->to_y)-pageof(cur->from_y));
        }
        else{
            int ulen = pageof(S_HEIGHT) - pageof(cur->from_y);
            int dlen = pageof(cur->to_y) - pageof(S_HEIGHT); // cut and go to down
            int xlen = cur->to_x - cur->from_x;
            uint8_t* up = (uint8_t*)malloc(ulen*xlen);
            uint8_t* down = (uint8_t*)malloc(dlen*xlen);
            for(int x=0; x<xlen; x++){
                for(int y=0; y<ulen; y++){
                    up[xlen*y+x] = cur->data[xlen*y+x];
                }
            }
            for(int x=0; x<xlen; x++){
                for(int y=0; y<dlen; y++){
                    down[xlen*y+x] = cur->data[xlen*(y+ulen)+x];
                }
            }
            update_area(i2c_fd, up, cur->from_x, pageof(cur->from_y), xlen, ulen);
            update_area(i2c_fd, down, cur->from_x, 0, xlen, dlen);
            free(up);
            free(down);
        }
    }
    else{
        if(cur->to_y <= S_HEIGHT){
            int llen = S_WIDTH - cur->from_x;
            int rlen = cur->to_x - S_WIDTH;
            int xlen = cur->to_x - cur->from_x;
            int ylen = pageof(cur->to_y) - pageof(cur->from_y);
            uint8_t* left = (uint8_t*)malloc(ylen*llen);
            uint8_t* right = (uint8_t*)malloc(ylen*rlen);
            for(int x=0; x<llen; x++){
                for(int y=0; y<ylen; y++){
                    left[llen*y+x] = cur->data[xlen*y+x];
                }
            }
            for(int x=0; x<rlen; x++){
                for(int y=0; y<ylen; y++){
                    right[rlen*y+x] = cur->data[xlen*y+ x + llen];
                }
            }
            update_area(i2c_fd, left, cur->from_x, cur->from_y, llen, ylen);
            update_area(i2c_fd, right, 0, cur->from_y, rlen, ylen);
            free(left);
            free(right);
        }
        else{
            int llen = S_WIDTH - cur->from_x;
            int rlen = cur->to_x - S_WIDTH;
            int ulen = pageof(S_HEIGHT) - pageof(cur->from_y);
            int dlen = pageof(cur->to_y) - pageof(S_HEIGHT);
            int xlen = cur->to_x - cur->from_x;
            int ylen = pageof(cur->to_y) - pageof(cur->from_y);
            uint8_t* lu = (uint8_t*)malloc(llen*ulen);
            uint8_t* ru = (uint8_t*)malloc(rlen*ulen);
            uint8_t* ld = (uint8_t*)malloc(llen*dlen);
            uint8_t* rd = (uint8_t*)malloc(rlen*dlen);
            for(int x=0; x<llen; x++){
                for(int y=0; y<ulen; y++){
                    lu[llen*y+x] = cur->data[xlen*y+x];
                }
            }
            for(int x=0; x<rlen; x++){
                for(int y=0; y<ulen; y++){
                    ru[rlen*y+x] = cur->data[xlen*y+(llen+x)];
                }
            }
            for(int x=0; x<llen; x++){
                for(int y=0; y<dlen; y++){
                    ld[llen*y+x] = cur->data[xlen*(y+ulen)+x];
                }
            }
            for(int x=0; x<rlen; x++){
                for(int y=0; y<dlen; y++){
                    rd[rlen*y+x] = cur->data[xlen*(y+ulen)+(llen+x)];
                }
            }
            update_area(i2c_fd, lu, cur->from_x, pageof(cur->from_y), llen, ulen);
            update_area(i2c_fd, ru, 0, pageof(cur->from_y), rlen, ulen);
            update_area(i2c_fd, ld, cur->from_x, 0, llen, dlen);
            update_area(i2c_fd, rd, 0, 0, rlen, dlen);
            free(lu);
            free(ru);
            free(ld);
            free(rd);
        }
    }


}
void update() {
	return;
}
void render(int i) {
    printf("Goes for %d / %d\n", i, total_frames);
    update_frame_area(i2c_fd, &frames[i], rpi);
	return;
}
void handler(int sig) {
	
    static int i = 0;
	update();
	render(i);
    
    i = (i+1)%total_frames;
}
