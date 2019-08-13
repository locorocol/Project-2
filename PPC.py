#!/usr/bin/python

"""PPC.py: This program creates a user interface to control a Mechademic 500 
Robotic Arm over TCP socket
"""

__author__      = "Kelson Kaiser, Fabian Sutandyo, Preston Fowler"
__copyright__   = "June 2018"



import socket
import math
import numpy
from threading import Thread
import tkinter as tk
import tkinter.scrolledtext as tkst
from datetime import datetime

TCP_PORT = 10000
s=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
BUFFER_SIZE=128
roundTo=3                   #round to decimal places
trimamount=1
jointVel=45
xyz=[0.0,0.0,0.0,0.0,0.0,0.0]       #xyz coordinates
origin=[0.0,0.0,0.0]          #xyz origin
matrix=[[1,0,0],[0,1,0],[0,0,1]]    #rotation matrix defining the orientation
connected = False

def is_connected(hostname):
  try:
    host = socket.gethostbyname(hostname)
    s = socket.create_connection((host, 80), 2)
    return True
  except:
     pass
  return False

def detect(input):
    if input.islower() == True:
        printbox("ERROR: Wrong input format ")
        return False
    else:
        return True
    
def printbox(text):
    if detect(text):  
        textbox.configure(state=tk.NORMAL)
        textbox.insert(tk.END, "[%s] %s\n" % (datetime.now().time().strftime('%H:%M'),text))
        textbox.see(tk.END)
        textbox.configure(state=tk.DISABLED)
        

def printIP(text):
    if validateIP(text):  
        textbox.configure(state=tk.NORMAL)
        textbox.insert(tk.END, "[%s] %s\n" % (datetime.now().time().strftime('%H:%M'),text))
        textbox.configure(state=tk.DISABLED)
        
def cos(degree):       #cosine of the degree
    return math.cos(math.pi*float(degree/180))

def sin(degree):       #sine of the degree angle
    return math.sin(math.pi*float(degree/180))

def isNumber(num):          #returns if the given can be cast to a float
    try:
        float(num)
        return True
    except ValueError:
        return False

def userCoorToRotationMatrix(roll, pitch, yaw):         #ZYX conversion to rotation matrix 
    global matrix
    matrix= [[cos(yaw)*cos(pitch), cos(yaw)*sin(pitch)*sin(roll)-cos(roll)*sin(yaw), sin(yaw)*sin(roll)+cos(yaw)*cos(roll)*sin(pitch)],
                 [cos(pitch)*sin(yaw), cos(yaw)*cos(roll)+sin(yaw)*sin(pitch)*sin(roll), cos(roll)*sin(yaw)*sin(pitch)-cos(yaw)*sin(roll)],
                 [-sin(pitch), cos(pitch)*sin(roll), cos(pitch)*cos(roll)]]

def getRoboCoor():             #get the robot's orientaiton angles
    global matrix
    #in radians:
    if matrix[0][2] !=1 and matrix[0][2] !=-1:
        betarad=math.asin(matrix[0][2])
        alpharad=math.atan2(-matrix[1][2]/math.cos(betarad),matrix[2][2]/math.cos(betarad))
        gammarad=math.atan2(-matrix[0][1]/math.cos(betarad),matrix[0][0]/math.cos(betarad))
    else:
        betarad=math.pi/2
        alpharad=0
        gammarad=-alpharad+math.atan2(matrix[1][0],matrix[1][1])
    #to degree
    beta=round(betarad*180/math.pi,roundTo)
    alpha=round(alpharad*180/math.pi,roundTo)
    gamma=round(gammarad*180/math.pi,roundTo)
    #avoid -0.0
    if beta==-0.0:
        beta=0.0
    if alpha==-0.0:
        alpha=0.0
    if gamma==-0.0:
        gamma=0.0
    return [alpha,beta,gamma]
    
