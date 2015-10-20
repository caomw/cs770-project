#!/usr/bin/env python

import SocketServer

class HandDataListener(SocketServer.BaseRequestHandler):
    """
    The RequestHandler class for our server. Currently only echos received data. This need to be moved into a ROS node.
    """

    def setup(self):
        """
        Initialization happens here (default does nothing).
        :return: none
        """
        print "SocketServer received request from {}...".format(self.client_address[0])

    def handle(self):
        """
        Handle the connection request.
        :return: none
        """
        self.data = self.request.recv(1024).strip()
        print "{} wrote: ".format(self.client_address[0]) + self.data

    def finish(self):
        """
        Clean up happens here (default does nothing).
        :return: none
        """
        print "SocketServer closed by {}...".format(self.client_address[0])


if __name__ == "__main__":
    HOST, PORT = "localhost", 5050

    # Create the server, binding to localhost on port 5050
    print "Starting SocketServer..."
    server = SocketServer.TCPServer((HOST, PORT), HandDataListener)

    # Activate the server; this will keep running until you
    # interrupt the program with Ctrl-C
    print "SocketServer listening for connections..."
    server.serve_forever()
    # server.shutdown() will end server_forever()