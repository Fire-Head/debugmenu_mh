#include "debugmenu.h"
#include "inttypes.h"
#include <stdio.h>

#define snprintf _snprintf

#define strdup _strdup

#define MUHKEYS \
	X(leftjustdown, rsLEFT) \
	X(rightjustdown, rsRIGHT) \
	X(upjustdown, rsUP) \
	X(downjustdown, rsDOWN) \
	X(pgupjustdown, rsPGUP) \
	X(pgdnjustdown, rsPGDN)

#define MUHBUTTONS \
	X(button1justdown, 1) \
	X(button2justdown, 2) \
	X(button3justdown, 3)

#define REPEATDELAY 700
#define REPEATINTERVAL 50
#define X(var, keycode) static int var;
MUHKEYS
#undef X
static int downtime;
static int repeattime;
static int lastkeydown;
static int *keyptr;

static int buttondown[3];
static int lastbuttondown;
static int *buttonptr;
static int button1justdown, button2justdown, button3justdown;
static float mouseX, mouseY;

extern CControllerConfigManager *ctrldummy;
static int menuOn;
static int menuInitialized;
static int screenWidth, screenHeight;
static RwRaster *cursor, *arrow;

static int firstBorder = 10;
static int topBorder = 10;
static int leading = 4;
static int gap = 10;
static int minwidth = 100;

void drawMouse(void);
void drawArrow(RwRect r, int direction, int style);

Menu toplevel;
Menu *activeMenu = &toplevel;
Menu *deepestMenu = &toplevel;
Menu *mouseOverMenu;
MenuEntry *mouseOverEntry;
MenuEntry scrollUpEntry("SCROLLUP"), scrollDownEntry("SCROLLDOWN");	// dummies

bool
isMouseInRect(RwRect r)
{
	return (mouseX >= r.x && mouseX < r.x+r.w &&
		mouseY >= r.y && mouseY < r.y+r.h);
}

/*
 * MenuEntry
 */

MenuEntry::MenuEntry(const char *name)
{
	this->type = MENUEMPTY;
	this->name = strdup(name);
	this->next = nil;
	this->menu = nil;
}

MenuEntry_Sub::MenuEntry_Sub(const char *name, Menu *menu)
: MenuEntry(name)
{
	this->type = MENUSUB;
	this->submenu = menu;
}

MenuEntry_Var::MenuEntry_Var(const char *name, int vartype)
: MenuEntry(name)
{
	this->type = MENUVAR;
	this->vartype = vartype;
	this->maxvallen = 0;
	this->wrapAround = false;
}

/*
 * *****************************
 * MenuEntry_Int
 * *****************************
 */

MenuEntry_Int::MenuEntry_Int(const char *name)
: MenuEntry_Var(name, MENUVAR_INT)
{
}

