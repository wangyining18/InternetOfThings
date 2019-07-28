import http.server
import socketserver
import sys

port = int(sys.argv[1])

Handler = http.server.SimpleHTTPRequestHandler

httpd = socketserver.TCPServer(("", port), Handler)
print("serving at port", port)
httpd.serve_forever()

