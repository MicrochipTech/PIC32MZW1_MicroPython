from test_pkg import post
from test_pkg import about

print("main.py test start...")

p = post.Post()
p.add_post("Python Programming")
author = about.get_author()
email = about.get_email()


print(p.titles)  
print(author)  
print(email)  

print("main.py test end...")