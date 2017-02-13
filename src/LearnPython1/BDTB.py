import urllib.request
import urllib.error
import urllib.parse
import re


class Tool(object):
    # 去除img标签,7位长空格
    removeImg = re.compile('<img.*?>| {7}|')
    # 删除超链接标签
    removeAddr = re.compile('<a.*?>|</a>')
    # 把换行的标签换为\n
    replaceLine = re.compile('<tr>|<div>|</div>|</p>')
    # 将表格制表<td>替换为\t
    replaceTD = re.compile('<td>')
    # 把段落开头换为\n加空两格
    replacePara = re.compile('<p.*?>')
    # 将换行符或双换行符替换为\n
    replaceBR = re.compile('<br><br>|<br>')
    # 将其余标签剔除
    removeExtraTag = re.compile('<.*?>')

    def replace(self, x):
        x = re.sub(self.removeImg, "", x)
        x = re.sub(self.removeAddr, "", x)
        x = re.sub(self.replaceLine, "\n", x)
        x = re.sub(self.replaceTD, "\t", x)
        x = re.sub(self.replacePara, "\n    ", x)
        x = re.sub(self.replaceBR, "\n", x)
        x = re.sub(self.removeExtraTag, "", x)
        # strip()将前后多余内容删除
        return x.strip()


class BDTB(object):
    def __init__(self, base_url, see_lz):
        self.base_url = base_url
        self.see_lz = '?see_lz=' + str(see_lz)
        self.tool = Tool()

    # 获取页面内容
    def get_page(self, page):
        try:
            url = self.base_url + self.see_lz + '&pn=' + str(page)
            request = urllib.request.Request(url)
            response = urllib.request.urlopen(request)
            text = response.read().decode('utf-8')
            # print(text)
            return text
        except urllib.error.URLError as e:
            if hasattr(e, "reason"):
                print(e.reason)

    def get_title(self, content):
        pattern = re.compile(r'<h3 class="core_title_txt.*?>(.*?)</h3>', re.S)
        result = re.search(pattern, content)
        if result:
            return result.group(1).strip()
        else:
            return None

    def get_page_num(self, content):
        pattern = re.compile(r'<span class="red">(.*?)</span>', re.S)
        result = re.search(pattern, content)
        if result:
            return result.group(1).strip()
        else:
            return None

    def get_content(self, content):
        pattern = re.compile(r'<div id="post_content.*?>\s(.*?)</div>', re.S)
        contents = re.findall(pattern, content)
        num = len(contents)
        print('共: ' + str(num))
        floor = 1
        file = open('tb.txt', 'w')
        for con in contents:
            # print('\n' + str(floor) + ' 楼---------------------------------------------------------------------------------\n')
            file.writelines('\n' + str(floor) + ' 楼---------------------------------------------------------------------------------\n')
            floor += 1
            # print(self.tool.replace(con))
            file.writelines(self.tool.replace(con))
        file.close()

base_url = 'http://tieba.baidu.com/p/3138733512'
bdtb = BDTB(base_url, 1)
page = bdtb.get_page(1)

title = bdtb.get_title(page)
print('标题: ' + title)

page_num = bdtb.get_page_num(page)
print('共: ' + page_num + '页')

bdtb.get_content(page)
