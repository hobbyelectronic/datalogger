#!/user/bin/env python
import sys
import csv
import glob
import tkMessageBox
import serial
import numpy as np
import time
import string
from numpy import arange, sin, pi

import matplotlib.pyplot as plt
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2TkAgg
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg,NavigationToolbar2TkAgg
#matplotlib.use("TkAgg")
from tkColorChooser import askcolor
import ttk
from tkFileDialog import *
from Tkinter import *



def getColor():
	graph = Toplevel()
	global SampleNos
	global Temperature
	graph.wm_title("Downloaded Data")
	graph.wm_iconbitmap(bitmap = "@/home/pi/meter.xbm")
	menubar = Menu(graph)
	filemenu = Menu(menubar, tearoff=0)
	filemenu.add_separator()
	filemenu.add_command(label="Exit", command=root.quit)
	menubar.add_cascade(label="File", menu=filemenu)
	editmenu = Menu(menubar, tearoff=0)
	editmenu.add_command(label="Cut", command=hello)
	editmenu.add_command(label="Copy", command=hello)
	editmenu.add_command(label="Paste", command=hello)
	editmenu.add_command(label="Change Color", command=getColor)
	menubar.add_cascade(label="Graph", menu=editmenu)
	helpmenu = Menu(menubar, tearoff=0)
	helpmenu.add_command(label="About", command=hello)
	menubar.add_cascade(label="Help", menu=helpmenu)
	graph.config(menu=menubar)

	global color
	colorreq = askcolor()
	print color[1]
	f = Figure(figsize=(5, 4), dpi=100)
	a = f.add_subplot(111)
	t = arange(0.0, 3.0, 0.01)
	s = sin(2*pi*t)
	a.plot(SampleNos,Temperature, color=colorreq[1],marker="o")
	canvas = FigureCanvasTkAgg(f, master=graph)
	canvas.show()
	canvas.get_tk_widget().pack()
	toolbar = NavigationToolbar2TkAgg(canvas, graph)
	toolbar.update()
	canvas._tkcanvas.pack()

	
def hello():
    print "hello!"

def openl():
	initial_dir = "/home/pi/"
	filename = askopenfilename(parent=root)
	f = open(filename, 'rb') # opens the csv file
	try:
		reader = csv.reader(f)  # creates the reader object
		for row in reader:   # iterates the rows of the file in orders
			print row    # prints each row
	finally:
		f.close()      # closing
	#cr = csv.reader(open(filename,"rb"))
	#for row in cr:    
	#	print row
	#cr.close()

def savefile():
	global SampleNos
	global Temperature
	global total
	if total == 0:
		tkMessageBox.showinfo("Error", "Please download data first")
	else:
		initial_dir = "/home/pi/"
		fout = asksaveasfile(mode='w', defaultextension=".csv")
		
		for x in range (0,total):
			fout.write(str(SampleNos[x]))
			fout.write(',')
			fout.write(str(Temperature[x]))
			fout.write('\n')
		fout.close()

	



 

def serial_ports():
    """Lists serial ports

    :raises EnvironmentError:
        On unsupported or unknown platforms
    :returns:
        A list of available serial ports
    """
    if sys.platform.startswith('win'):
        ports = ['COM' + str(i + 1) for i in range(256)]

    elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
        # this is to exclude your current terminal "/dev/tty"
        ports = glob.glob('/dev/tty[A-Za-z]*')

    elif sys.platform.startswith('darwin'):
        ports = glob.glob('/dev/tty.*')

    else:
        raise EnvironmentError('Unsupported platform')

    result = []
    for port in ports:
        try:
            s = serial.Serial(port)
            s.close()
            result.append(port)
        except (OSError, serial.SerialException):
            pass
    return result

def start():
	try:
		global portselection
		portselection = portselection.get()
		ser = serial.Serial(
			port = portselection,
			baudrate = 1200,
			parity = serial.PARITY_NONE,
			stopbits = serial.STOPBITS_ONE,
			bytesize = serial.EIGHTBITS,
			timeout = 1)
		
		ser.close()
		ser.open()
		ser.write("s")
		ser.close
	except TypeError as e:
    #Disconnect of USB->UART occured
		self.port.close()
		return None
	except serial.SerialException as e:
		tkMessageBox.showinfo("Error", "Serial Error")
		top.destroy()
		rootpage()
		return 1
		exit
	
	except:
		print portselection
		print Count
		tkMessageBox.showinfo("Error", "Serial Error")
		top.destroy()
		rootpage()
		return 1
		exit


