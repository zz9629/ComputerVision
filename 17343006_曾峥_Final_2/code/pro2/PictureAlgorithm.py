# -*- coding: utf-8 -*-
"""
Created on Wed Oct 18 14:58:54 2017

@author: vento
"""
import numpy as np
import sys

sys.setrecursionlimit(10000000)

# Essential vavriable 基础变量
# Standard size 标准大小
WIDTH = 32
HEIGHT = 48
# Gray threshold 灰度阈值
color = 100 / 255
# 相似图片前几名
n = 10


def CalculateDistance(test, train, num, n):
    '''计算每个图片前n相似图片'''
    # 前n个放距离，后n个放数字
    dis = np.zeros(2 * n * len(test)).reshape(len(test), 2 * n)
    for i, item in enumerate(test):
        # 计算出每个训练图片与该待识别图片的距离
        itemDis = np.sqrt(np.sum((item - train) ** 2, axis=1))
        # 对距离进行排序，找出前n个
        sortDis = np.sort(itemDis)
        dis[i, 0:n] = sortDis[0:n]
        for j in range(n):
            # 找到前几个在原矩阵中的位置
            maxPoint = list(itemDis).index(sortDis[j])
            # 找到num对应位置的数字，存入dis中
            dis[i, j + n] = num[maxPoint]
    return dis


def CalculateResult(test, train):
    '''计算待识别图片test的可能分类'''
    # 得到每个图片的前n相似图片
    testDis = CalculateDistance(test[:, 0:WIDTH * HEIGHT], train[:, 0:WIDTH * HEIGHT], train[:,WIDTH * HEIGHT], n)
    # 将testDis变成列表
    tt = testDis.tolist()
    return tt


def ShowRank(weightDict):
    '''输出单个图片的排名顺序'''
    maxRank = [0, 9999]
    for item in weightDict:
        # 寻找最小距离的数字
        if weightDict[item] < maxRank[1]:
            maxRank = [item, weightDict[item]]
        # print('数字' + item + '的相对距离为' + str(weightDict[item]))
    # print('最有可能为数字' + str(maxRank[0]) + '，相对距离为' + str(weightDict[item]))
    # print(type(maxRank[0][0]))，maxRank是0.0之类的str
    return maxRank[0][0]


def CalculateWeight(pictures, n, testFiles):
    '''计算加权距离'''
    # 权重（前五名）
    temp = np.array([1, 2, 3, 4, 5])
    weight = list(temp / np.sum(temp))
    weightNum = len(weight)
    predicts = []
    for j, pic in enumerate(pictures):
        # print(testFiles[j])
        # 存储加权距离的字典
        weightDict = {}
        for i in range(weightNum):
            # 判断该数字之前是否出现过
            if str(pic[n + i]) in weightDict:
                weightDict[str(pic[n + i])] = weightDict[str(pic[n + i])] + weight[i] * pic[i]
            else:
                weightDict[str(pic[n + i])] = weight[i] * pic[i]
        predict = ShowRank(weightDict)
        predicts += predict
    return predicts