#define X(NAME, TYPE, MAXLEN, FMT) \
int								     \
MenuEntry_##NAME::findStringLen(void){				     \
	TYPE i;							     \
	int len, maxlen = 0;					     \
	for(i = this->lowerBound; i <= this->upperBound; i++){	     \
		len = strlen(this->strings[i-this->lowerBound]);     \
		if(len > maxlen)				     \
			maxlen = len;				     \
	}							     \
	return maxlen;						     \
}								     \
void								     \
MenuEntry_##NAME::processInput(bool mouseOver, bool selected)				     \
{								     \
	TYPE v, oldv;						     \
	int overflow = 0;					     \
	int underflow = 0;					     \
								     \
	v = *this->variable;					     \
	oldv = v;						     \
								     \
	if((selected && leftjustdown) || (mouseOver && button3justdown)){					     \
		v -= this->step;				     \
		if(v > oldv)					     \
			underflow = 1;				     \
	}							     \
	if((selected && rightjustdown) || (mouseOver && button1justdown)){					     \
		v += this->step;				     \
		if(v < oldv)					     \
			overflow = 1;				     \
	}							     \
	if(this->wrapAround){					     \
		if(v > this->upperBound || overflow) v = this->lowerBound;	     \
		if(v < this->lowerBound || underflow) v = this->upperBound;	     \
	}else{									     \
		if(v > this->upperBound || overflow) v = this->upperBound;	     \
		if(v < this->lowerBound || underflow) v = this->lowerBound;	     \
	}							     \
								     \
	*this->variable = v;					     \
	if(oldv != v && this->triggerFunc)			     \
		this->triggerFunc();				     \
}								     \
void											       \
MenuEntry_##NAME::getValStr(char *str, int len)						       \
{											       \
	static char tmp[20];								       \
	if(this->strings){								       \
		snprintf(tmp, 20, "%%%ds", this->maxvallen);				       \
		if(*this->variable < this->lowerBound || *this->variable > this->upperBound){  \
			snprintf(str, len, "ERROR");					       \
			return;								       \
		}									       \
		snprintf(str, len, tmp, this->strings[*this->variable-this->lowerBound]);      \
	}else										       \
		snprintf(str, len, this->fmt, *this->variable);				       \
}											       \
void																			   \
MenuEntry_##NAME::setStrings(const char **strings)													   \
{																			   \
	this->strings = strings;															   \
	if(this->strings)																   \
		this->maxvallen = findStringLen();													   \
}																			   \
MenuEntry_##NAME::MenuEntry_##NAME(const char *name, TYPE *ptr, TriggerFunc triggerFunc, TYPE step, TYPE lowerBound, TYPE upperBound, const char **strings)   \
: MenuEntry_Int(name)																	   \
{																			   \
	this->variable = ptr;																   \
	this->step = step;																   \
	this->lowerBound = lowerBound;															   \
	this->upperBound = upperBound;															   \
	this->triggerFunc = triggerFunc;														   \
	this->maxvallen = MAXLEN;															   \
	this->fmt = FMT;																   \
	this->setStrings(strings);																   \
}
MUHINTS
#undef X

/*
 * *****************************
 * MenuEntry_Float
 * *****************************
 */

#define X(NAME, TYPE, MAXLEN, FMT) \
MenuEntry_##NAME::MenuEntry_##NAME(const char *name, TYPE *ptr, TriggerFunc triggerFunc, TYPE step, TYPE lowerBound, TYPE upperBound)	     \
: MenuEntry_Var(name, MENUVAR_FLOAT)															     \
{																	     \
	this->variable = ptr;														     \
	this->step = step;														     \
	this->lowerBound = lowerBound;													     \
	this->upperBound = upperBound;													     \
	this->triggerFunc = triggerFunc;												     \
	this->maxvallen = MAXLEN;													     \
	this->fmt = FMT;														     \
}																	     \
void																	     \
MenuEntry_##NAME::getValStr(char *str, int len)												     \
{																	     \
	snprintf(str, len, this->fmt, *this->variable);											     \
}																	     \
void																	     \
MenuEntry_##NAME::processInput(bool mouseOver, bool selected)													     \
{																	     \
	float v, oldv;															     \
	int overflow = 0;					     \
	int underflow = 0;					     \
																	     \
	v = *this->variable;														     \
	oldv = v;															     \
																	     \
	if((selected && leftjustdown) || (mouseOver && button3justdown)){					     \
		v -= this->step;				     \
		if(v > oldv)					     \
			underflow = 1;				     \
	}							     \
	if((selected && rightjustdown) || (mouseOver && button1justdown)){					     \
		v += this->step;				     \
		if(v < oldv)					     \
			overflow = 1;				     \
	}							     \
	if(this->wrapAround){					     \
		if(v > this->upperBound || overflow) v = this->lowerBound;	     \
		if(v < this->lowerBound || underflow) v = this->upperBound;	     \
	}else{									     \
		if(v > this->upperBound || overflow) v = this->upperBound;	     \
		if(v < this->lowerBound || underflow) v = this->lowerBound;	     \
	}							     \
																	     \
	*this->variable = v;														     \
	if(oldv != v && this->triggerFunc)												     \
		this->triggerFunc();													     \
}

MUHFLOATS
#undef X

/*
 * *****************************
 * MenuEntry_Cmd
 * *****************************
 */

void
MenuEntry_Cmd::processInput(bool mouseOver, bool selected)
{
	// Don't execute on button3
	if(this->triggerFunc && (selected && (leftjustdown || rightjustdown) || (mouseOver && button1justdown)))
		this->triggerFunc();
}

void
MenuEntry_Cmd::getValStr(char *str, int len)
{
	strncpy(str, "<", len);
}

