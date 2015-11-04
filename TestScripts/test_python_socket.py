import SocketServer
import thread

class HandDataListener(SocketServer.BaseRequestHandler):
    """
    The RequestHandler class for our server. Currently only echos received data. This need to be moved into a ROS node.
    """

    def setup(self):
        """
        Initialization happens here (default does nothing).
        :return: none
        """
        print("SocketServer received request from {}...".format(self.client_address[0]))
        self.active = True

    def handle(self):
        """
        Handle the data stream connection.
        :return: none
        """
        while self.active:
            self.data = self.request.recv(128).strip()

            if self.data == "":
                self.active = False
            else:
                print("{} wrote: ".format(self.client_address[0]) + self.data)

                # split data
                splitData = self.data.split("|")
                handDisp = splitData[0].split(",")
                handRef = splitData[1]

                # update ref frame
                if(handRef != ""):
                    print("Updated Hand Ref: {}".format(str(handRef.split(","))))

                # update displacement
                print("Hand Disp: {}".format(str(handDisp)))

    def finish(self):
        """
        Clean up happens here (default does nothing).
        :return: none
        """
        print("SocketServer closed by {}...".format(self.client_address[0]))

if __name__ == '__main__':
    HOST, PORT = "localhost", 5050

    # Create the server, binding to localhost on port 5050
    print("Starting SocketServer...")
    server = SocketServer.TCPServer((HOST, PORT), HandDataListener)

    # Activate the server; this will keep running until you
    # interrupt the program with Ctrl-C
    print("SocketServer listening for connections...")
    server.serve_forever()