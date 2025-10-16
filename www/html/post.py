#!/usr/bin/env python3
import cgi
import os

form = cgi.FieldStorage()

print("Content-Type: text/html")
print()  # 헤더 끝
print("<html><head><title>Echo POST</title></head><body>")
print("<h1>POST Data Received:</h1>")
print("<ul>")

for key in form.keys():
    value = form.getvalue(key)
    print(f"<li>{key} = {value}</li>")

print("</ul>")
print("</body></html>")