MenuEntry_Cmd::MenuEntry_Cmd(const char *name, TriggerFunc triggerFunc)
: MenuEntry_Var(name, MENUVAR_CMD)
{
	this->maxvallen = 1;
	this->triggerFunc = triggerFunc;
}


/*
 * *****************************
 * Menu
 * *****************************
 */

void
Menu::scroll(int off) {
	if(isScrollingUp && off < 0)
		scrollStart += off;
	if(isScrollingDown && off > 0)
		scrollStart += off;
	if(scrollStart < 0) scrollStart = 0;
	if(scrollStart > numEntries-numVisible) scrollStart = numEntries-numVisible;
}

void
Menu::changeSelection(int newsel){
	selection = newsel;
	if(selection < 0) selection = 0;
	if(selection >= numEntries) selection = numEntries-1;
	if(selection < scrollStart) scrollStart = selection;
	if(selection >= scrollStart+numVisible) scrollStart = selection-numVisible+1;
}

void
Menu::changeSelection(MenuEntry *sel)
{
	MenuEntry *e;
	int i = 0;
	for(e = this->entries; e; e = e->next){
		if(e == sel){
			this->selection = i;
			this->selectedEntry = sel;
			break;
		}
		i++;
	}
}



MenuEntry*
Menu::findEntry(const char *entryname)
{
	MenuEntry *m;
	for(m = this->entries; m; m = m->next)
		if(strcmp(entryname, m->name) == 0)
			return m;
	return nil;
}

void
Menu::insertEntrySorted(MenuEntry *entry)
{
	MenuEntry **mp;
	int cmp;
	for(mp = &this->entries; *mp; mp = &(*mp)->next){
		cmp = strcmp(entry->name, (*mp)->name);
		if(cmp == 0)
			return;
		if(cmp < 0)
			break;
	}
	entry->next = *mp;
	*mp = entry;
	entry->menu = this;
	this->numEntries++;
}

void
Menu::appendEntry(MenuEntry *entry)
{
	MenuEntry **mp;
	for(mp = &this->entries; *mp; mp = &(*mp)->next);
	entry->next = *mp;
	*mp = entry;
	entry->menu = this;
	this->numEntries++;
}

void
Menu::update(void)
{
	int i;
	int x, y;
	Pt sz;
	MenuEntry *e;
	int onscreen;
	x = this->r.x;
	y = this->r.y + 18;
	int end = this->r.y+this->r.h - 18;
	this->numVisible = 0;

	deepestMenu = this;

	int bottomy = end;
	onscreen = 1;
	i = 0;
	this->maxNameWidth = 0;
	this->maxValWidth = 0;
	this->isScrollingUp = this->scrollStart > 0;
	this->isScrollingDown = false;
	this->selectedEntry = nil;
	for(e = this->entries; e; e = e->next){
		sz = fontGetStringSize(e->name);
		e->r.x = x;
		e->r.y = y;
		e->r.w = sz.x;
		e->r.h = sz.y;

		if(i == this->selection)
			this->selectedEntry = e;

		if(i >= this->scrollStart)
			y += sz.y + leading*fontscale;
		if(y >= end){
			this->isScrollingDown = true;
			onscreen = 0;
		}else
			bottomy = y;
		if(i >= this->scrollStart && onscreen)
			this->numVisible++;

		if(e->type == MENUVAR){
			int valwidth = fontGetLen(((MenuEntry_Var*)e)->getValWidth());
			if(valwidth > maxValWidth)
				maxValWidth = valwidth;
		}
		if(e->r.w > maxNameWidth)
			maxNameWidth = e->r.w;
		i++;
	}
	if(this->r.w < maxNameWidth + maxValWidth + gap*fontscale)
		this->r.w = maxNameWidth + maxValWidth + gap*fontscale;

	this->scrollUpR = this->r;
	this->scrollUpR.h = 16;
	this->scrollDownR = this->scrollUpR;
	this->scrollDownR.y = bottomy;

	// Update active submenu
	if(this->selectedEntry && this->selectedEntry->type == MENUSUB){
		Menu *submenu = ((MenuEntry_Sub*)this->selectedEntry)->submenu;
		submenu->r.x = this->r.x+this->r.w + 10;
		submenu->r.y = this->r.y;
		submenu->r.w = minwidth;	// update menu will expand
		submenu->r.h = this->r.h;
		submenu->update();
	}
}

