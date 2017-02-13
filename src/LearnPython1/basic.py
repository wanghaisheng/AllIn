
# list test
classmates = ['Michael', 'Bob', 'Tracy']
print(len(classmates))
print(classmates)

# dict test
d = {'Mich': 95, 'Bob': 75, 'Tracy': 85}
print(d)

existed = 'Jim' in d
print(existed)

existed = 'Tracy' in d
print(existed)

print(d.get('Mich'))

d.pop('Tracy')
print(d)


# string operation
# s.isalnum()  # 所有字符都是数字或者字母
# s.isalpha()  # 所有字符都是字母
# s.isdigit()  # 所有字符都是数字
# s.islower()  # 所有字符都是小写
# s.isupper()  # 所有字符都是大写
# s.istitle()  # 所有单词都是首字母大写，像标题
# s.isspace()  # 所有字符都是空白字符、\t、\n
#
# s.upper() #把所有字符中的小写字母转换成大写字母
# s.lower() #把所有字符中的大写字母转换成小写字母
# s.capitalize()  #把第一个字母转化为大写字母，其余小写
# s.title()  #把每个单词的第一个字母转化为大写，其余小写

s = 'kailani'
print(s.upper())
