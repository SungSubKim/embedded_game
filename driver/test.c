#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#define AM2320_I2C_DEV 0x5C
void am2320_read_4b(int i2c_fd, uint8_t addr);

int main(int argc, char *argv[])
{
    int i2c_fd = open("/dev/i2c-1", O_RDWR);
    if (i2c_fd < 0)
    {
        printf("err opening device\n");
        return -1;
    }

    if (ioctl(i2c_fd, I2C_SLAVE, AM2320_I2C_DEV) < 0)
    {
        printf("err setting i2c slave address\n");
        return -1;
    }
    am2320_read_4b(i2c_fd, 0x0);
    close(i2c_fd);

    return 0;
}
void am2320_read_4b(int i2c_fd, uint8_t addr)
{
    int res;
    uint8_t buffer[10] = {0};
    struct timespec time_before, time_after, time_diff;

    //step 1: Wakeup sensor
    while (1)
    {
        clock_gettime(CLOCK_MONOTONIC, &time_before);
        write(i2c_fd, buffer, 1);
        usleep(800);
        clock_gettime(CLOCK_MONOTONIC, &time_after);
        time_diff.tv_sec = time_after.tv_sec - time_before.tv_sec;
        time_diff.tv_nsec = time_after.tv_nsec - time_before.tv_nsec;
        long long diff_nsec =
            time_diff.tv_sec * 1e9 + time_diff.tv_nsec;
        if (diff_nsec < 3000000)
            break;
    }
    //step 2: Send read command for 4-byte read from ‘addr’
    buffer[0] = 0x3;
    buffer[1] = addr;
    buffer[2] = 0x4;
    if ((res = write(i2c_fd, buffer, 3)) != 3)
    {
        printf("i2c write failed! %d\n", res);
    }
    //step 3: Read Data
    usleep(1500);
    if (read(i2c_fd, buffer, 8) != 8)
    {
        printf("i2c read failed!\n");
    }

    uint16_t hum = buffer[2] << 8 | buffer[3];
    uint16_t temp = buffer[4] << 8 | buffer[5];
    printf("Humidity %d.%01d%% Temperature %d.%01dc\n", hum / 10, hum % 10, temp / 10, temp % 10);
}
