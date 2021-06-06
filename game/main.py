from conv import update_area
import fcntl,array,random,time

tact =[0,0,0,0]
old_tact = list(tact)
level, day, score, phaze = 0,0,5,0
background = ["seed","kids","child","adult","sakura"]
phaze_str= ["moring","daytime","night"]
old_level, old_day, old_score, old_phaze = level, day,score,phaze
water_chance= True
last_water = 0
event= []
event_chances = [0,0,0,0]
last_sudden_event = 0
led_event_idx = 0
led_last_changed = 0
current_animation = None
animation_start_time = 0
animation_duration = 0

rpikey = open("/dev/rpikey","w")
display_items =[]
selected =0

COLOR_WHITE   = [1,1,1]
COLOR_OFF     = [0,0,0]
COLOR_RED     = [1,0,0]
COLOR_GREEN   = [0,1,0]
COLOR_BLUE    = [0,0,1]
COLOR_CYAN    = [0,1,1]
COLOR_MAGENTA = [1,0,1]
COLOR_YELLOW  = [1,1,0]

EVENT_WATER      = "water"
EVENT_SUN        = "sun"
EVENT_BUG        = "bug"
EVENT_FERTILIZER = "fertilizer"

EVENT_TO_COLOR = {
    EVENT_WATER     : COLOR_BLUE,
    EVENT_SUN       : COLOR_RED,
    EVENT_BUG       : COLOR_MAGENTA,
    EVENT_FERTILIZER: COLOR_YELLOW,
}

IDX_TO_EVENT = {
    0: EVENT_WATER,
    1: EVENT_SUN,
    2: EVENT_BUG,
    3: EVENT_FERTILIZER,
}

EVENT_TO_IDX = {
    EVENT_WATER      :0,
    EVENT_SUN        :1,
    EVENT_BUG        :2,
    EVENT_FERTILIZER :3,
}

class display_item():
    def __init__(self,x,y,img_name):
        self.x = x
        self.y = y
        self.img_name = img_name

class event_item():
    def __init__(self, type, only_on_day=-1, duration=60):
        self.type = type
        if only_on_day == -1:
            self.exp = time.time()+duration
            self.day = None
        else:
            self.day = only_on_day
            self.exp = None
    
    def is_expired(self, day=-1):
        if self.day != None:
            return (day != self.day)
        else:
            return time.time() > self.exp


rpikey = open("/dev/rpikey","w")
display_items =[]
selected =0
def process():
    global day, phaze, water_chance, score, last_water, \
        level, selected, event, event_chances, display_items, \
        current_animation, animation_start_time, animation_duration, old_day, \
        last_sudden_event

    display_items.clear()
    
    display_items.append(display_item(0,      128 - 16, "bot1.bmp"))
    display_items.append(display_item(16,     128 - 16, "bot2.bmp"))
    display_items.append(display_item(16 * 2, 128 - 16, "bot3.bmp"))
    display_items.append(display_item(16 * 3, 128 - 16, "bot4.bmp"))

    event[:] = [e for e in event if e.is_expired(day) == False]

    if day==0:
        print('game_start')
        day+=1

    if current_animation != None:
        display_items.append(display_item(16 * 2,0,  f"ani{selected+1}.bmp"))
        if time.time() - animation_start_time >= animation_duration:
            current_animation = None

    elif tact[0] == 1 and old_tact[0] == 0:
        selected = (selected-1) % 4

    elif tact[1] == 1 and old_tact[1] == 0:
        selected = (selected + 1) % 4

    elif tact[2] == 1 and old_tact[2] == 0:
        print(f"OK button pressed with {selected+1}")
        if IDX_TO_EVENT[selected] in [e.type for e in event]:
            print(f"Delete event {IDX_TO_EVENT[selected]}!")
            current_animation = IDX_TO_EVENT[selected]
            del event[[e.type for e in event].index(IDX_TO_EVENT[selected])]
            animation_start_time = time.time()
            animation_duration = 2.5

    elif tact[3] == 1 and old_tact[3] == 0:
        phaze += 1
        if phaze == 3:
            phaze = 0
            day += 1

    display_items.append(display_item(16 * selected, 128-16, "selected.bmp"))

    sensor_init()
    val = array.array('H', [0, 0])
    fcntl.ioctl(rpikey, 301, val, 1)
    res = [x / 10 for x in val]  # [0]: 습도, [1]: 온도
    if day != old_day: # TODO 확률 추가
        print(f'Day passed to {day} from {old_day}')
        event.append(event_item(EVENT_SUN, duration=10))
        event.append(event_item(EVENT_BUG, duration=10))
        if day % 2:
            event.append(event_item(EVENT_WATER, only_on_day=day))
        print(f"Current evnet: {event}")
        old_day = day 

    if score  <1:
        print('plant is dead')
        exit(0)

def display():
    global led_event_idx, led_last_changed

    if day!=old_day:
        print(f'day {day}')
        if score<=3:
            print('your plant is weak')
    if phaze!=old_phaze:
        print(phaze_str[phaze])

    if old_day==0 or level!=old_level:
        print(background[level])
        if level==4:
            print('end')
            exit(0)
    display_items.append(display_item(0,0,"background"+str(level)+".bmp"))

    if time.time() - led_last_changed > 1.0: # 1.0: led 변경 주기
        color = COLOR_OFF
        if len(event) == 0: # 이벤트가 없을 때
            color = COLOR_WHITE
        else:
            led_event_idx += 1 # 있으면 순회
            if led_event_idx >= len(event):
                led_event_idx = 0
            led_last_changed = time.time()
            color = EVENT_TO_COLOR[event[led_event_idx].type]

        val = array.array('L', color)
        fcntl.ioctl(rpikey, 101, val, 0)

    data=[0 for _ in range(128)]
    for dis in display_items:
        update_area(dis.x,dis.y,dis.img_name,data)
    val = array.array('Q',data)
    fcntl.ioctl(rpikey,200,val,0)

def callback():
    ar = array.array('L', [0, 0,0,0])
    fcntl.ioctl(rpikey, 100, ar, 1)
    for i in range(4):
        tact[i]=ar[i]

def save_old_val():
    global old_level, old_day, old_score, old_phaze,old_tact,old_event
    old_level, old_day, old_score, old_phaze = level, day, score, phaze
    old_event = event
    old_tact = list(tact)

def sensor_init():
    val = array.array('H', [0])  # 혹시 몰라서 만든 더미값
    fcntl.ioctl(rpikey, 300, val, 0)
    time.sleep(0.001)

def main():
    while True:
        process()
        display()
        save_old_val()
        callback()
        time.sleep(0.02)
if __name__=="__main__":
    main()