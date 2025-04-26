# simple_keylogger
This is a simple keylogger written in cpp. 

Logs pressed keys directly. e.g if you press right shift and h, get "R_SHIFT + h" in log instead just "H".
Logs timestamp in format H:M:S:MS, when particular key is pressed
Logs active window name 

There are some bugs like logging multiple times of pressing special keys, e.g 
11:13:12:974: R_SHIFT + R_SHIFT
11:13:13:008: R_SHIFT + R_SHIFT
11:13:13:042: R_SHIFT + R_SHIFT
11:13:13:077: R_SHIFT + R_SHIFT
but it means key has been held down 

IMPORANT!
As all of my scripts, this was created just for curiosity and learning purposes. I'm not responsible for illegal use of this script. 