def getUserCoor():             #get the user's orientation angles
    global matrix
    #in radians:
    if matrix[2][0] !=1 and matrix[2][0] !=-1:
        betarad=-math.asin(matrix[2][0])
        alpharad=math.atan2(matrix[2][1]/math.cos(betarad),matrix[2][2]/math.cos(betarad))
        gammarad=math.atan2(matrix[1][0]/math.cos(betarad),matrix[0][0]/math.cos(betarad))
    else:
        betarad=math.pi/2
        alpharad=0
        gammarad=alpharad-math.atan2(matrix[0][1],matrix[0][2])
        
    #to degree
    beta=round(betarad*180/math.pi,roundTo)
    alpha=round(alpharad*180/math.pi,roundTo)
    gamma=round(gammarad*180/math.pi,roundTo)
    #avoid -0.0
    if beta==-0.0:
        beta=0.0
    if alpha==-0.0:
        alpha=0.0
    if gamma==-0.0:
        gamma=0.0
    return [alpha,beta,gamma]

def rec():                  #receive message from robot
    global connected
    try:
        r = str(s.recv(BUFFER_SIZE), 'ASCII')       #thread will terminate if there is no motion to clear
    except ConnectionAbortedError:
        r=""
    except OSError:
        r=""
    if r[1:5] != "2006":            #does not print if trying to clear a non-existing error
        printbox(r)
    
    if r[1:5] == "3000":
        connected = True
        
    if r[1:5] == '2002':
        sendCom('MovePose(250,0,150,0,90,0)\0')

def validateIP(s):
    a = s.split('.')
    if len(a) != 4:
        return False
    for x in a:
        if not x.isdigit():
            return False
        i = int(x)
        if i < 0 or i > 255:
            return False
    return True

def connect():                      #connects the socket to the robot and activates it
    TCP_IP=e1.get()                 #NEED check for IP format
    if validateIP(TCP_IP):
        s.connect((TCP_IP,TCP_PORT))
        rec()
        sendCom('ActivateRobot\0')
        home()
        #sendCom("ResetError\0")
    else:
        printbox("Invalid IP Address")
    

def sendCom(MESSAGE):               #sends the given command and calls rec()
    if connected:
        b = bytes(MESSAGE, 'ASCII')
        s.sendall(b)
        if MESSAGE[0:8] == "MovePose":
            angles = getUserCoor()
            printbox( "Move To: ("+str(xyz[0]-origin[0])+","+str(xyz[1]-origin[1])+","+str(xyz[2]-origin[2])+","+str(angles[0])+","+str(angles[1])+","+str(angles[2])+")")
        
        elif MESSAGE != "ResetError\0":
            printbox(MESSAGE)
        #creates a parallel thread to wait for response
        worker = Thread(target=rec, args=())
        worker.setDaemon(True)
        worker.start()

def rewrite():                      #updates the Gui
    for i in range(3):
        e[i].delete(0, tk.END)
        e[i].insert(0, round(xyz[i]-origin[i],roundTo))
    for i in range(3,6):
        e[i].delete(0, tk.END)
        e[i].insert(0, round(getUserCoor()[i-3],roundTo))
    vel.delete(0, tk.END)
    vel.insert(0, jointVel)

def home():                         #homes the robot
    global xyz
    sendCom('Home\0')
    xyz=[250.0,0.0,150.0]
    userCoorToRotationMatrix(0,90,0)
    rewrite()
    
def stop():                         #stops the robot as soon as it recieves this command
    sendCom('ClearMotion\0')
    #If a motion is interupted, this program loses the location of the arm
    #delete this loop if reading robot arm location from socket using GetPose
    for i in range(6):
        e[i].delete(0, tk.END)
        e[i].insert(0, "UNKOWN")
        
