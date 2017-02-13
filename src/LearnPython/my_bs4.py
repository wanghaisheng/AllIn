from bs4 import BeautifulSoup


html = """
<html><head><title>The Dormouse's story</title></head>
<body>
<p class="title" name="dromouse"><b>The Dormouse's story</b></p>
<p class="story">Once upon a time there were three little sisters; and their names were
<a href="http://example.com/elsie" class="sister" id="link1"><!-- Elsie --></a>,
<a href="http://example.com/lacie" class="sister" id="link2">Lacie</a> and
<a href="http://example.com/tillie" class="sister" id="link3">Tillie</a>;
and they lived at the bottom of a well.</p>
<p class="story">...</p>
"""

soup = BeautifulSoup(html, "lxml")
print(soup.prettify())
print(soup.a)
print(soup.head)

print(soup.a.string)
print(soup.head.string)

print(soup.name)
print(soup.attrs)

print(soup.head.contents)
print(soup.head.children)
for child in soup.head.children:
    print(child)

print("descendants")
for child in soup.descendants:
    print(child)
