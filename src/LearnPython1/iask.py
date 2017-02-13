import urllib.request
import urllib.error
import urllib.parse
import re
import mysql.connector


class Post(object):
    def __init__(self, url_suffix, title, reply, date):
        self.url_suffix = url_suffix
        self.title = title
        self.reply = reply
        self.date = date


class StorePosts(object):
    def __init__(self, posts):
        self.posts = posts
        try:
            self.cnn = mysql.connector.connect(host='127.0.0.1', port=3306, user='root', password='1958', database='test')
            # cursors
            self.cur = self.cnn.cursor(buffered=True)
        except mysql.connector.Error as e:
                print('连接数据库失败!{}'.format(e))

    def store(self):
        for post in self.posts:
            try:
                # execute 使用tuple进行参数传递, 而不是单个参数传递
                self.cur.execute("INSERT INTO posts(suffix,title,reply,date) VALUES(%s, %s, %s, %s)", (post.url_suffix, post.title, post.reply, post.date))
            except mysql.connector.Error as e:
                print('insert post error!{}'.format(e))

    def show(self):
        try:
            self.cur.execute("SELECT * FROM posts")
            print(self.cur.fetchall())
        except mysql.connector.Error as e:
                print('查询数据库失败!{}'.format(e))

    def stop(self):
        # 调用commit将数据更新到数据库
        self.cnn.commit()
        self.cur.close()
        self.cnn.close()


class IAsk(object):
    def __init__(self, request_url):
        # 外语学习
        self.request_url = request_url
        self.base_url = 'http://iask.sina.com.cn'
        self.posts = []

    def get_page(self, page_num):
        request = urllib.request.Request(self.request_url)
        response = urllib.request.urlopen(request)
        page = response.read().decode('utf-8')
        # print(page)
        return page

    def get_total_page(self, page):
        pattern = re.compile(r'<a href="/c/187-all-86988-new.html" class="">(.*?)</a>')
        result = re.search(pattern, page)
        if result:
            return result.group(1).strip()
        else:
            return None

    def get_posts(self, page):
        pattern = re.compile(r'<div class="question-title">.*?<a href="(.*?)" target="_blank" title.*?>(.*?)</a>.*?</div>', re.S)
        other_pattern = re.compile(r'<div class="queation-other">.*?<span class="fl answer_num db">(.*?)</span>.*?<span>(.*?)</span>.*?</div>', re.S)
        result = re.findall(pattern, page)
        other_result = re.findall(other_pattern, page)
        # print(str(len(result)) + ', ' + str(len(other_result)))
        if len(result) != len(other_result):
            return None

        i = 0
        while i < len(result):
            post = Post(result[i][0].strip(), result[i][1].strip(), other_result[i][0].strip(), other_result[i][1].strip())
            self.posts.append(post)
            i += 1

        j = 0
        while j < len(result):
            print(self.base_url + self.posts[j].url_suffix + ', ' + self.posts[j].title +
                  self.posts[j].reply + ', ' + self.posts[j].date)
            j += 1

        return self.posts
        # for item in result:
        #     print(item)
        # for item in other_result:
        #     print(item)

    # 所有页的url
    def get_pages_url(self, page):
        pattern = re.compile(r'<a href=(.*?).html">(.*?)</a>', re.S)
        result = re.findall(pattern, page)
        for item in result:
            print(item)


ask = IAsk('http://iask.sina.com.cn/c/187.html')
page = ask.get_page(1)

pages = ask.get_total_page(page)
print(pages)

store = StorePosts(ask.get_posts(page))
store.store()
store.show()

store.stop()

# ask.get_pages_url(page)
