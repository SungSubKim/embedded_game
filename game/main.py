from conv import update_area
import fcntl,array,random,time
# import keyboard
tact =[0,0,0,0]
old_tact = list(tact)
level, day, score, phaze = 0,0,5,0
background = ["seed","kids","child","adult","sakura"]
phaze_str= ["moring","daytime","night"]
old_level, old_day, old_score, old_phaze = level, day,score,phaze
water_chance= True
last_water ,event= 0 , 0
old_event= event
event_time,event_arr=0,[2,2,2,3,3,4,0,0]
rpikey = open("/dev/rpikey","w")
display_items =[]
selected =0
class display_item():
    def __init__(self,x,y,img_name):
        self.x = x
        self.y = y
        self.img_name = img_name
def process():
    global day,phaze,water_chance,score,last_water,level,selected,event,display_items,event_time,event_arr
    display_items.clear()
    display_items.append(display_item(0,      128 - 16, "bot1.bmp"))
    display_items.append(display_item(16,     128 - 16, "bot2.bmp"))
    display_items.append(display_item(16 * 2, 128 - 16, "bot3.bmp"))
    display_items.append(display_item(16 * 3, 128 - 16, "bot3.bmp"))
    if day==0:
        print('game_start')
        day+=1
    if tact[0] == 1 and old_tact[0] == 0:
        selected = (selected-1)%4
    elif tact[1] == 1 and old_tact[1] == 0:
        selected = (selected + 1) % 4
    elif tact[2] == 1 and old_tact[2] == 0:
        if selected == 0:
            if water_chance:
                display_items.append(display_item(16 * 2,0,  "ani1.bmp"))
                print('water supply ', end='')
                water_chance = False
                if day % 2 == 0 or phaze == 1:
                    score -= 1
                    print(f'bad')
                elif phaze == 2:
                    score -= 2
                    print(f'too bad')
                else:
                    print('good')
                    score += 1
                last_water = day
        elif selected == 1:
            if event==2:
                display_items.append(display_item(16 * 2,0,  "ani2.bmp"))
                event=0
        elif selected == 2:
            if event==3:
                display_items.append(display_item(16 * 2,0,  "ani3.bmp"))
                event=0
        else:
            if event==4:
                display_items.append(display_item(16 * 2,0,  "ani4.bmp"))
                event=0
    elif tact[3] == 1 and old_tact[3] == 0:
        phaze += 1
        if phaze == 3:
            phaze = 0
            day += 1

    display_items.append(display_item(16 * selected, 128-16, "selected.bmp"))
    if day!=old_day:
        water_chance=True
        event = random.sample(event_arr,1)[0] #이벤트생성
        if event!=0:
            event_time = time.time()
        if day==5 or day==8 or day==12 or day==20:
            level+=1
        if last_water-day==-3:
            print('too bad')
            last_water = day
            score-=2

    val = array.array('H', [0, 0])
    fcntl.ioctl(rpikey, 301, val, 1)
    res = [x / 10 for x in val]  # [0]: 습도, [1]: 온도
    if res[0]>0.4:
        if res[1]>20:
            event_arr = [3,3,3,4,4,4]
        else:
            event_arr = [2,2,2,3,3,3,0,0]
    else:
        if res[1]>20:
            event_arr = [2,2,2,2,2,2,0,0]
        else:
            event_arr = [2,2,2,4,4,4]

    if score  <1:
        print('plant is dead')
        exit(0)
def display():
    if day!=old_day:
        print(f'day {day}')
        if score<=3:
            print('your plant is weak')
    if phaze!=old_phaze:
        print(phaze_str[phaze])

    if old_day==0 or level!=old_level:
        print(background[level])
        display_items.append(display_item(0,0,"background"+str(level)+".bmp"))
        if level==4:
            print('end')
            exit(0)

    diff= time.time()-event_time
    color =[0,0,0]
    if event>0 and  diff< 2:
        if event==2:
            color=[1,0,0]
        elif event == 3:
            color = [0,1, 0]
        elif event == 4:
            color = [0, 0,1]
        if diff<0.5 or (1<diff<1.5):
            val = array.array('L', color)  # R,G,B: 초록색 켤 것
        else:
            val = array.array('L',[0,0,0])
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
    time.sleep(0.2)
def main():
    # display_init()
    sensor_init()
    while True:
        process()
        display()
        save_old_val()
        callback()
        time.sleep(0.1)
if __name__=="__main__":
    main()