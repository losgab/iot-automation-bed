# Menu
- Menu task is started from main and handles the creation, start and destruction of programs
- Prints programs available
- Allows for selection of program using buttons
- CONST max_programs = 6

### Variables
- SSD1306 device
- Array of programs
- Cursor location
- Current running program

### Methods
- Add program to array
- Move cursor up/down based on button input & refresh screen
- Select program, call initialising function of program
- End program, call destructor function of program with 2 second hold on button 4, resume main menu

# Programs in Gabe Menu
Programs are based on FreeRTOS tasks that run in threads in the background whenever there is something to do for that program. Programs are added to the menu internally through the menu_add_program() function, although in the future it would be great if programs were saved on the flash and loaded at runtime, whatever that means. I've still got a lot to learn but I'm eager to get a headstart. Program initiation occurs through a function that starts a task which does stuff. 

### Declaring programs
Programs from the menu's perspectiove is a struct that contains:
- Task Handle

Programs have an entry point function, have an infinite loop where functionality is implemented. When the task is no longer needed, the program is deleted and the task terminated. 