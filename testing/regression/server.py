#!/usr/bin/env python
######################################################################
# This script must be run for all the buttons on the generated
# web pages to work. 
######################################################################

import BaseHTTPServer, CGIHTTPServer
import sys, os, threading, time
            
def runServer(host, port):
    """Run the HTTP server under the given host name and port.
    """
    httpd = BaseHTTPServer.HTTPServer((host,port), CGIHTTPServer.CGIHTTPRequestHandler)
    httpd.serve_forever()


class NullStream:
    """This class is used to ignore log messages."""
    def write(self, *args):
        pass

###############################################################

host = "localhost"
port = 24678
   
# Disable the logging messages from the HTTP server...
sys.stderr = NullStream() 

print "Starting HTTP server thread..."
server = threading.Thread(target=runServer, args=(host,port), name="RenderTester HTTP Server Thread")
server.setDaemon(True)
server.start()
print "Point your browser to http://%s:%d/html for viewing the test results."%(host,port)
print "Press Ctrl-C to quit the server."

# Do nothing but wait for the Ctrl-C...
try:
    while 1:
        time.sleep(60)
except KeyboardInterrupt:
    print "Quit server..."
