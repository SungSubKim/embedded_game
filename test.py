import threading, fcntl, array, time

if __name__ == "__main__":
    with open("/dev/rpikey","w") as rpikey:
        while(True):
            time.sleep(0.1)
            ar = array.array('L', [0,0,0,0])
            fcntl.ioctl(rpikey, 100, ar, 1)
            print(ar)
