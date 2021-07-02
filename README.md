
![image](https://user-images.githubusercontent.com/26774830/124318534-77141100-db81-11eb-82f8-c52477e0e54a.png)

# debugmenu_mh
It's a port of aap's debug menu for Manhunt, useful thing in terms of convenience. Adds simple gui for various things (almost like imgui).

# How to install
Put debugmenu.dll for your game into the game's root directory.
Plugins that make use of it will load it so DON'T rename the file.


# How to control
- Ctrl-M toggles the debug menu
- Arrow Up/Down moves the selection
- Enter/Backspace enters/exists a submenu
- Arrow Left/Right changes values/executes commands
- the mouse is pretty intuitive

# How to use (for programmers)
Include debugmenu_public.h in your plugin, define 'DebugMenuAPI gDebugMenuAPI'
globally somewhere and load the debugmenu.dll by calling 'bool DebugMenuLoad(void)',
which loads all function pointers and returns whether the library could
be loaded.
NB: it is NOT recommended to do this from DllMain. Hook an early function and
do it from there!

Integer variables can be added to the menu with functions of this type:

MenuEntry *DebugMenuAdd(const char *path, const char *name, INT *ptr, TriggerFunc triggerFunc, INT step, INT lowerBound, INT upperBound, const char **strings)

where INT stands for all common integer types.
Path is a path in the menu in the form "menu|submenu1|submenu2", name is
the name of the entry you want to add.
Ptr is a pointer to the variable, triggerFunc is a function that is called
when the value was changed (can be NULL).
Step, lowerBound and upperBound control how the variable's value is changed.
Strings is an array of strings that are displayed instead of the numbers (for enums).
DebugMenuAddVarBool8/16/32 is provided for convenience, but just calls the respective INT function.

MenuEntry *DebugMenuAdd(const char *path, const char *name, FLOAT *ptr, TriggerFunc triggerFunc, FLOAT step, FLOAT lowerBound, FLOAT upperBound)
Similarly adds floats and doubles but you can't display strings with them.

MenuEntry *DebugMenuAddCmd(const char *path, const char *name, TriggerFunc triggerFunc)
registers a function that is called from the menu, no values are changed.

void DebugMenuEntrySetWrap(MenuEntry *e, bool wrap)
sets whether values should wrap around at the ends. (when e == NULL, this does nothing)

void DebugMenuEntrySetStrings(MenuEntry *e, const char **strings)
changes the strings for a variable. (when e == NULL, this does nothing)
