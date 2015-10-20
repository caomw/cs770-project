#!/usr/bin/env python

import SocketServer
#import rospy
#from geometry_msgs.msg import Pose
#from std_msgs.msg import Int8

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
        # self.pose_pub = rospy.Publisher('HandPose', Pose, queue_size=50)
        # self.state_pub = rospy.Publisher('HandState', Int8, queue_size=50)

    def handle(self):
        """
        Handle the data stream connection.
        :return: none
        """
        active = True
        while active: #and not rospy.is_shutdown():
            self.data = self.request.recv(1024).strip()

            if self.data == "":
                active = False
            else:
                print "{} wrote: ".format(self.client_address[0]) + self.data
                # split_data = self.data.split(",")
                #
                # newPose = geometry_msgs.msg.Pose()
                # newPose.position.x = float(split_data[0])
                # newPose.position.y = float(split_data[1])
                # newPose.position.z = float(split_data[2])
                # newPose.orientation.x = float(split_data[3])
                # newPose.orientation.y = float(split_data[4])
                # newPose.orientation.z = float(split_data[5])
                # newPose.orientation.w = float(split_data[6])
                #
                # hand_state = std_msgs.msg.Int8(int(split_data[7]))
                #
                # self.pose_pub.publish(newPose)
                # self.state_pub.publish(hand_state)

    def finish(self):
        """
        Clean up happens here (default does nothing).
        :return: none
        """
        print "SocketServer closed by {}...".format(self.client_address[0])
        # self.pose_pub.unregister()
        # self.state_pub.unregister()
        # if rospy.is_shutdown():
        #     self.shutdown()


if __name__ == "__main__":
    HOST, PORT = "localhost", 5050

    # Create the server, binding to localhost on port 5050
    print "Starting SocketServer..."
    server = SocketServer.TCPServer((HOST, PORT), HandDataListener)

    # rospy.init_node('HandDataListener', anonymous=True)

    # Activate the server; this will keep running until you
    # interrupt the program with Ctrl-C
    print "SocketServer listening for connections..."
    server.serve_forever()
    # server.shutdown() will end server_forever()