def moveTo():                       #moves the robot to the location given in the GUI
    global xyz
    global jointVel
    for i in range(0,3):            #xyz
        if (len(e[i].get()) > 0) and isNumber(e[i].get()):
            xyz[i]=float(e[i].get())+origin[i]
    angles=[]
    for i in range(0,3):            #roll pitch yaw
        if (len(e[i+3].get()) > 0) and isNumber(e[i+3].get()):
            angles=angles + [float(e[i+3].get())]
    if (len(angles)==3):
        userCoorToRotationMatrix(angles[0],angles[1],angles[2])
    enteredVel=vel.get()
    if isNumber(enteredVel) and (float(enteredVel)<=135 and float(enteredVel)>=1):
        if float(enteredVel)!=jointVel:
            jointVel=enteredVel
            sendCom("SetJointVel("+enteredVel+")\0")
    message="MovePose("+str(xyz[0])+","+str(xyz[1])+","+str(xyz[2])+","+str(getRoboCoor()[0])+","+str(getRoboCoor()[1])+","+str(getRoboCoor()[2])+")\0"
    sendCom(message)
    rewrite()
    sendCom("ResetError\0")
    
def trim(var,sign):                 #adjusts the given orientation by the trimamount along given axis
    global xyz
    global matrix
    global trimamount
    global jointVel
    if isNumber(trimsize.get()) and float(trimsize.get())!=trimamount:
        trimamount=float(trimsize.get())
    else:
        trimsize.delete(0, tk.END)
        trimsize.insert(0,trimamount)
    
    enteredVel=vel.get()
    if isNumber(enteredVel) and (float(enteredVel)<=135 and float(enteredVel)>=1):
        if float(enteredVel)!=jointVel:
            jointVel=enteredVel
            sendCom("SetJointVel("+enteredVel+")\0")
    
    if var<3:                       #var is x (0), y (1), or z (2)
        xyz[var]=xyz[var]+sign*trimamount
    else:                           #var is an angle roll (3), pitch (4), yaw (5)
        radangle=trimamount*math.pi/180
        if var==3:
            trimmatrix=[[1,0,0],[0,math.cos(radangle),-math.sin(sign*radangle)],[0,math.sin(sign*radangle),math.cos(radangle)]]
        elif var==4:
            trimmatrix=[[math.cos(radangle),0,math.sin(sign*radangle)],[0,1,0],[-math.sin(sign*radangle),0,math.cos(radangle)]]
        else:
            trimmatrix=[[math.cos(radangle),-math.sin(sign*radangle),0],[math.sin(sign*radangle),math.cos(radangle),0],[0,0,1]]
        matrix=numpy.matmul(trimmatrix,matrix)          #roation is relative to global [Rotation]*[Orientation]
    
    message="MovePose("+str(xyz[0])+","+str(xyz[1])+","+str(xyz[2])+","+str(getRoboCoor()[0])+","+str(getRoboCoor()[1])+","+str(getRoboCoor()[2])+")\0"
    sendCom(message)
    rewrite()
    sendCom("ResetError\0")
    
def setOrigin():                    #sets the origin to the x,y,z from the move boxes
    global origin
    for i in range(3):
        if (len(e[i].get()) > 0) and isNumber(e[i].get()):
            origin[i]=float(e[i].get())+origin[i]
    rewrite()

def reset():                        #resets the origin, velocity, and any errors
    global origin
    origin=[0.0,0.0,0.0]
    sendCom("ResetError\0")
    sendCom("SetJointVel(45)\0")
    vel.delete(0, tk.END)
    vel.insert(0,"45")
    rewrite()
    printbox(reset)
    
    
root = tk.Tk()
root.title("Precision Platform Control")
root.geometry("700x500")
root.resizable(0, 0)

#frame = tk.Frame(master = root, bg='black')
#frame.grid_propagate(0)
#frame.grid(fill=tk.BOTH, expand=1)

#connect and home
tk.Label(root, text="IP Address:").grid(row=0)
e1 = tk.Entry(root)
e1.insert(0,"172.25.181.22")				#default IP address
e1.grid(row=0,column=1)
tk.Button(root, text="Connect", command=connect).grid(row=0,column=2)

#input coordinates
e=[tk.Entry(root),tk.Entry(root),tk.Entry(root),tk.Entry(root),tk.Entry(root),tk.Entry(root)]
tk.Label(root, text="Move To:").grid(row=1)

