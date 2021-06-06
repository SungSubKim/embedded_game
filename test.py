import threading, fcntl, array, time

if __name__ == "__main__":
    with open("/dev/rpikey","w") as rpikey:
        lastval = 0
        while(True):
            time.sleep(0.5)
            ar = array.array('L', [0,0,0,0])
            fcntl.ioctl(rpikey, 100, ar, 1)
            led = array.array('L', [1^ar[0], 1^ar[1], 1^ar[2]])
            fcntl.ioctl(rpikey, 101, led, 0)
            val = (ar[0] << 3) + (ar[1] << 2) + (ar[2] << 1) + (ar[3] << 0)
            print(ar, val)
            if lastval != val:
                lastval = val
                ar1 = array.array('Q', [x%4==0 and val or 0 for x in range(128)])
                print(ar1)
                fcntl.ioctl(rpikey, 200, ar1, 0)
