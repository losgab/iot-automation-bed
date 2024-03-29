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
