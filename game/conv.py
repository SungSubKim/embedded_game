from PIL import Image
def update_area(x,y,image_name,data):
    # im = Image.open(input("image name: "))
    im = Image.open(image_name)
    width, height = im.size[0],im.size[1]
    #print(f'{width} {height}')
    pix = im.load()
    # data= [0 for _ in range(128)]
    # x, y = 16,128-16
    for row in range(height):
        for col in range(width):
            if sum(pix[(col,row)])/3 > 128:
                data[y+row] |= 2**(64-1-(x+col))
        # print(' ')
    # for x in range
    # print(data)
    # for row in range(height):
    #     for col in range(width):
    #         if (data[y+row]&2**(64-1-(x+col)))>0:
    #             print('O',end='')
    #         else:
    #             print(' ',end='')
    #     print(' ')
    # return data

# update_area()