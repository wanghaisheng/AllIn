import urllib.parse
import urllib.request
from bs4 import BeautifulSoup
 
values = {"username":"yong80408813","password":"1988329"}
data = urllib.parse.urlencode(values)
binary_data = data.encode('utf-8')
url = "https://passport.csdn.net/account/login?from=http://my.csdn.net/my/mycsdn"
request = urllib.request.Request(url,binary_data)
response = urllib.request.urlopen(request)
print (response.read())