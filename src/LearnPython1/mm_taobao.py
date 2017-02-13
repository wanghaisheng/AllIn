import urllib.request
import urllib.parse
import re
import os


class Spider:
    def __init__(self):
        self.siteURL = 'http://mm.taobao.com/json/request_top_list.htm'

    def get_page(self, page_index):
        url = self.siteURL + "?page=" + str(page_index)
        print(url)
        request = urllib.request.Request(url)
        response = urllib.request.urlopen(request)
        return response.read().decode('gbk')

    def get_contents(self, page_index):
        page = self.get_page(page_index)
        pattern = re.compile('<div class="list-item".*?pic-word.*?<a href="(.*?)".*?<img src="(.*?)".*?<a class="lady-name.*?>(.*?)</a>.*?<strong>(.*?)</strong>.*?<span>(.*?)</span>',re.S)
        items = re.findall(pattern, page)
        contents = []
        for item in items:
            contents.append([item[0], item[1], item[2], item[3], item[4]])
        return contents

    def get_detail_page(self, info_url):
        response = urllib.request.urlopen("http:" + info_url)
        return response.read().decode('gbk')

    def get_brief(self, page):
        pattern = re.compile('<div class="mm-aixiu-content".*?>(.*?)<!--', re.S)
        result = re.search(pattern, page)
        return result.tool.replace(result.group(1))

    def get_all_img(self, page):
        pattern = re.compile('<div class="mm-aixiu-content".*?>(.*?)<!--', re.S)
        content = re.search(pattern, page)
        patternImg = re.compile('<img.*?src="(.*?)"', re.S)
        images = re.findall(patternImg, content.group(1))
        return images

    def save_icon(self, icon_url, name):
        splitPath = icon_url.split('.')
        fTail = splitPath.pop()
        fileName = name + "/icon." + fTail
        self.save_img(icon_url, fileName)

    def save_imgs(self, images, name):
        number = 1
        # print("发现" + name + "共有" + len(images) + "张照片")
        for image_url in images:
            split_path = image_url.split('.')
            fTail = split_path.pop()
            if len(fTail) > 3:
                fTail = "jpg"
            file_name = name + "/" + str(number) + "." + fTail
            self.save_img(image_url, file_name)
            number += 1

    def save_img(self, img_url, file_name):
        u = urllib.request.urlopen("http:" + img_url)
        data = u.read()
        f = open(file_name, 'wb')
        f.write(data)
        f.close()

    def save_brief(self, content, name):
        file_name = name + "/" + name + ".txt"
        f = open(file_name, "w+")
        print("正在保存个人信息为" + file_name)
        f.write(content.encode('utf-8'))

    def mkdir(self, path):
        path = path.strip()
        is_exist = os.path.exists(path)
        if not is_exist:
            os.makedirs(path)
            return True
        else:
            return False

    def save_page_info(self, page_index):
        # 获取第一页淘宝MM列表
        contents = self.get_contents(page_index)
        for item in contents:
            # item[0]个人详情URL,item[1]头像URL,item[2]姓名,item[3]年龄,item[4]居住地
            print(u"发现一位模特,名字叫", item[2], u"芳龄", item[3], u",她在", item[4])

            print(u"正在偷偷地保存", item[2], "的信息")

            print(u"又意外地发现她的个人地址是", item[0])

            # 个人详情页面的URL
            detail_url = item[0]
            # 得到个人详情页面代码
            detail_page = self.get_detail_page(detail_url)

            # 获取个人简介
            # brief = self.get_brief(detail_page)

            # 获取所有图片列表
            images = self.get_all_img(detail_page)

            self.mkdir(item[2])

            # 保存个人简介
            # self.save_brief(brief, item[2])

            # 保存头像
            self.save_icon(item[1], item[2])

            # 保存图片
            self.save_imgs(images, item[2])

    def save_pages_info(self, start, end):
        2  0n�������/�y��p��a�t�������C�O��X����vz��ܒ�b�*���
2Z$�!��8� �.�U�B2Q��>�u%�c*M�@�i��b���k�s&pY��#-�V�l�)������ثt��nD����L��g�����I9N�Z��i����)���;��E�0ىU�'J��� X���Xۥ%~d