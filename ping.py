from tkinter import Canvas, Tk
import subprocess
import time

## Constants
HEIGHT, WIDTH = 200, 500
FRAME_DELAY = 100 #In milliseconds

PING_DENSITY = 20

## Changing Variables
ping_list = []

## Functions
def ping(host, show):
    """ Uses cmd to get the ping of the host.
    :host - The adreess that is being pinged.
    """
    output = -1
    try:
        command = ['ping', '-n', '1', host] #Uses the command 'ping -n 1 {host}'
        output = subprocess.check_output(command, shell=show)
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



