const unsigned char font[96][7] = {
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00}, //  
	{0x7f,0x51,0x7f,0x00,0x00,0x00,0x00}, // !
	{0x0f,0x09,0x0f,0x09,0x0f,0x00,0x00}, // "
	{0x3e,0x6b,0x41,0x6b,0x41,0x6b,0x3e}, // #
	{0x0e,0x7b,0x55,0x41,0x55,0x6f,0x38}, // $
	{0x77,0x5d,0x6f,0x36,0x7b,0x4d,0x77}, // %
	{0x3e,0x6b,0x51,0x55,0x53,0x6e,0x54}, // &
	{0x0f,0x09,0x0f,0x00,0x00,0x00,0x00}, // '
	{0x3e,0x63,0x5d,0x77,0x00,0x00,0x00}, // (
	{0x77,0x5d,0x63,0x3e,0x00,0x00,0x00}, // )
	{0x3e,0x2a,0x36,0x2a,0x3e,0x00,0x00}, // *
	{0x1c,0x36,0x22,0x36,0x1c,0x00,0x00}, // +
	{0xf0,0x90,0xf0,0x00,0x00,0x00,0x00}, // ,
	{0x1c,0x14,0x14,0x14,0x1c,0x00,0x00}, // -
	{0x70,0x50,0x70,0x00,0x00,0x00,0x00}, // .
	{0x78,0x4e,0x73,0x1d,0x07,0x00,0x00}, // /
	{0x3e,0x63,0x41,0x4d,0x4d,0x61,0x3f}, // 0
	{0x07,0x7d,0x41,0x7f,0x00,0x00,0x00}, // 1
	{0x7f,0x4d,0x55,0x55,0x5b,0x7e,0x00}, // 2
	{0x7f,0x55,0x55,0x55,0x69,0x3f,0x00}, // 3
	{0x1f,0x31,0x6f,0x6f,0x41,0x6f,0x38}, // 4
	{0x7f,0x51,0x55,0x55,0x6d,0x3f,0x00}, // 5
	{0x7e,0x43,0x45,0x55,0x55,0x6d,0x3f}, // 6
	{0x0f,0x09,0x7d,0x5d,0x6d,0x31,0x1f}, // 7
	{0x3e,0x2a,0x55,0x55,0x55,0x69,0x3f}, // 8
	{0x0e,0x7b,0x51,0x55,0x55,0x61,0x3f}, // 9
	{0x7e,0x42,0x7e,0x00,0x00,0x00,0x00}, // :
	{0xfe,0x82,0xfe,0x00,0x00,0x00,0x00}, // ;
	{0x1c,0x36,0x6b,0x5d,0x77,0x00,0x00}, // <
	{0x3e,0x2a,0x2a,0x2a,0x3e,0x00,0x00}, // =
	{0x77,0x5d,0x6b,0x36,0x1c,0x00,0x00}, // >
	{0x07,0x7d,0x55,0x75,0x1b,0x0e,0x00}, // ?
	{0x7e,0x42,0x7d,0x65,0x55,0x55,0x61}, // @
	{0x7e,0x43,0x6d,0x2d,0x6d,0x41,0x7f}, // A
	{0x7e,0x43,0x41,0x45,0x51,0x6f,0x38}, // B
	{0x3e,0x63,0x5d,0x55,0x55,0x77,0x00}, // C
	{0x7e,0x43,0x4d,0x4d,0x5d,0x61,0x3f}, // D
	{0x7e,0x43,0x55,0x55,0x55,0x7f,0x00}, // E
	{0x7e,0x43,0x6d,0x2d,0x2d,0x3f,0x00}, // F
	{0x7e,0x43,0x4d,0x4d,0x55,0x65,0x3f}, // G
	{0x7f,0x41,0x77,0x77,0x41,0x7f,0x00}, // H
	{0x77,0x55,0x41,0x5d,0x75,0x00,0x00}, // I
	{0x78,0x41,0x4d,0x5d,0x61,0x3f,0x00}, // J
	{0x7f,0x41,0x77,0x73,0x4d,0x7f,0x00}, // K
	{0x7f,0x41,0x5f,0x50,0x50,0x70,0x00}, // L
	{0x7e,0x43,0x7d,0x7d,0x41,0x7d,0x7d}, // M
	{0x7f,0x43,0x7d,0x05,0x7d,0x41,0x7f}, // N
	{0x3e,0x63,0x5d,0x5d,0x5d,0x61,0x3f}, // O
	{0x7e,0x43,0x6d,0x2d,0x2d,0x31,0x1f}, // P
	{0x3e,0x63,0x5d,0xdd,0x9d,0xe1,0x3f}, // Q
	{0x7e,0x43,0x6d,0x6d,0x4d,0x71,0x1f}, // R
	{0x7e,0x5b,0x55,0x55,0x6d,0x3f,0x00}, // S
	{0x07,0x05,0x7d,0x41,0x7d,0x05,0x07}, // T
	{0x3f,0x61,0x5f,0x50,0x5f,0x61,0x3f}, // U
	{0x1f,0x31,0x6f,0x58,0x68,0x31,0x1f}, // V
	{0x7f,0x41,0x5f,0x5f,0x61,0x41,0x5f}, // W
	{0x7f,0x49,0x77,0x77,0x49,0x7f,0x00}, // X
	{0x7f,0x59,0x57,0x57,0x61,0x3f,0x00}, // Y
	{0x7f,0x4d,0x55,0x55,0x59,0x7f,0x00}, // Z
	{0x7f,0x41,0x5d,0x55,0x00,0x00,0x00}, // [
	{0x07,0x1d,0x73,0x4e,0x78,0x00,0x00}, // "\"
	{0x77,0x5d,0x41,0x7f,0x00,0x00,0x00}, // ]
	{0x0e,0x0b,0x0d,0x0b,0x0e,0x00,0x00}, // ^
	{0x70,0x50,0x50,0x50,0x70,0x00,0x00}, // _
	{0x07,0x0d,0x0b,0x0e,0x00,0x00,0x00}, // `
	{0x7e,0x03,0x6d,0x2d,0x6d,0x01,0x7f}, // a
	{0x7e,0x43,0x41,0x55,0x51,0x6f,0x38}, // b
	{0x3e,0x63,0x5d,0x55,0x55,0x77,0x00}, // c
	{0x7e,0x43,0x41,0x4d,0x4d,0x61,0x3f}, // d
	{0x7e,0x43,0x55,0x55,0x55,0x7f,0x00}, // e
	{0x7e,0x43,0x6d,0x2d,0x2d,0x3f,0x00}, // f
	{0x7e,0x43,0x5d,0x55,0x45,0x65,0x3f}, // g
	{0x7f,0x41,0x77,0x77,0x41,0x7f,0x00}, // h
	{0x77,0x5d,0x41,0x5d,0x77,0x00,0x00}, // i
	{0x78,0x4f,0x5d,0x5d,0x61,0x3f,0x00}, // j
	{0x7f,0x00,0x77,0x71,0x0c,0x7f,0x00}, // k
	{0x7f,0x41,0x5f,0x50,0x50,0x70,0x00}, // l
	{0x7e,0x43,0x7d,0x7d,0x43,0x7d,0x7d}, // m
	{0x7f,0x43,0x7d,0x05,0x7d,0x41,0x7f}, // n
	{0x3e,0x63,0x41,0x4d,0x4d,0x61,0x3f}, // o
	{0x7e,0x43,0x65,0x25,0x25,0x31,0x1f}, // p
	{0x3e,0x63,0x41,0xcd,0x8d,0xe1,0x3f}, // q
	{0x7e,0x43,0x61,0x65,0x45,0x71,0x1f}, // r
	{0x7e,0x5b,0x55,0x55,0x6d,0x3f,0x00}, // s
	{0x07,0x05,0x7d,0x41,0x7d,0x05,0x07}, // t
	{0x3f,0x61,0x5f,0x50,0x5f,0x61,0x3f}, // u
	{0x1f,0x31,0x6f,0x58,0x6f,0x31,0x1f}, // v
	{0x7f,0x41,0x5f,0x5f,0x61,0x5f,0x5f}, // w
	{0x7f,0x49,0x77,0x77,0x49,0x7f,0x00}, // x
	{0x7f,0x59,0x57,0x57,0x61,0x3f,0x00}, // y
	{0x7f,0x4d,0x55,0x55,0x59,0x7f,0x00}, // z
	{0x1c,0x77,0x41,0x5d,0x77,0x00,0x00}, // {
	{0x7f,0x41,0x7f,0x00,0x00,0x00,0x00}, // |
	{0x77,0x5d,0x41,0x77,0x1c,0x00,0x00}, // }
	{0x06,0x02,0x05,0x01,0x01,0x00,0x00}, // ~
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00}
};

#define FONT_NAME "Minimum+1_font"
#define FONT_WIDTH 6
#define FONT_HEIGHT 1 //1 page
void write_char(int i2c_fd, char c, int x, int y) {
	if(c < ' ') c = ' ';
	update_area(i2c_fd,font[c-' '],x,y,FONT_WIDTH,FONT_HEIGHT);
}
void write_str(int i2c_fd, char* str, int x, int y) {
	char c;
	while (c = *str++) {
		write_char(i2c_fd,c,x,y);
		x += FONT_WIDTH;
	}
}