void
Menu::draw(void)
{
	static char val[100];
	int i;
	MenuEntry *e;
	i = 0;
	for(e = this->entries; e; e = e->next){
		if(i >= this->scrollStart+this->numVisible)
			break;
		if(i >= this->scrollStart){
			int style = FONT_NORMAL;
			if(i == this->selection)
				style = this == activeMenu ? FONT_SEL_ACTIVE : FONT_SEL_INACTIVE;
			if(style != FONT_SEL_ACTIVE && e == mouseOverEntry)
				style = FONT_MOUSE;
			fontPrint(e->name, e->r.x, e->r.y, style);
			if(e->type == MENUVAR){
				int valw = fontGetLen(((MenuEntry_Var*)e)->getValWidth());
				((MenuEntry_Var*)e)->getValStr(val, 100);
				fontPrint(val, e->r.x+this->r.w-valw, e->r.y, style);
			}
		}
		i++;
	}

	if(this->isScrollingUp)
		drawArrow(this->scrollUpR, -1, isMouseInRect(this->scrollUpR));
	if(this->isScrollingDown)
		drawArrow(this->scrollDownR, 1, isMouseInRect(this->scrollDownR));

	if(this->selectedEntry && this->selectedEntry->type == MENUSUB)
		((MenuEntry_Sub*)this->selectedEntry)->submenu->draw();
}

Menu*
findMenu(const char *name)
{
	Menu *m;
	MenuEntry *e;
	char *tmppath = strdup(name);
	char *next, *curname;

	curname = tmppath;
	next = curname;

	m = &toplevel;
	while(*next){
		curname = next;
		while(*next){
			if(*next == '|'){
				*next++ = '\0';
				break;
			}
			next++;
		}
		e = m->findEntry(curname);
		if(e){
			// return an error if the entry exists but isn't a menu
			if(e->type != MENUSUB){
				free(tmppath);
				return nil;
			}
			m = ((MenuEntry_Sub*)e)->submenu;
		}else{
			// Create submenus that don't exist yet
			Menu *submenu = new Menu();
			submenu->parent = m;
			MenuEntry *me = new MenuEntry_Sub(curname, submenu);
			// Don't sort submenus outside the toplevel menu
			if(m == &toplevel)
				m->insertEntrySorted(me);
			else
				m->appendEntry(me);
			m = submenu;
		}
	}

	free(tmppath);
	return m;
}

/*
 * ****************
 * debug menu
 * ****************
 */

void
initDebug(void)
{
	createDebugFonts();

	RwInt32 w, h, d, flags;
	RwImage *img = readTGA(IDR_CURSOR);
	RwImageFindRasterFormat(img, rwRASTERTYPETEXTURE, &w, &h, &d, &flags);
	cursor = RwRasterCreate(w, h, d, flags);
	cursor = RwRasterSetFromImage(cursor, img);
	assert(cursor);
	RwImageDestroy(img);

	img = readTGA(IDR_ARROW);
	RwImageFindRasterFormat(img, rwRASTERTYPETEXTURE, &w, &h, &d, &flags);
	arrow = RwRasterCreate(w, h, d, flags);
	arrow = RwRasterSetFromImage(arrow, img);
	assert(arrow);
	RwImageDestroy(img);
}

