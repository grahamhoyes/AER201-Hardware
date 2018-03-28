from tkinter import*
from PIL import ImageTk, Image
import serial

#arduinoSerial = serial.Serial("/dev/ttyACM0", 9600)

class PillMePleasePCInterface:
    def __init__(self, master):
        self.master = master
        master.title("Pill Me Please")

        self.label = Label(master, text="This is the PC user interface for the Pill Me Please Pill Boxing Machine!", padx = 15, pady = 10)
        self.label.pack()
        
        initImg = Image.open(initImagePath)
        initImg2 = initImg.resize((340,350),Image.ANTIALIAS)
        initImg = ImageTk.PhotoImage(initImg2)
        self.panel = Label(master, image = initImg)
        self.panel.image = initImg
        self.panel.pack()

        self.dl_button = Button(master, text="Download Data", command=self.downloadData)
        self.dl_button.pack()
        
        self.view_button = Button(master, text="View History", command=self.viewHistory)
        self.view_button.pack()
        
        self.del_button = Button(master, text="Delete History", command=self.delHistory)
        self.del_button.pack()

        self.close_button = Button(master, text="Close", pady = 20, command=master.quit)
        self.close_button.pack()
    
    def downloadData(self):
        #arduinoSerial.write('d')
        #response = arduinoSerial.readlines()
        response = ["hello world 6", "hello world 7", "hello world 8", "hello world 9"]
        
        f = open(filename, "r")
        content = f.readlines()
        content = [x.strip() for x in content]
        
        for line in response:
            if line not in content:
                line_prepender(filename, line)
        
        print("Downloaded!")
        
    def viewHistory(self):  
        logs = ""
        f = open(filename, "r")
        
        content = f.readlines()
        content = [x.strip() for x in content]
        
        for line in content:
            logs += decode(line) 
            logs += "\n"
            
        clickView(logs)
        
        print("Displaying history")

    def delHistory(self):
        f = open(filename, "w")
        f.write("")
        print("Logs cleared!")

def line_prepender(filename, line):
    with open(filename, 'a+') as f:
        content = f.read()
        f.seek(0, 0)
        f.write(line.rstrip('\r\n') + '\n' + content)
        
def clickView(logs):
    toplevel = Tk()
    toplevel.title("Pill Me Please History")
    
    message = Label(toplevel, text="History of machine operation (from oldest to newest):", height=0, width=100)
    message.pack()

    logs = Label(toplevel, text=logs, height=0, width=100)
    logs.pack()

    close_button = Button(toplevel, text="Close", pady = 20, command=toplevel.destroy)
    close_button.pack()
    
    center(toplevel)

def decode(line):
    decodedLine = line[::-1]
    return decodedLine

def center(toplevel):
    toplevel.update_idletasks()
    w = toplevel.winfo_screenwidth()
    h = toplevel.winfo_screenheight()
    size = tuple(int(_) for _ in toplevel.geometry().split('+')[0].split('x'))
    x = w/2 - size[0]/2
    y = h/2 - size[1]/2
    toplevel.geometry("%dx%d+%d+%d" % (size + (x, y)))


if __name__ == '__main__':
    initImagePath = "pillmeplease.png"
    filename = "PillMePlease_Log.txt"
    
    app = Tk()
    my_gui = PillMePleasePCInterface(app)
    center(app)
    
    app.mainloop()

