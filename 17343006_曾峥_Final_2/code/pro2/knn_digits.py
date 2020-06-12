# -*- coding: utf-8 -*-
"""

"""

import os
import OperatePicture as OP
import OperateDatabase as OD
import PictureAlgorithm as PA
import csv
import functools
import time

# Essential vavriable 基础变量
# Standard size 标准大小
WIDTH = 32
HEIGHT = 48
# Gray threshold 灰度阈值
color = 150 / 255

n = 10


def renameFiles(path, i):
    # 加一个前序
    # 0/13.bmp -> 0/0_13.bmp
    count = 0
    for file in os.listdir(path):
        # print(file)
        os.rename(os.path.join(path, file), os.path.join(path, str(i) + "_" + str(count) + ".bmp"))
        count += 1


# 如果训练图集的名字还没有初始化的时候，使用这个函数把图像名字加上前缀i
# 用到辅助函数renameFiles
def InitFile(fileName):
    i = 0
    Names = []
    while i < 10:
        subFile = os.path.join(fileName, str(i))
        # 重命名 rename images -> 0_1.bmp
        renameFiles(subFile, i)
        i += 1


def getAllModels(fileName):
    i = 0
    Names = []
    while i < 10:
        subFile = fileName + str(i) + "/"
        # 添加这个子文件夹的所有文件
        Names += os.listdir(subFile)
        i += 1
    # print(Names)
    return Names


def compare_personal(x, y):
    temp_x = x.split('.')[0]
    temp_y = y.split('.')[0]
    return int(temp_x) - int(temp_y)


def getKNNModel():
    # 读取原CSV文件
    reader = list(csv.reader(open('data\\Database.csv', encoding='utf-8')))

    # 清除读取后的第一个空行
    del reader[0]

    # 读取models目录下的所有文件名
    fileNames = getAllModels("./models/")

    # 对比fileNames与reader，得到新增的图片newFileNames
    newFileNames = OD.NewFiles(fileNames, reader)
    print('New pictures are: ', newFileNames)

    # 得到newFilesNames对应的矩阵
    pic = OP.GetTrainPicture(newFileNames)

    # 将新增图片矩阵存入CSV中
    OD.SaveToCSV(pic, newFileNames)

    # 将原数据库矩阵与新数据库矩阵合并
    pic = OD.Combination(reader, pic)

    return pic


def knn_test(pic, folder):
    testImgs = os.listdir(folder)

    # 排序，根据文件名称
    testImgs = sorted(testImgs, key=functools.cmp_to_key(compare_personal))

    # 获取测试图片的矩阵信息
    testPic = OP.GetTestPicture(folder, testImgs)

    # 计算每一个待识别图片的可能分类
    result = PA.CalculateResult(testPic, pic)
    for i, item in enumerate(result):
        print(testImgs[i])
        # print(item[0])
        # for i in range(n):
        # print('第' + str(i + 1) + '个向量为' + str(item[i + n]) + ',距离为' + str(item[i]))

    # 通过分配权重重新计算排名
    newResult = PA.CalculateWeight(result, n, testImgs)

    # 这个文件夹的最终结果
    strLine = ""
    for i in newResult:
        strLine += i
    # print(strLine)
    return strLine


count = 0
right = 0
map = {}


def check(ret, file):
    reader = list(csv.reader(open('data\\result.csv', encoding='utf-8')))
    for item in reader:
        if file == item[0]:
            result = item[1]
            right_temp = 0
            for i in range(0, min(len(ret), len(result))):
                if ret[i] == result[i]:
                    right_temp += 1
            global right, count, map
            right += right_temp
            count += len(result)
            result = [ret, right_temp]
            map[file] = result
            print('识别结果以及正确的个数为 ', result)
            return result


def main():
    # 得到KNN的预先加载模型csv文件
    start = time.time()
    pic = getKNNModel()
    end1 = time.time()

    # 测试
    path = 'C:\\Users\\inplus\\Desktop\\Project1\\Project1\\result\\groups'

    for file in os.listdir(path):
        print(file)
        folder = os.path.join(path, file, "digits")
        if not os.path.exists(folder):
            os.makedirs(folder)  # 创建文件夹

        ret = knn_test(pic, folder)
        map[file] = check(ret, file)

        end2 = time.time()

    print(map)
    end3 = time.time()
    print("训练所需运行时间:%.2f秒" % (end1 - start))
    print("50个测试所需运行总时间:%.2f秒" % (end3 - end1))
    print("所有数字识别正确率:%.4f" % (right / count))


main()
