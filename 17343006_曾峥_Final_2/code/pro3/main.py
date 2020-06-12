import csv
import functools
import os
import time
from os import path

import pytesseract
from PIL import Image

pytesseract.pytesseract.tesseract_cmd = 'D:/Program Files (x86)/Tesseract-OCR/tesseract.exe'
config1 = ' -psm 7'
config2 = ' -psm 10'


def compare_personal(x, y):
    temp_x = x.split('.')[0]
    temp_y = y.split('.')[0]
    return int(temp_x) - int(temp_y)


# img = Image.open('C:\\Users\\inplus\\Desktop\\3.bmp')
# print(type(img))
#
# text = pytesseract.image_to_string(img, lang='chi_sim', config=tessdata_dir_config)
# print(text)

def test():
    folder = 'C:\\Users\\inplus\\Desktop\\data\\result\\groups\\IMG_20191204_001325.bmp\\group_7'

    Imgs = os.listdir(folder)
    # 排序，根据文件名称
    Imgs = sorted(Imgs, key=functools.cmp_to_key(compare_personal))

    for i in Imgs:
        PATH = path.join(folder, i)
        img = Image.open(PATH)
        text = pytesseract.image_to_string(img, lang='chi_sim', config=tessdata_dir_config)
        print('str = ', text)
        # 通过单个字符识别字符，每一个群写入一行


def file_list(dirname, ext='.csv'):
    """获取目录下所有特定后缀的文件
    @param dirname: str 目录的完整路径
    @param ext: str 后缀名, 以点号开头
    @return: list(str) 所有子文件名(不包含路径)组成的列表
    """
    return list(filter(
        lambda filename: os.path.splitext(filename)[1] == ext,
        os.listdir(dirname)))


def main():
    # 通过直接读取整张图像读取文件，去读后缀名为.bmp的群名
    with open("data/GroupResult.csv", "w") as csvfile:
        writer = csv.writer(csvfile)
        # 先写入columns_name
        groupName = ["img_name"]
        for i in range(0, 15):
            groupName.append("Group_" + str(i))
        print(groupName)
        writer.writerow(groupName)

        # 写入群的识别结果
        # 写入多行用writerows
        appendContent = []

        __dirPath = 'C:\\Users\\inplus\\Desktop\\data\\result\\groups'
        for file in os.listdir(__dirPath):
            tempFile = path.join(__dirPath, file)  # 获取每一个图片的群图片
            print('*' * 80)
            print(tempFile)
            imgs = file_list(tempFile, '.bmp')      # 获取群的图像
            # 排序，根据文件名称
            imgs = sorted(imgs, key=functools.cmp_to_key(compare_personal))

            result = [file]
            for i, name in enumerate(imgs):
                imgPath = path.join(tempFile, name)
                item = Image.open(imgPath)

                text = pytesseract.image_to_string(item, lang='chi_sim', config=config1)
                text = text.replace('\n', '').replace(' ', '')
                if text == '':
                    text = pytesseract.image_to_string(item, lang='chi_sim', config=config2)

                print(name, '的识别结果为', text)
                result.append(text)
            print('\n', result)
            appendContent.append(result)

        writer.writerows(appendContent)


if __name__ == "__main__":
    start = time.time()
    main()
    end = time.time()
    print("识别所有字符总运行时间:%.2f秒" % (end - start))
    print("识别所有字符平均时间:%.2f秒" % ((end - start)/60))


