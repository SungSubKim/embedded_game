from conv import update_area,write_str,font_rotate
import fcntl,array,random,time

tact =[1,1,1,1]
old_tact = list(tact)
level, day, score, phaze = 0,0,10,0
background = ["seed","kids","child","adult","sakura"]
phaze_str= ["morning","afternoon","night"]
old_level, old_day, old_score, old_phaze = level, day,score,phaze
water_chance= True
last_water = 0
event= []
event_chances = [0,0,0,0]
last_sudden_event = 0
event_interval = 0
led_event_idx = 0
led_last_changed = 0
current_animation = None
animation_start_time = 0
animation_duration = 0
phase_start_time = 0
phase_duration = 4
state = None

rpikey = open("/dev/rpikey","w")
display_items =[]
selected =0

STATE_LOADING = "loading"
STATE_DIED    = "died"
STATE_END     = "ended"
STATE_HOWTO   = "howto"
STATE_INGAME  = "ingame"


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
EVENT_WRONG      = "wrong"

EVENTS = [EVENT_WATER, EVENT_SUN, EVENT_BUG, EVENT_FERTILIZER, EVENT_WRONG]

EVENT_TO_COLOR = {
    EVENT_WATER     : COLOR_BLUE,
    EVENT_SUN       : COLOR_RED,
    EVENT_BUG       : COLOR_GREEN,
    EVENT_FERTILIZER: COLOR_YELLOW,
}

IDX_TO_EVENT = {
    0: EVENT_WATER,
    1: EVENT_SUN,
    2: EVENT_BUG,
    3: EVENT_FERTILIZER,
    4: EVENT_WRONG
}

