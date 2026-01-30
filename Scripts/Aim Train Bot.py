from pyautogui import *
import pyautogui
import time
import keyboard
import win32api, win32con
import threading
from pynput import keyboard

start = False
running = True
kb_listener = None

time.sleep(4)

def on_press(key):
    global running, start
    try:
        if key.char == 'q':
            running = False
    except AttributeError:
        if key == keyboard.Key.esc:
            running = False
            if kb_listener:
                kb_listener.stop()
            return False

def listen_for_keys():
    global kb_listener
    kb_listener = keyboard.Listener(on_press=on_press)
    kb_listener.start()
    kb_listener.join()

def click(x,y):
    win32api.SetCursorPos((x,y))
    win32api.mouse_event(win32con.MOUSEEVENTF_LEFTDOWN,0,0)
    win32api.mouse_event(win32con.MOUSEEVENTF_LEFTUP,0,0)

def main ():
    global start,running, kb_listener
    
    key_listener_thread = threading.Thread(target=listen_for_keys)
    key_listener_thread.daemon = True # Lets the thread run in the background
    key_listener_thread.start()
    
    while running:
        pic = pyautogui.screenshot(region=(765, 245, 1000, 590))
        width, height = pic.size
        
        for x in range(0,1000,5):
            for y in range(0,590,5):
                r,g,b = pic.getpixel((x,y))
                
                if g == 230 and b == 222:
                    click(x+765, y+245)
                    time.sleep(0.1)
                    break

if __name__ == '__main__':
    main()
