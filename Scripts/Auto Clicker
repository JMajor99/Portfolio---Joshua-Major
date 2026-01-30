import threading
from pynput import keyboard, mouse
import pyautogui as pag
import time

# To control the mouse 
mouse_controller = mouse.Controller()
clicking = False
running = True
kb_listener = None
mouse_listener = None

def get_coord(x, y):
    print("Now at : {}".format((x, y)))

# Make function to check wether we need to be clicking
def on_press(key):
    global clicking, running
    try:
        if key.char == 's':
            clicking = True
        elif key.char == 'e':
            clicking = False
        elif key.char == 'q':
            running = False
            clicking = False
    except AttributeError:
        if key == keyboard.Key.esc:
            clicking = False
            running = False
            if mouse_listener:
                mouse_listener.stop()
            if kb_listener:
                kb_listener.stop()
            return False
            
def listen_for_keys():
    global kb_listener
    kb_listener = keyboard.Listener(on_press=on_press)
    kb_listener.start()
    kb_listener.join()
        
def find_coord():
    global mouse_listener
    mouse_listener = mouse.Listener(on_move = get_coord)
    mouse_listener.start()
    mouse_listener.join()
        
def main():
    global kb_listener, mouse_listener, running
    
    key_listener_thread = threading.Thread(target=listen_for_keys)
    key_listener_thread.daemon = True # Lets the thread run in the background
    key_listener_thread.start()

    movement_listener_thread = threading.Thread(target=find_coord)
    movement_listener_thread.daemon = True
    movement_listener_thread.start()
    
    while running:
        # The Auto Clicker
        if clicking:
            mouse_controller.click(mouse.Button.left, 1) # Makes it click left button on mouse
            time.sleep(1) # Speed of click
        else:
            time.sleep(0.1)
        
if __name__ == '__main__':
    main()

#pag.moveTo(600,600)
