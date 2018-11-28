#!/usr/bin/python3

import socket
import ssl
from http.server import BaseHTTPRequestHandler, HTTPServer
import time
import zlib
import os
import re

hostName = "home.eee"
hostPort = 3333
validIds = ["123", "321"]

class MyServer(BaseHTTPRequestHandler):

    file_count = 0
    
    #	GET is for clients geting the predi
    def do_GET(self):
        if re.search(r'\d+', self.path).group() in validIds:
            directory = os.path.dirname(self.path)
            ans = ""
            if (not os.path.isdir(directory)) : 
                os.makedirs(directory)
                ans = "NONE"
            else :
                for f in os.listdir(directory):
                    print(f)
                    if f.endswith(".part"):
                        ans += f + " "
                ans += '4'
            if (ans == ""):
                ans = "NONE"
            self.send_response(200, ans)
            self.end_headers()
            self.connection.close()
        else:
            self.send_response(403, "Invalid ID")
            self.end_headers()
            self.connection.close()
    #	POST is for submitting data.
    def do_POST(self):

        print( "incomming http: ", self.path )
        content_length = int(self.headers['Content-Length']) # <--- Gets the size of data
        post_data = self.rfile.read(content_length) # <--- Gets the data itself
        with open('./%s' % self.path, 'wb') as out:
            out.write(zlib.decompress(post_data))
        self.send_response(200, "OK")
        self.end_headers()
        self.connection.close()

if __name__ == "__main__":
    myServer = HTTPServer((hostName, hostPort), MyServer)
    
    #myServer.socket = ssl.wrap_socket(myServer.socket, certfile='../certs/key.pem', server_side=True)
    
    print(time.asctime(), "Server Starts - %s:%s" % (hostName, hostPort))

    try:
            myServer.serve_forever()
    except KeyboardInterrupt:
            pass

    myServer.server_close()
    print(time.asctime(), "Server Stops - %s:%s" % (hostName, hostPort))
