import os
import cv2
import numpy as np
import matplotlib.pyplot as plt


def getImg(pathSource,file):
    imgName = os.path.join(pathSource, file)
    img = cv2.imread(imgName)
    return img


def getPositions(pathTxt, file):
    path = os.path.join(pathTxt, file+'.txt')
    # 原图中(左上、右上、左下、右下)
    pos = np.loadtxt(path)
    pos = np.float32(pos)
    x_mean = (pos[0][0] + pos[1][0])/2
    y_mean = (pos[0][1] + pos[2][1])/2
    pos[0][0] += 30
    pos[0][1] += 18
    pos[1][0] -= 30
    pos[1][1] += 18
    pos[2][0] += 30
    pos[2][1] -= 18
    pos[3][0] -= 30
    pos[3][1] -= 18
    # 校正后
    # pts1 = np.float32([[488, 560], [2312, 592], [408, 1688], [2392, 1656]])
    # pts2 = np.float32([[0, 0], [2312 - 488, 0], [0, 1688 - 560], [2312 - 488, 1688 - 560]])
    width = int(pos[1][0] - pos[0][0])
    height = int(pos[3][1] - pos[0][1])
    pos2 = np.float32([[0, 0],
                       [width, 0],
                       [0, height],
                       [width, height]])
    return pos, pos2


def Warping(img, pts1, pts2):
    # 原图中书本的四个角点(左上、右上、左下、右下),与变换后矩阵位置
    width = int(pts2[3][0])
    height = int(pts2[3][1])
    # 生成透视变换矩阵；进行透视变换
    M = cv2.getPerspectiveTransform(pts1, pts2)
    result = cv2.warpPerspective(img, M, (width, height))  # (2312 - 488, 1688 - 560))
    # 显示
    # cv2.imshow("original_img",img)
    # cv2.imshow("result", result)
    cv2.destroyAllWindows()
    return result


def main():
    """
    # 原图像，获得名称
    # 找到对应的txt文件
    # 切割
    # 保存图像
    :return:
    """
    __dirpath = 'C:\\Users\\inplus\\Desktop\\Project1\\Project1'
    pathSource = os.path.join(__dirpath, 'Dataset')
    pathTxt = os.path.join(__dirpath, 'Vertex4_txt')
    pathDest = os.path.join(__dirpath, 'cutImg')

    for file in os.listdir(pathSource):
        print('正在处理图像' + file + '...')
        img = getImg(pathSource,file)
        pts1, pts2 = getPositions(pathTxt, file)
        print('4个顶点的坐标为：\n', pts1)
        result = Warping(img, pts1, pts2)
        # cv2.imshow("result_img", result)
        cv2.imwrite(os.path.join(pathDest, file), result)
        # cv2.waitKey(0)
        # cv2.destroyAllWindows()
    print('全部处理完毕！')

main()