void
processInput(void)
{
	int shift = KEYDOWN(rsRSHIFT) || KEYDOWN(rsLSHIFT);
#define X(var, keycode) var = KEYJUSTDOWN(keycode);
	MUHKEYS
#undef X

	// Implement auto-repeat
#define X(var, keycode) \
	if(var){ \
		repeattime = downtime = CGameTime::ms_currGameTime; \
		lastkeydown = keycode; \
		keyptr = &var; \
	}
	MUHKEYS
#undef X
	if(lastkeydown){
		if(KEYDOWN(lastkeydown)){
			int curtime = CGameTime::ms_currGameTime;
			if(curtime - downtime > REPEATDELAY){
				if(curtime - repeattime > REPEATINTERVAL){
					repeattime = curtime;
					*keyptr = 1;
				}
			}
		}else{
			lastkeydown = 0;
		}
	}

	// Also for mouse buttons
#define X(var, num)							  \
	if(var){							  \
		repeattime = downtime = CGameTime::ms_currGameTime;	  \
		lastbuttondown = num;					  \
		buttonptr = &var;					  \
	}
	MUHBUTTONS
#undef X
	if(lastbuttondown){
		if(buttondown[lastbuttondown-1]){
			int curtime = CGameTime::ms_currGameTime;
			if(curtime - downtime > REPEATDELAY){
				if(curtime - repeattime > REPEATINTERVAL){
					repeattime = curtime;
					*buttonptr = 1;
				}
			}
		}else{
			lastbuttondown = 0;
		}
	}

	// Walk through all visible menus and figure out which one the mouse is over
	mouseOverMenu = nil;
	mouseOverEntry = nil;
	Menu *menu;
	for(menu = deepestMenu; menu; menu = menu->parent)
		if(isMouseInRect(menu->r)){
			mouseOverMenu = menu;
			break;
		}
	if(mouseOverMenu){
		// Walk all visibile entries and figure out which one the mouse is over
		MenuEntry *e;
		int i = 0;
		for(e = mouseOverMenu->entries; e; e = e->next){
			if(i >= mouseOverMenu->scrollStart+mouseOverMenu->numVisible)
				break;
			if(i >= mouseOverMenu->scrollStart){
				RwRect r = e->r;
				r.w = mouseOverMenu->r.w;	// span the whole menu
				if(isMouseInRect(r)){
					mouseOverEntry = e;
					break;
				}
			}
			i++;
		}
		if(mouseOverMenu->isScrollingUp && isMouseInRect(mouseOverMenu->scrollUpR)){
			mouseOverEntry = &scrollUpEntry;
			mouseOverEntry->r = mouseOverMenu->scrollUpR;
			mouseOverEntry->menu = mouseOverMenu;
			mouseOverEntry->type = MENUSCROLL;
		}
		if(mouseOverMenu->isScrollingDown && isMouseInRect(mouseOverMenu->scrollDownR)){
			mouseOverEntry = &scrollDownEntry;
			mouseOverEntry->r = mouseOverMenu->scrollDownR;
			mouseOverEntry->menu = mouseOverMenu;
			mouseOverEntry->type = MENUSCROLL;
		}
	}

	if(pgupjustdown)
		activeMenu->scroll(shift ? -5 : -1);
	if(pgdnjustdown)
		activeMenu->scroll(shift ? 5 : 1);
	if(downjustdown)
		activeMenu->changeSelection(activeMenu->selection + (shift ? 5 : 1));
	if(upjustdown)
		activeMenu->changeSelection(activeMenu->selection - (shift ? 5 : 1));

	if(CPad::NewMouseControllerState.wheelUp){
		if(mouseOverMenu)
			activeMenu = mouseOverMenu;
		activeMenu->scroll(shift ? -5 : -1);
	}
	if(CPad::NewMouseControllerState.wheelDown){
		if(mouseOverMenu)
			activeMenu = mouseOverMenu;
		activeMenu->scroll(shift ? 5 : 1);
	}

	if(mouseOverEntry == &scrollUpEntry){
		if(button1justdown){
			activeMenu = mouseOverEntry->menu;
			activeMenu->scroll(shift ? -5 : -1);
		}
	}
	if(mouseOverEntry == &scrollDownEntry){
		if(button1justdown){
			activeMenu = mouseOverEntry->menu;
			activeMenu->scroll(shift ? 5 : 1);
		}
	}

	// Have to call this before processInput below because menu entry can change
	if((button1justdown || button3justdown) && mouseOverEntry){
		activeMenu = mouseOverEntry->menu;
		activeMenu->changeSelection(mouseOverEntry);
	}
	if(KEYJUSTDOWN(rsENTER)){
		if(activeMenu->selectedEntry && activeMenu->selectedEntry->type == MENUSUB)
			activeMenu = ((MenuEntry_Sub*)activeMenu->selectedEntry)->submenu;
	}else if(KEYJUSTDOWN(rsBACKSP)){
		if(activeMenu->parent)
			activeMenu = activeMenu->parent;
	}else{
		if(mouseOverEntry && mouseOverEntry->type == MENUVAR)
			((MenuEntry_Var*)mouseOverEntry)->processInput(true, mouseOverEntry == activeMenu->selectedEntry);
		if(activeMenu->selectedEntry && activeMenu->selectedEntry->type == MENUVAR &&
		   mouseOverEntry != activeMenu->selectedEntry)
			((MenuEntry_Var*)activeMenu->selectedEntry)->processInput(false, true);
	}
}