tk.Button(root, text="Set Origin", command= setOrigin).grid(row=1,column=2,pady=10)
tk.Button(root, text="Reset", command= reset).grid(row=1, column=3,pady=10)

tk.Label(root, text="X:").grid(row=2)
e[0].grid(row=2,column=1,pady=5)
tk.Label(root, text="Roll:").grid(row=2,column=2)
e[3].grid(row=2,column=3,pady=5)

tk.Label(root, text="Y:").grid(row=3)
e[1].grid(row=3,column=1,pady=5)
tk.Label(root, text="Pitch:").grid(row=3,column=2)
e[4].grid(row=3,column=3,pady=5)

tk.Label(root, text="Z:").grid(row=4)
e[2].grid(row=4,column=1,pady=5)
tk.Label(root, text="Yaw:").grid(row=4,column=2)
e[5].grid(row=4,column=3,pady=5)

tk.Button(root, text="Move", command=moveTo).grid(row=5,pady=5)
tk.Button(root, text="STOP", command=stop).grid(row=5,column=1,pady=5)

tk.Label(root, text="Velocity (1-135 degrees/s):").grid(row=5,column=2,pady=5)
vel=tk.Entry(root)
vel.grid(row=5,column=3,pady=5)
vel.insert(0,"45")    #default is 45

#trim coordinates
tk.Label(root, text="Trim Coordinates:").grid(row=6)
trimsize=tk.Entry(root)
trimsize.grid(row=6,column=1,pady=5)
trimsize.insert(0, trimamount)
tk.Button(root, text="X-", command= lambda: trim(0,-1)).grid(row=7,pady=5)
tk.Button(root, text="X+", command= lambda: trim(0,1)).grid(row=7,column=1,pady=5)
tk.Button(root, text="Y-", command= lambda: trim(1,-1)).grid(row=8,pady=5)
tk.Button(root, text="Y+", command= lambda: trim(1,1)).grid(row=8,column=1,pady=5)
tk.Button(root, text="Z-", command= lambda: trim(2,-1)).grid(row=9,pady=5)
tk.Button(root, text="Z+", command= lambda: trim(2,1)).grid(row=9,column=1,pady=5)

tk.Button(root, text="Roll-", command= lambda: trim(3,-1)).grid(row=7,column=2,pady=5)
tk.Button(root, text="Roll+", command= lambda: trim(3,1)).grid(row=7,column=3,pady=5)
tk.Button(root, text="Pitch-", command= lambda: trim(4,-1)).grid(row=8,column=2,pady=5)
tk.Button(root, text="Pitch+", command= lambda: trim(4,1)).grid(row=8,column=3,pady=5)
tk.Button(root, text="Yaw-", command= lambda: trim(5,-1)).grid(row=9,column=2,pady=5)
tk.Button(root, text="Yaw+", command= lambda: trim(5,1)).grid(row=9,column=3,pady=5)

tk.Label(root, text="                           Command History and Feedback:").grid(row=10)
textbox=tkst.ScrolledText(root, width = 60, height=8)
textbox.grid(row=30, column = 0, columnspan = 10)
textbox.configure(state=tk.DISABLED)
if is_connected("www.google.com"):
    printbox("Connected to internet")
else:
    printbox("Not connected to internet")
    printbox("Please check connection before starting")

root.mainloop()

def recv():                  #receive message from robot
    try:
        r = str(s.recv(BUFFER_SIZE), 'ASCII')       #thread will terminate if there is no motion to clear
    except ConnectionAbortedError:
        r=""
    except OSError:
        r=""
    if r[0:5] != "[2006]":            #does not print if trying to clear a non-existing error
        print(r)
if connected:
    s.sendall(bytes('DeactivateRobot\0', 'ASCII'))

#creates a parallel thread to possibly recieve responce
worker = Thread(target=recv, args=())
worker.setDaemon(True)
worker.start()

s.close()
