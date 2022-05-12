from tkinter import Canvas, Tk
import subprocess
import time

## Constants
HEIGHT, WIDTH = 200, 500
FRAME_DELAY = 100 #In milliseconds

ROOT = Tk()
CANVAS = Canvas(ROOT, width=WIDTH, height=HEIGHT, bg="#ffffff")

PING_DENSITY = 20

## Changing Variables
ping_list = []

## Backgroung Structure
BACK_COLOR = ['#ff0000', '#00ff00']

## Functions
def ping(host):
    """ Uses cmd to get the ping of the host.
    :host - The adreess that is being pinged.
    """
    output = -1
    try:
        command = ['ping', '-n', '1', host] #Uses the command 'ping -n 1 {host}'
        output = subprocess.check_output(command, shell=True)
        output = output.decode()
        output = convert(output)
        
    except:
        print("Lost connection.")
    
    return output

def convert(text):
    """ Converts decoded information from cmd to the values of the ping.
    :text - the output of the cmd code.
    """
    
    text_begin = 'Average ='
    text_end = 'ms'

    value = text[text.find(text_begin) + len(text_begin) : ].strip()
    value = value[ : value.find(text_end)]

    return value

def convert_hex(value):
    """ Converts hex string into rgb tuple.
    :value - the hex string to be converted.
    """
    
    h = value.lstrip('#')
    
    rgb = tuple(int(h[i:i+2], 16) for i in (0, 2, 4))

    return rgb
    

def convert_rgb(value):
    """ Converts rgb tuple into hex string.
    :value - the rgb tuple to be converted.
    """
    
    rgb = value

    h = '#' + ('%02x%02x%02x' % rgb)

    return h

def color(value):
    pass

def background():
    """ Creates the background color. """

    color_step = 255 // HEIGHT
    
    for height in range(HEIGHT):
        line_color = convert_rgb((255 - color_step * height, color_step * height, 0))
        CANVAS.create_rectangle(0, height, WIDTH, height, fill=line_color, outline=line_color)

def draw():
    """ Draws the visuals of the current ping. """
    
    CANVAS.delete("all") #Clear Canvas
    background()

    removed_ping = 0
    
    if len(ping_list) > PING_DENSITY:
        removed_ping = int(ping_list.pop(0))
    
    list_length = len(ping_list)
    line_length = WIDTH/list_length

    for i, p in enumerate(ping_list):
        determinant = min(i, 1)
        
        previous_ping = int(ping_list[i - 1])
        previous_height = HEIGHT - (previous_ping * determinant) - (removed_ping * (not determinant))
        previous_start = line_length * (i)
        
        current_ping = int(p)
        current_height = HEIGHT - current_ping
        current_start = line_length * (i + 1)
        
        CANVAS.create_line(previous_start, previous_height, current_start, current_height, fill='#111111')

def stats():
    """ Prints the statsticts onto the screen. """
    color = '#ffffff'
    location = (100, WIDTH-100)
    font_size = 10
    height = font_size

    average = sum(map(int, ping_list)) // len(ping_list)
    highest = max(map(int, ping_list))

    if list(map(int, ping_list)).count(-1) < 3:
        font_size = str(font_size)
        CANVAS.create_text(location[1], height, text=f'Average: {average}ms', fill='#00ffff', font=(f'Helvetica {font_size} bold'))
        CANVAS.create_text(location[0], height, text=f'Highest: {highest}ms', fill='#ffff00', font=(f'Helvetica {font_size} bold'))

        CANVAS.create_line(0, HEIGHT - average, WIDTH, HEIGHT - average, fill='#00ffff')
        CANVAS.create_line(0, HEIGHT - highest, WIDTH, HEIGHT - highest, fill='#ffff00')
    else:
        CANVAS.create_text(WIDTH/2, height, text=f'No Connection', fill='#ffffff', font=(f'Helvetica {font_size} bold'))
        

def loop():
    """ The loop in the canvas. """
    
    ping_list.append(ping('8.8.8.8'))

    draw()
    stats()
    print('Average:', sum(map(int, ping_list)) // len(ping_list), 'Highest:', max(map(int, ping_list)))
    
    ROOT.after(FRAME_DELAY, loop)

def main():
    """ Where the main event happens. """
    
    ROOT.title("Current Ping")
    
    CANVAS.pack()
    
    ROOT.after(FRAME_DELAY, loop)
    ROOT.mainloop()

## Run
main()



