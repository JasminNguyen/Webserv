<?php
// Minimal CGI PHP script

// CGI headers
//header("Content-Type: text/html");

// Body
echo "<html>";
echo "<head><title>CGI Test (PHP)</title></head>";
echo "<body>";
echo "<h1>Hello from PHP CGI!</h1>";
echo "<p>REQUEST_METHOD = " . $_SERVER['REQUEST_METHOD'] . "</p>";
echo "<p>SCRIPT_NAME = " . $_SERVER['SCRIPT_NAME'] . "</p>";
echo "<p>QUERY_STRING = " . $_SERVER['QUERY_STRING'] . "</p>";
echo "<p>SERVER_PROTOCOL = " . $_SERVER['SERVER_PROTOCOL'] . "</p>";
echo "</body></html>";
?>