void
updateMouse(void)
{
	int dirX = 1;
	int dirY = 1;

	//if(MousePointerStateHelper.m_bInvertHorizontally) dirX = -1;
	//if(MousePointerStateHelper.m_bInvertVertically) dirY = -1;

	mouseX += CPad::NewMouseControllerState.X*dirX;
	mouseY += CPad::NewMouseControllerState.Y*dirY;

	if(mouseX < 0.0f) mouseX = 0.0f;
	if(mouseY < 0.0f) mouseY = 0.0f;
	if(mouseX >= screenWidth) mouseX = screenWidth;
	if(mouseY >= screenHeight) mouseY = screenHeight;

	button1justdown = CPad::NewMouseControllerState.lmb && !CPad::OldMouseControllerState.lmb;
	button2justdown = CPad::NewMouseControllerState.mmb && !CPad::OldMouseControllerState.mmb;
	button3justdown = CPad::NewMouseControllerState.rmb && !CPad::OldMouseControllerState.rmb;
	buttondown[0] = CPad::NewMouseControllerState.lmb;
	buttondown[1] = CPad::NewMouseControllerState.mmb;
	buttondown[2] = CPad::NewMouseControllerState.rmb;

	// Zero the mouse position so the camera won't move
	CInputManager::m_mouseState.X=0;
	CInputManager::m_mouseState.Y=0;
	CInputManager::m_mouseState.MouseButton=0;
}

EXPORT void
DebugMenuProcess(void)
{
	// We only process some input here
	if(CTRLJUSTDOWN('M'))
		menuOn = !menuOn;
	if(!menuOn)
		return;
	
	for ( int i = 0; i < 10; i++ )
	{
		CInputManager::m_keysPressed[i].keyCode = 255;
		CInputManager::m_keysPressed[i].bDown = 0;
		CInputManager::m_keysPressed[i].bUp = 0;
	}
	// TODO: this could happen earlier
	if(!menuInitialized){
		initDebug();
		menuInitialized = 1;
	}
	updateMouse();
}

EXPORT void
DebugMenuRender(void)
{
	if(!menuOn)
		return;

	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, 0);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, 0);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEFOGENABLE, 0);
	RwRenderStateSet(rwRENDERSTATECULLMODE, (void*)rwCULLMODECULLNONE);

	RwCamera *cam = (RwCamera*)RWSRCGLOBAL(curCamera);
	screenWidth = RwRasterGetWidth(RwCameraGetRaster(cam));
	screenHeight = RwRasterGetHeight(RwCameraGetRaster(cam));

	if(screenHeight > 1080)
		fontscale = 2;
	else
		fontscale = 1;

	// !!! Important to set up the correct font for update and draw!
	curfont = &vga;
	Pt sz;
	sz = fontPrint("Debug Menu", firstBorder*fontscale, topBorder, 0);

	toplevel.r.x = firstBorder*fontscale;
	toplevel.r.y = topBorder + sz.y + 10;
	toplevel.r.w = minwidth;	// update menu will expand
	toplevel.r.h = screenHeight - 10 - toplevel.r.y;
	toplevel.update();
	toplevel.draw();
	processInput();

	drawMouse();
}



