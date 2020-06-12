# -*- coding: utf-8 -*-
"""
Created on Sat Oct 14 18:47:21 2017

@author: vento
"""

import os
from skimage import io, color, filters
import numpy as np
import PictureAlgorithm as PA

##Essential vavriable 基础变量
# Standard size 标准大小
WIDTH = 32
HEIGHT = 48
# Gray threshold 灰度阈值
colorGray = 150 / 255

STR = list("$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\|()1{}[]?-_+~<>i!lI;:,\"^`'. ")


def JudgeEdge(img, length, flag, size):
    '''Judge the Edge of Picture判断图片切割的边界'''
    for i in range(length):
        # Row or Column 判断是行是列
        if flag == 0:
            # Positive sequence 正序判断该行是否有手写数字
            line1 = img[i, img[i, :] < colorGray]
            # Negative sequence 倒序判断该行是否有手写数字
            line2 = img[length - 1 - i, img[length - 1 - i, :] < colorGray]
        else:
            line1 = img[img[:, i] < colorGray, i]
            line2 = img[img[:, length - 1 - i] < colorGray, length - 1 - i]
        # If edge, recode serial number 若有手写数字，即到达边界，记录下行
        if len(line1) >= 1 and size[0] == -1:
            size[0] = i
        if len(line2) >= 1 and size[1] == -1:
            size[1] = length - 1 - i
        # If get the both of edge, break 若上下边界都得到，则跳出
        if size[0] != -1 and size[1] != -1:
            break
    return size


# 用于切割图像中的margin，使得数字沾满整个图像
# 而这个作业中，train和test的图片都是处理过后的图，因此不需要再切割
# 用到了辅助函数 JudgeEdge
def CutPicture(img):
    '''Cut the Picture 切割图象'''
    # 初始化新大小
    size = []
    # 图片的行数
    length = len(img)
    # 图片的列数
    width = len(img[0, :])
    # 计算新大小
    size.append(JudgeEdge(img, length, 0, [-1, -1]))
    size.append(JudgeEdge(img, width, 1, [-1, -1]))
    size = np.array(size).reshape(4)
    return img[size[0]:size[1] + 1, size[2]:size[3] + 1]


def GetTrainPicture(files):
    '''Read and save train picture 读取训练图片并保存'''
    Picture = np.zeros([len(files), WIDTH * HEIGHT + 1])
    # loop all pictures 循环所有图片文件
    for i, item in enumerate(files):
        # Read the picture读取这个图片
        pre = item[0]
        path = './models/' + pre + '/' + item
        img = io.imread(path)  # as_gray

        # turn RGB to grey 转为灰度值，并二值化
        img = color.rgb2gray(img)
        colorGray = filters.threshold_otsu(img, nbins=256)
        # Clear the noise清除噪音，二值化图像
        img = (img <= colorGray) * 1.0

        # Cut the picture and get the picture of handwritten number
        # 将图片进行切割，得到有手写数字的的图像

        # Stretch the picture and get the standard size 100x100
        # 将图片进行拉伸，得到标准大小100x100
        img = img.reshape(WIDTH * HEIGHT)
        # Save the picture to the matrix 将图片存入矩阵
        Picture[i, 0:WIDTH * HEIGHT] = img
        # Save picture's name to the matrix 将图片的名字存入矩阵
        Picture[i, WIDTH * HEIGHT] = int(item[0])
    return Picture


def GetTestPicture(folder, files):
    '''得到待检测图片并保存'''
    Picture = np.zeros([len(files), WIDTH * HEIGHT])
    for i, item in enumerate(files):
        path = os.path.join(folder, item)
        # print(path)
        # path = "./temp/" + str(index) + "/" + DigitType + "/" + item
        img = io.imread(path)  # as_gray
        img = color.rgb2gray(img)
        colorGray = filters.threshold_otsu(img, nbins=256)
        img = (img <= colorGray) * 1.0
        img = CutPicture(img)
        img = img.reshape(WIDTH * HEIGHT)
        Picture[i, 0:WIDTH * HEIGHT] = img
    return Picture


def ShowPicture(pic):
    l = len(STR)
    for item in pic:
        nowPic = item[0:WIDTH * HEIGHT]
        txt = ''
        nowPic = nowPic.reshape(WIDTH , HEIGHT)
        for i in nowPic:
            for j in i:
                point = int(np.floor(l * (1 - j)))
                nowStr = STR[point - 1]
                txt = txt + nowStr
            txt = txt + '\n'
        f = open('./showpic/output1.txt', 'w')
        f.write(txt)
        f.close()
