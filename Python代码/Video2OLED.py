import numpy as np
import cv2
import serial
import time

#将128*64的二值化图像转换为OLED数组格式
def img2array(frame):
    
    array = np.zeros((8, 132), dtype='uint8')
    
    for j in range(64):
        for i in range(132):
            if frame[j][i] > 0:
                array[j // 8][i] = (array[j // 8][i]) | (0x01 << (j % 8))
    
    return array

def change(gray_avg,img):
    step =51 # int(255/(2*2+1))
    gray_scale = int(gray_avg/step)
    if gray_scale == 0:
        img[0][0]=0
        img[0][1]=0
        img[1][0]=0
        img[1][1]=0
    elif gray_scale==1:
        img[0][0]=0
        img[0][1]=0
        img[1][0]=255
        img[1][1]=0
    elif gray_scale==2:
        img[0][0]=0
        img[0][1]=255
        img[1][0]=255
        img[1][1]=0
    elif gray_scale==3:
        img[0][0]=0
        img[0][1]=255
        img[1][0]=255
        img[1][1]=255
    else :
        img[0][0]=255
        img[0][1]=255
        img[1][0]=255
        img[1][1]=255        

#打开串口
serial_port = serial.Serial('COM4', 921600)


#设置预览窗口
cv2.namedWindow('img', cv2.WINDOW_NORMAL)
cv2.resizeWindow('img', 132 * 4, 64 * 4)

#打开视频
cap = cv2.VideoCapture('打篮球.mp4')
# cap = cv2.VideoCapture(0)

#记住开始时间
start_time = time.time()

while cap.isOpened():

    #获取当前时间对应的帧数
    run_time = time.time() - start_time
    fps = cap.get(cv2.CAP_PROP_FPS)
    frame_number = int(run_time * fps)
    
    #获取当前帧
    cap.set(cv2.CAP_PROP_POS_FRAMES, frame_number)
    flag, img = cap.read()
    if not flag:
        break
    
    #帧图像处理
    img = cv2.resize(img, (132, 64))                         #修改尺寸
    img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)              #灰度化
    img = cv2.threshold(img, 170, 255, cv2.THRESH_BINARY)[1] #二值化
    # k_size = 2 
    # for h in range(0,64,k_size):
    #     for w in range(0,132,k_size):
    #         gray_avg = int(sum([img[h][w],img[h][w+1],img[h+1][w],img[h+1][w+1]])/(k_size*k_size))
    #         temp =img[h:h+2,w:w+2] #取样
    #         change(gray_avg,temp)


    #预览图像
    cv2.imshow('img', img)

    #转换为数组并使用串口发送
    img_array = img2array(img)
    serial_port.write(img_array)

    #等待按键，按q键退出
    key = cv2.waitKey(1)
    if key & 0xFF == ord('q'):
        break

#释放资源
cap.release()
cv2.destroyAllWindows()
serial_port.close()