void
drawArrow(RwRect r, int direction, int style)
{
	static RwImVertexIndex indices[] = { 0, 1, 2, 2, 1, 3 };
	static RwIm2DVertex arrowVerts[4];

	RwCamera *cam = (RwCamera*)RWSRCGLOBAL(curCamera);
	float recipz = 1.0f/RwCameraGetNearClipPlane(cam);

	int width = RwRasterGetWidth(arrow);
	int height = RwRasterGetHeight(arrow);

	int left = r.x + (r.w - width)/2;
	int right = left + width;
	int top = r.y;
	int bottom = r.y+r.h;

	float umin = 0.5f / width;
	float vmin = 0.5f / height;
	float umax = (width + 0.5f) / width;
	float vmax = (height + 0.5f) / height;
	if(direction < 0){
		vmin = (height - 0.5f) / height;
		vmax = -0.5f / height;
	}

	if(style){
		RwIm2DVertexSetScreenX(&arrowVerts[0], r.x);
		RwIm2DVertexSetScreenY(&arrowVerts[0], r.y-1);
		RwIm2DVertexSetScreenZ(&arrowVerts[0], RwIm2DGetNearScreenZ());
		RwIm2DVertexSetCameraZ(&arrowVerts[0], RwCameraGetNearClipPlane(cam));
		RwIm2DVertexSetRecipCameraZ(&arrowVerts[0], recipz);
		RwIm2DVertexSetIntRGBA(&arrowVerts[0], 132, 132, 132, 255);

		RwIm2DVertexSetScreenX(&arrowVerts[1], r.x+r.w);
		RwIm2DVertexSetScreenY(&arrowVerts[1], r.y-1);
		RwIm2DVertexSetScreenZ(&arrowVerts[1], RwIm2DGetNearScreenZ());
		RwIm2DVertexSetCameraZ(&arrowVerts[1], RwCameraGetNearClipPlane(cam));
		RwIm2DVertexSetRecipCameraZ(&arrowVerts[1], recipz);
		RwIm2DVertexSetIntRGBA(&arrowVerts[1], 132, 132, 132, 255);

		RwIm2DVertexSetScreenX(&arrowVerts[2], r.x);
		RwIm2DVertexSetScreenY(&arrowVerts[2], r.y+r.h+1);
		RwIm2DVertexSetScreenZ(&arrowVerts[2], RwIm2DGetNearScreenZ());
		RwIm2DVertexSetCameraZ(&arrowVerts[2], RwCameraGetNearClipPlane(cam));
		RwIm2DVertexSetRecipCameraZ(&arrowVerts[2], recipz);
		RwIm2DVertexSetIntRGBA(&arrowVerts[2], 132, 132, 132, 255);

		RwIm2DVertexSetScreenX(&arrowVerts[3], r.x+r.w);
		RwIm2DVertexSetScreenY(&arrowVerts[3], r.y+r.h+1);
		RwIm2DVertexSetScreenZ(&arrowVerts[3], RwIm2DGetNearScreenZ());
		RwIm2DVertexSetCameraZ(&arrowVerts[3], RwCameraGetNearClipPlane(cam));
		RwIm2DVertexSetRecipCameraZ(&arrowVerts[3], recipz);
		RwIm2DVertexSetIntRGBA(&arrowVerts[3], 132, 132, 132, 255);

		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, nil);
		RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERLINEAR);
		RwIm2DRenderIndexedPrimitive(rwPRIMTYPETRILIST, arrowVerts, 4, indices, 6);
	}


	RwIm2DVertexSetScreenX(&arrowVerts[0], left);
	RwIm2DVertexSetScreenY(&arrowVerts[0], top);
	RwIm2DVertexSetScreenZ(&arrowVerts[0], RwIm2DGetNearScreenZ());
	RwIm2DVertexSetCameraZ(&arrowVerts[0], RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetRecipCameraZ(&arrowVerts[0], recipz);
	RwIm2DVertexSetIntRGBA(&arrowVerts[0], 255, 255, 255, 255);
	RwIm2DVertexSetU(&arrowVerts[0], umin, recipz);
	RwIm2DVertexSetV(&arrowVerts[0], vmin, recipz);

	RwIm2DVertexSetScreenX(&arrowVerts[1], right);
	RwIm2DVertexSetScreenY(&arrowVerts[1], top);
	RwIm2DVertexSetScreenZ(&arrowVerts[1], RwIm2DGetNearScreenZ());
	RwIm2DVertexSetCameraZ(&arrowVerts[1], RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetRecipCameraZ(&arrowVerts[1], recipz);
	RwIm2DVertexSetIntRGBA(&arrowVerts[1], 255, 255, 255, 255);
	RwIm2DVertexSetU(&arrowVerts[1], umax, recipz);
	RwIm2DVertexSetV(&arrowVerts[1], vmin, recipz);

	RwIm2DVertexSetScreenX(&arrowVerts[2], left);
	RwIm2DVertexSetScreenY(&arrowVerts[2], bottom);
	RwIm2DVertexSetScreenZ(&arrowVerts[2], RwIm2DGetNearScreenZ());
	RwIm2DVertexSetCameraZ(&arrowVerts[2], RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetRecipCameraZ(&arrowVerts[2], recipz);
	RwIm2DVertexSetIntRGBA(&arrowVerts[2], 255, 255, 255, 255);
	RwIm2DVertexSetU(&arrowVerts[2], umin, recipz);
	RwIm2DVertexSetV(&arrowVerts[2], vmax, recipz);

	RwIm2DVertexSetScreenX(&arrowVerts[3], right);
	RwIm2DVertexSetScreenY(&arrowVerts[3], bottom);
	RwIm2DVertexSetScreenZ(&arrowVerts[3], RwIm2DGetNearScreenZ());
	RwIm2DVertexSetCameraZ(&arrowVerts[3], RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetRecipCameraZ(&arrowVerts[3], recipz);
	RwIm2DVertexSetIntRGBA(&arrowVerts[3], 255, 255, 255, 255);
	RwIm2DVertexSetU(&arrowVerts[3], umax, recipz);
	RwIm2DVertexSetV(&arrowVerts[3], vmax, recipz);

	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, arrow);
	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERLINEAR);
	RwIm2DRenderIndexedPrimitive(rwPRIMTYPETRILIST, arrowVerts, 4, indices, 6);
}

void
drawMouse(void)
{
	static RwImVertexIndex indices[] = { 0, 1, 2, 2, 1, 3 };
	static RwIm2DVertex vertices[4];
	RwIm2DVertex *vert;
	RwCamera *cam;
	cam = (RwCamera*)RWSRCGLOBAL(curCamera);
	float x = mouseX;
	float y = mouseY;
	float w = RwRasterGetWidth(cursor);
	float h = RwRasterGetHeight(cursor);
	float recipz = 1.0f/RwCameraGetNearClipPlane(cam);

	float umin = 0.5f / w;
	float vmin = 0.5f / h;
	float umax = (w + 0.5f) / w;
	float vmax = (h + 0.5f) / h;

	vert = vertices;
	RwIm2DVertexSetScreenX(vert, x);
	RwIm2DVertexSetScreenY(vert, y);
	RwIm2DVertexSetScreenZ(vert, RwIm2DGetNearScreenZ());
	RwIm2DVertexSetCameraZ(vert, RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetRecipCameraZ(vert, recipz);
	RwIm2DVertexSetIntRGBA(vert, 255, 255, 255, 255);
	RwIm2DVertexSetU(vert, umin, recipz);
	RwIm2DVertexSetV(vert, vmin, recipz);
	vert++;

	RwIm2DVertexSetScreenX(vert, x+w);
	RwIm2DVertexSetScreenY(vert, y);
	RwIm2DVertexSetScreenZ(vert, RwIm2DGetNearScreenZ());
	RwIm2DVertexSetCameraZ(vert, RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetRecipCameraZ(vert, recipz);
	RwIm2DVertexSetIntRGBA(vert, 255, 255, 255, 255);
	RwIm2DVertexSetU(vert, umax, recipz);
	RwIm2DVertexSetV(vert, vmin, recipz);
	vert++;

	RwIm2DVertexSetScreenX(vert, x);
	RwIm2DVertexSetScreenY(vert, y+h);
	RwIm2DVertexSetScreenZ(vert, RwIm2DGetNearScreenZ());
	RwIm2DVertexSetCameraZ(vert, RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetRecipCameraZ(vert, recipz);
	RwIm2DVertexSetIntRGBA(vert, 255, 255, 255, 255);
	RwIm2DVertexSetU(vert, umin, recipz);
	RwIm2DVertexSetV(vert, vmax, recipz);
	vert++;

	RwIm2DVertexSetScreenX(vert, x+w);
	RwIm2DVertexSetScreenY(vert, y+h);
	RwIm2DVertexSetScreenZ(vert, RwIm2DGetNearScreenZ());
	RwIm2DVertexSetCameraZ(vert, RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetRecipCameraZ(vert, recipz);
	RwIm2DVertexSetIntRGBA(vert, 255, 255, 255, 255);
	RwIm2DVertexSetU(vert, umax, recipz);
	RwIm2DVertexSetV(vert, vmax, recipz);
	vert++;

	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, cursor);
	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERLINEAR);
	RwIm2DRenderIndexedPrimitive(rwPRIMTYPETRILIST, vertices, 4, indices, 6);
}