EVENT_TO_IDX = {
    EVENT_WATER      :0,
    EVENT_SUN        :1,
    EVENT_BUG        :2,
    EVENT_FERTILIZER :3,
    EVENT_WRONG      :4
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

def get_env():
    global event_chances

    sensor_init()
    val = array.array('H', [0, 0])
    fcntl.ioctl(rpikey, 301, val, 1)
    res = [x / 10 for x in val]  # [0]: 습도, [1]: 온도
    
    # 확률은 0 밑이나 1 위로 올라가도 됨, 알아서 처리됨
    event_chances[0] = (120 - res[0]) / 100 # 습도가 낮으면 물 확률 증가
    event_chances[1] = (50 - res[1]) / 100 # 온도가 낮으면 햇빛 확률 증가
    event_chances[2] = ((32-abs(32-res[1])) + (70-abs(70-res[0]))) / 100 # 적절한 고온 다습에서 벌레 출현 증가
    event_chances[3] = 30 / 100 # 비료는 고정 확률

    print(f"Read environment, event chances: {event_chances}")


def remove_expired_events():
    global event, led_last_changed, led_event_idx, score

    prev_count = len(event)
    event[:] = [e for e in event if e.is_expired(day) == False] # remove expired events
    changed_count = len(event)
    if prev_count != changed_count:
        led_last_changed = 0
        led_event_idx = 0
        score -= prev_count - changed_count
    

rpikey = open("/dev/rpikey","w")
display_items =[]
selected =0
def process():
    global day, phaze, water_chance, score, last_water, \
        level, selected, event, event_chances, display_items, \
        current_animation, animation_start_time, animation_duration, old_day, \
        led_last_changed, led_event_idx, \
        last_sudden_event, event_interval, \
        phase_duration, phase_start_time, \
        state, last_state \

    display_items.clear()
    
    display_items.append(display_item(0,      128 - 16, "bot1.bmp"))
    display_items.append(display_item(16,     128 - 16, "bot2.bmp"))
    display_items.append(display_item(16 * 2, 128 - 16, "bot3.bmp"))
    display_items.append(display_item(16 * 3, 128 - 16, "bot4.bmp"))

    remove_expired_events()

    if state == STATE_HOWTO:
        if tact[2] == 1 and old_tact[2] == 0:
            state = STATE_INGAME
            day = 0
            score = 10
            led_last_changed = 0
            led_event_idx = 0
            phase_start_time = 0
            phase_duration = 4
            phaze = 0
            level = 0
            event = []
            current_animation = None
            selected = 0
        return
    if state == STATE_DIED:
        if tact[2] == 1 and old_tact[2] == 0:
            state = STATE_HOWTO
        return
    if state == STATE_END:
        if tact[2] == 1 and old_tact[2] == 0:
            state = STATE_HOWTO
        return        


    if day==0:
        print('game_start')
        day+=1
        phase_start_time = time.time()
        phase_duration = 4

    if current_animation != None:
        if current_animation == EVENT_WRONG:
            display_items.clear()
            display_items.append(display_item(16 * 2,0,  f"wrong.bmp"))
        else:
            display_items.clear()
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
            animation_duration = 0.3
            if EVENTS[selected] == EVENT_WATER:
                if phaze == 0:
                    score += 10
                elif phaze == 1:
                    score += 7
                elif phaze == 2:
                    score += 3
            else:
                score += 2
        else: # wrong press
            current_animation = EVENT_WRONG
            animation_start_time = time.time()
            animation_duration = 0.3
            score -= 1

    if time.time() - phase_start_time > phase_duration \
            or (tact[3] == 1 and old_tact[3] == 0):
        phase_start_time = time.time()
        phase_duration = 4
        phaze += 1 
        if phaze >= 3:
            phaze = 0
            day += 1

    display_items.append(display_item(16 * selected, 128-16, "selected.bmp"))

    if day != old_day:
        get_env()
        print(f'Day passed to {day} from {old_day}')
        if random.random() < event_chances[0]:
            event.append(event_item(EVENT_WATER, only_on_day=day))
        remove_expired_events()
        print(f"Current event: {[e.type for e in event]}")
        old_day = day 
        
        if day in [3,5,7,10,11]:
            level+=1

    if time.time() - last_sudden_event > event_interval:
        get_env() # 온/습도 센서 기반 확률 산정
        last_sudden_event = time.time()
        print(f"Sudden event generation!")
        event_interval = random.randrange(3, 10)
        for i in range(1,4): # 물은 날이 변경될 때만 진행
            if EVENTS[i] in [e.type for e in event]: # 이벤트가 있을 경우 생성하지 않음
                continue
            if random.random() < event_chances[i]: # 확률적으로 이벤트 생성
                event.append(event_item(EVENTS[i], duration=random.randrange(5,8)))
        remove_expired_events()
        print(f"Current event: {[e.type for e in event]}")

    if score  <1:
        print('plant is dead')
        state = STATE_DIED

def display():
    global led_event_idx, led_last_changed, state, last_state, level

    if last_state == state and state != STATE_INGAME:
        val = array.array('L', COLOR_OFF)
        fcntl.ioctl(rpikey, 101, val, 0)
        return

    if day!=old_day:
        print(f'day {day}')
        if score<=3:
            print('your plant is weak')
    if phaze!=old_phaze:
        print(phaze_str[phaze])

    if old_day==0 or level!=old_level:
        if level==5:
            print('end')
            state = STATE_END
            level = 4
    display_items.append(display_item(0,0,"background"+str(level)+".bmp"))

    if level==4:
        p_count = score//8
        if p_count > 10:
            p_count = 10
        if p_count < 1:
            p_count = 1
        display_items.append(display_item(0, 0, "particle" + str(p_count) + ".bmp"))
    if time.time() - led_last_changed > 0.3: # 1.0: led 변경 주기
        color = COLOR_OFF
        if len(event) == 0: # 이벤트가 없을 때
            color = COLOR_OFF
        else:
            if led_event_idx >= len(event):
                led_event_idx = 0
            led_last_changed = time.time()
            color = EVENT_TO_COLOR[event[led_event_idx].type]
            led_event_idx += 1

        val = array.array('L', color)
        fcntl.ioctl(rpikey, 101, val, 0)

    data=[0 for _ in range(128)]

    if state != STATE_INGAME:
        last_state = state
        if state == STATE_HOWTO:
            howto()
            return
        write_str('press select', 6, 114, data);
        write_str('to restart', 19, 120, data);
        display_items[:] = [d for d in display_items if "back" in d.img_name or "parti" in d.img_name]        
    
    for dis in display_items:
        update_area(dis.x,dis.y,dis.img_name,data)

    if state == STATE_DIED:
        write_str('You died!', 15, 100, data, inversed=True);

    if state == STATE_END:
        write_str('CLEARED!', 16, 100, data, inversed=True);

    write_str('  day '+str(day), 0, 8,data);
    write_str(phaze_str[phaze], 0, 16,data);
    write_str('score '+str(score), 0, 0, data);
    val = array.array('Q',data)

    


    fcntl.ioctl(rpikey,200,val,0)

def callback():
    ar = array.array('L', [0,0,0,0])
    fcntl.ioctl(rpikey, 100, ar, 1)
    for i in range(4):
        tact[i]=ar[i]

def save_old_val():
    global old_level, old_day, old_score, old_phaze,old_tact,old_event
    old_level, old_day, old_score, old_phaze = level, day, score, phaze
    old_event = event
    old_tact = list(tact)

def sensor_init(): # 센서 읽기 요청
    val = array.array('H', [0]) 
    fcntl.ioctl(rpikey, 300, val, 0)
    time.sleep(0.001)

def load_imgs():
    for i in range(1, 4+1):
        display_items.append(display_item(16*(i-1), 128-16, f"bot{i}.bmp"))
        display_items.append(display_item(16*(i-1), 128-16, "selected.bmp"))
        display_items.append(display_item(0, 0, f"background{i}.bmp"))
        display_items.append(display_item(16 * 2,0,  f"ani{i}.bmp"))
    for i in range(1, 10+1):
        display_items.append(display_item(0, 0, "particle" + str(i) + ".bmp"))
    display_items.append(display_item(16 * 2,0,  f"wrong.bmp"))
    data=[0 for _ in range(128)]
    for dis in display_items:
        update_area(dis.x,dis.y,dis.img_name,data)

def show_loading():
    val = array.array('L', COLOR_OFF)
    fcntl.ioctl(rpikey, 101, val, 0)
    data=[0 for _ in range(128)]
    write_str('Loading...', 14, 61, data);
    val = array.array('Q',data)
    fcntl.ioctl(rpikey,200,val,0)

def howto():
    val = array.array('L', COLOR_OFF)
    fcntl.ioctl(rpikey, 101, val, 0)
    data=[0 for _ in range(128)]
    write_str('HOW-TO', 20, 0, data);
    write_str('on the middle', 0, 8, data);
    write_str('your tree grow', 0, 14, data);
    write_str('on the below', 0, 22, data);
    write_str('your selected', 0, 28, data);
    write_str('menu is shown', 0, 34, data);
    write_str('button guide', 0, 42, data);
    write_str('UL: select left', 0, 48, data);
    write_str('UR: select right', 0, 54, data);
    write_str('DL: OK, SELCT', 0, 60, data);
    write_str('DR: phase skip', 0, 66, data);
    write_str('led guide', 0, 74, data);
    write_str('blue: water', 0, 80, data);
    write_str('red: sun', 0, 86, data);
    write_str('green: kill bug', 0, 92, data);
    write_str('yellow: ferti.', 0, 98, data);
    write_str('press select', 4, 114, data);
    write_str('to start', 26, 120, data);
    val = array.array('Q',data)
    fcntl.ioctl(rpikey,200,val,0)

def main():
    global state, last_state
    state = STATE_HOWTO
    last_state = STATE_LOADING
    font_rotate()
    show_loading()
    load_imgs()
    while True:
        process()
        display()
        save_old_val()
        callback()
if __name__=="__main__":
    main()