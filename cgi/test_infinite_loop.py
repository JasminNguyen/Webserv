#!/usr/bin/env python3

import time
import sys

# CGI must always output headers first
print("Content-Type: text/html")
print("")  # blank line between headers and body
sys.stdout.flush()

# Infinite loop
while True:
    print("<p>Still running...</p>")
    sys.stdout.flush()
    time.sleep(1)  # avoid busy CPU loop
