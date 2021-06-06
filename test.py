import threading, fcntl, array, time

if __name__ == "__main__":
    with open("/dev/rpikey","w") as rpikey:
        while(True):
            time.sleep(0.1)
            ar = array.array('L', [0,0,0,0])
            fcntl.ioctl(rpikey, 100, ar, 1)
            led = array.array('L', [1^ar[0], 1^ar[1], 1^ar[2]])
            fcntl.ioctl(rpikey, 101, led, 0)
            print(ar)
