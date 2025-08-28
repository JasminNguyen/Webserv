#!/usr/bin/env python3

import os

# CGI requires printing HTTP headers first
#print("Content-Type: text/html\n")

print("<html>")
print("<head><title>CGI Test</title></head>")
print("<body>")
print("<h1>Hello from Python CGI!</h1>")

# Show some environment variables (CGI passes these)
#print("<h2>Environment Variables</h2>")
#print("<ul>")
#for key, value in os.environ.items():
   # print(f"<li>{key} = {value}</li>")
#print("</ul>")

print("</body></html>")