def stop():
	
	Count = 0
	tick = 0
	Serialinput=[]
	global portselection
	portselection = portselection.get()
	try:
		ser = serial.Serial(
			port = portselection,
			baudrate = 1200,
			parity = serial.PARITY_NONE,
			stopbits = serial.STOPBITS_ONE,
			bytesize = serial.EIGHTBITS,
			timeout = 1)	
		ser.close()
		ser.open()
		ser.write("t")
		time.sleep(0.3)
		ser.write('c')
		time.sleep(0.3)
		ser.read(2)
		Downloadlabel = Label (root,text="Downloading data")
		Downloadlabel.grid(row=6, column=1)
		progress = ttk.Progressbar(root,orient=HORIZONTAL,length = 250)
		progress.grid(row = 6, column=2, sticky=W)
		progress.config(mode = 'determinate')
		global total
		total=' '
		total= int(ser.read(5))
		print total
		progress["maximum"] = total
		ser.write('r')
		global SampleNos
		global Temperature
		SampleNos = []
		Temperature = []
		progress["value"] = 0
		progress.start()
		x=' '
		x=ser.read(4)
		while (x != 'E'):
		
			print total
			tick = tick+1
			if (tick >= 20):
				self.port.close()
				tkMessageBox.showinfo("Error", "Disconnected Serial")
				progress.destroy()
				Downloadlabel.destroy()
				top.destroy()
				rootpage()
				exit
				return None
			x=ser.read()
			Serialinput.append(x)		
			if x =='\n':
				print "got here 8"
				Count = Count + 1
				String = "".join(Serialinput)    
				String = String.rstrip()
				String = String.split(',')
				y = int(String[0])
				SampleNos.append(y)
				z= int(String[1])
				Temperature.append(z)
				print "Count"
				print Count 
				tree.insert('', 'end', text=y, values=(str(y*30),str(z)))
				progress["value"] = Count
				progress.update_idletasks()
				Serialinput =[]
				tick=0
					
		print SampleNos
		print Temperature	
		
		ser.close
		progress.stop()
	
		viewdata()
		
	except TypeError as e:
    #Disconnect of USB->UART occured
		self.port.close()
		return None
	except serial.SerialException as e:
		tkMessageBox.showinfo("Error", "Serial Error")
		progress.destroy()
		Downloadlabel.destroy()
		top.destroy()
		rootpage()
		return 1
		exit
	
	except:
		print portselection
		print Count
		tkMessageBox.showinfo("Error", "Serial Error")
		progress.destroy()
		Downloadlabel.destroy()
		top.destroy()
		rootpage()
		return 1
		exit
	
		

def viewdata():
	graph = Toplevel()
	global SampleNos
	global Temperature
	graph.wm_title("Downloaded Data")
	graph.wm_iconbitmap(bitmap = "@/home/pi/meter.xbm")
	menubar = Menu(graph)
	filemenu = Menu(menubar, tearoff=0)
	filemenu.add_command(label="Open", command=openl)
	filemenu.add_command(label="Save", command=hello)
	filemenu.add_separator()
	filemenu.add_command(label="Exit", command=root.quit)
	menubar.add_cascade(label="File", menu=filemenu)
	graph.config(menu=menubar)
	f = Figure(figsize=(5, 4), dpi=100)
	a = f.add_subplot(111)
	t = arange(0.0, 3.0, 0.01)
	s = sin(2*pi*t)
	a.plot(SampleNos,Temperature, 'bo')
	canvas = FigureCanvasTkAgg(f, master=graph)
	canvas.show()
	canvas.get_tk_widget().pack()
	toolbar = NavigationToolbar2TkAgg(canvas, graph)
	toolbar.update()
	canvas._tkcanvas.pack()

def rootpage():	

	root.title("Datalogger Software")
	root.wm_iconbitmap(bitmap = "@/home/pi/meter.xbm")
	plt.style.use('bmh')
	global color
	global SampleNos
	global Temperature
	SampleNos = []
	Temperature = []
	color = 'blue'
	global top
	top = Toplevel()
	top.title("Downloaded Data")
	top.wm_iconbitmap(bitmap = "@/home/pi/meter.xbm")
	menubar = Menu(top)
	filemenu = Menu(menubar, tearoff=0)
	filemenu.add_command(label="Open", command=openl)
	filemenu.add_command(label="Save", command=savefile)
	filemenu.add_separator()
	filemenu.add_command(label="Exit", command=root.quit)
	menubar.add_cascade(label="File", menu=filemenu)

	# display the menu
	top.config(menu=menubar)
	global tree
	tree = ttk.Treeview(top, columns=('col1','col2'),height=15)

	ysb = ttk.Scrollbar(top, orient='vertical', command=tree.yview)
	tree.configure(yscroll=ysb.set)
	tree.grid(row=0, column=0, sticky='nesw')
	ysb.grid(row=0, column=1, sticky='ns')

	top.grid()


	tree.column('col1', width=120, anchor='center')
	tree.column('col2', width=120, anchor='center')

	tree.heading("#0", text='Sample Number', anchor='w')
	tree.column("#0",width=120, anchor="w")
	tree.heading('col1', text='Elapsed Time')
	tree.heading('col2', text='Temperature')
		
	global portselection
	portselection = StringVar() 
	global total
	total = 0


	s = ttk.Style()
	s.theme_use('classic')		
	Startbutton = Button (root,text="Start",command = start)
	Startbutton.grid(row=2, column=0)
	Stopbutton = Button (root,text="Stop",command = stop)
	Stopbutton.grid(row=3, column=0)
	Startlabel = Label (root,text="Start datalogger recording data ")
	Startlabel.grid(row=2, column=1)
	Stoplabel = Label (root,text="Stop datalogger and retrieve data ")
	Stoplabel.grid(row=3, column=1)
	Commslabel = Label (root,text="Select Comms Port")
	Commslabel.grid(row=0, column=1)
	selection = serial_ports()
	selection.insert(0,"Select Port")
	comms = ttk.Combobox (values = selection,textvariable = portselection)
	comms.current(0)
	comms.grid(row = 0, column=2)


root = Tk()
rootpage()
root.mainloop()

