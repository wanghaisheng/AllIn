import urllib.request
import urllib.parse
import urllib.error
from pymongo import MongoClient
import re
import os


class DoubanBook(object):
    def __init__(self):
        self.base_url = 'https://book.douban.com/tag/'

    def get_page(self, tag):
        try:
            # handles for chinese tag
            tag = urllib.parse.quote(tag)
            url = self.base_url + tag
            url = url.encode('utf-8').decode()

            request = urllib.request.Request(url)
            response = urllib.request.urlopen(request)
            text = response.read().decode('utf-8')
            # print(text)
            return text
        except urllib.error.URLError as e:
            if hasattr(e, "reason"):
                print("err occurs: " + e.reason)

    def parse(self, page):
        pattern = re.compile(r'<li.*?class="subject-item">(.*?)</li>', re.S)
        subject_items = re.findall(pattern, page)
        for item in subject_items:
            print(item)

obj = DoubanBook()
page = obj.get_page("编程")
obj.parse(page)
