#!/bin/sh
# Minimal CGI shell script

# CGI must always output headers first:
echo "Content-Type: text/html"
echo ""   # Blank line to separate headers from body

# Body
echo "<html>"
echo "<head><title>CGI Test (sh)</title><link rel="icon" href="data:,"></head>"
echo "<body>"
echo "<h1>Hello from sh CGI!</h1>"
#echo "<p>SCRIPT_NAME = $SCRIPT_NAME</p>"
echo "<p>QUERY_STRING = $QUERY_STRING</p>"
printf 'CWD: %s\n' "$(pwd)"
#echo "<p>SERVER_PROTOCOL = $SERVER_PROTOCOL</p>"
echo "</body></html>"