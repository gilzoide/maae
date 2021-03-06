#include "maae.h"
#include "argpstuff.h"

int main (int argc, char *argv[]) {
	const char *file_name = arguments (argc, argv);
	CursInit ();
	
	// initialize stuff
	//  cursor
	Cursor cursor;
	InitCursor (&cursor);
	//  defaults
	Direction default_direction = RIGHT;
	mos_attr default_attr = Normal;
	//  copy buffer
	CopyBuffer buffer;
	InitCopyBuffer (&buffer);
	//  images
	IMGS everyone;
	InitIMGS (&everyone);
	int current_index = 0;

	int c = 0;
	// Mouse support: bt1 and bt3 click
	MEVENT event;
	mousemask (BUTTON1_CLICKED | BUTTON1_DOUBLE_CLICKED |
			BUTTON3_CLICKED, NULL);

	// the current image in edition
	CURS_MOS *current = NULL;
	// we really need a current image, so ask for it until user creates it!
	// but if asked to open a file in argv, creates an empty MOSAIC and loads it
	if (file_name) {
		current = NewCURS_MOS (0, 0);
		// try to load...
		int load_return = LoadCURS_MOS (current, file_name);
		// ... it may be alright...
		if (load_return == 0 || load_return == EUNKNSTRGFMT) {
			CircularIMGS (&everyone, current);
			InitSaveLoadMOSAIC (file_name);
		}
		// ...but it might go wrong
		else {
			FreeCURS_MOS (current);
			current = NULL;
		}
	}
	while (!current) {
		current = CreateNewMOSAIC (&everyone, current);
	}
	ENTER_(REDRAW);

	// main loop!
	while (!IS_(QUIT)) {
		if (c == KEY_F(10)) {
			c = Menu ();
		}

		switch (c) {
			/* if nothing is returned by the menu, do nothing */
			case 0:
				break;

			/* ESC: exit selection mode */
			case KEY_ESC:
				UN_(SELECTION);
				UnprintSelection (current);
				break;

			/* Mouse event: clicked the window, or help/menu/quit */
			case KEY_MOUSE:
				getmouse (&event);
				// bt1 double click: select until
				if (event.bstate & BUTTON1_DOUBLE_CLICKED) {
					ENTER_(SELECTION);
				}
				// bt3 (right button) click: Menu (yep, anywhere)
				else if (event.bstate & BUTTON3_CLICKED) {
					ungetch (KEY_F(10));
					break;
				}
				// bt1: move to mouse anyway
				MoveTo (&cursor, current, event.y, event.x);
				break;
				
			/* move up */
			case KEY_UP:
				Move (&cursor, current, UP);
				break;

			/* move down */
			case KEY_DOWN:
				Move (&cursor, current, DOWN);
				break;

			/* move left */
			case KEY_LEFT:
				Move (&cursor, current, LEFT);
				break;

			/* move right */
			case KEY_RIGHT:
				Move (&cursor, current, RIGHT);
				break;

			/* next click change direction */
			case KEY_CTRL_D:
				DefaultDirection (&default_direction);
				break;
				
			/* previous mosaic */
			case KEY_PPAGE:
				current = current->prev;
				current_index = current_index - 1;
				if (current_index < 0) {
					current_index = everyone.size - 1;
				}
				VPrintHud (FALSE, "img %d", current_index);
				break;
				
			/* next mosaic */
			case KEY_NPAGE:
				current = current->next;
				current_index = current_index + 1;
				if (current_index >= everyone.size) {
					current_index = 0;
				}
				VPrintHud (FALSE, "img %d", current_index);
				break;

			/* go to page */
			case KEY_CTRL_G:
				{
					int index = VPrintHud (SCAN,
							"img %d. New image index (0~%d):",
							current_index, everyone.size - 1);
					if (index != ERR) {
						CURS_MOS *aux = GoToPage (&everyone, index);
						if (aux) {
							current = aux;
							current_index = index;
						}
						else {
							PrintHud (FALSE, "Invalid index");
						}
					}
				}
				break;

			/* move to first */
			case KEY_HOME:
				MoveAll (&cursor, current, REVERSE (default_direction));
				break;
			
			/* move to last */
			case KEY_END:
				MoveAll (&cursor, current, default_direction);
				break;

			/* show help */
			case KEY_F(1):
				Help ();
				DisplayCurrent (current);
				break;
				
			/* new mosaic */
			case KEY_F(2):
				{
					CURS_MOS *aux = CreateNewMOSAIC (&everyone, current);
					// aux was really created, so update the current curs_mos
					if (aux) {
						current = aux;
					}
				}
				break;

			/* show about Window */
			case KEY_F(12):
				About ();
				break;
				
			/* save mosaic */
			case KEY_CTRL_S:
				switch (Save (current)) {
					case 0:
						PrintHud (FALSE, "Saved successfully!");
						break;

					case ERR:	// canceled
						break;

					default:
						PrintHud (TRUE, "Sorry, no can save this... =/");
						break;
				}
				UN_(TOUCHED);
				break;
				
			/* load mosaic */
			case KEY_CTRL_O:
				switch (Load (current)) {
					case 0:
						PrintHud (FALSE, "Loaded successfully!");
						ENTER_(TOUCHED | REDRAW);
						break;

					case ERR:	// canceled
						break;

					case ENOENT:
						PrintHud (TRUE, "File doesn't exist");
						break;

					case ENODIMENSIONS:
						PrintHud (TRUE, "No dimensions in this file, dude! =/");
						break;

					case EUNKNSTRGFMT:
						PrintHud (TRUE, "Sorry, couldn't load attributes...");
						break;

					default:
						PrintHud (TRUE, "Sorry, no can load this... =/");
						break;
				}
				break;
				
			/* resize mosaic */
			case KEY_CTRL_R:
				Resize (current, &cursor);
				break;
				
			/* box selection mode! */
			case KEY_CTRL_B:
				TOGGLE_(SELECTION);
				UnprintSelection (current);
				break;
			
			/* select all */
			case KEY_CTRL_A:
				// from the beggining...
				MoveTo (&cursor, current, 0, 0);
				// ...select...
				ENTER_(SELECTION);
				// ...until the end
				MoveTo (&cursor, current, 
						current->img->height - 1, current->img->width - 1);
				break;

			/* Trim MOSAIC */
			case KEY_CTRL_K:
				if (AskMessage ("Trim the mosaic?")) {
					int resize = AskMessage ("Resize it?");
					// clear screen, as it may resize
					ClearWin (current);
					// Trim and ask if want to resize it
					TrimCURS_MOS (current, resize);
					// move to inside the resized MOSAIC
					MoveResized (&cursor, current);
					PrintHud (FALSE, "Trimmed");
					ENTER_(REDRAW);
				}
				break;

			/* toggle paint mode (or just paint SELECTION) */
			case KEY_CTRL_P:
				InformToggleState (PAINT, "Paint mode ON", "Paint mode OFF");
				default_attr = curs_mosGetAttr (current, cursor.origin_y,
						cursor.origin_x);
				break;

			/* toggle transparent paste */
			case KEY_CTRL_T:
				InformToggleState (TRANSPARENT, "Transparent paste ON",
						"Transparent paste OFF");
				break;
				
			/* copy selected area */
			case KEY_CTRL_C:	
				UnprintSelection (current);
				UN_(SELECTION);
				Copy (&buffer, current, cursor);
				break;

			/* cut selected area */
			case KEY_CTRL_X:
				UnprintSelection (current);
				UN_(SELECTION);
				Cut (&buffer, current, cursor);
				break;

			/* paste copy buffer */
			case KEY_CTRL_V:	
				UnprintSelection (current);
				UN_(SELECTION);
				// if the buffer was never used, sry
				if (!Paste (&buffer, current, cursor)) {
					PrintHud (TRUE, "Nothing in the buffer...");
				}
				else {
					ENTER_(TOUCHED);
				}
				break;

			/* enter moving mode */
			case KEY_CTRL_N:
				UnprintSelection (current);
				UN_(SELECTION);
				PrintHud (FALSE, "Move selection. ENTER to accept, ESC to cancel");
				cursor = MoveSelection (current, cursor);
				break;

			/* toggle insert mode */
			case KEY_IC:
				InformToggleState (INSERT, "Insert mode ON", "Insert mode OFF");
				break;

			/* attribute table */
			case '\t':
				default_attr = AttrTable (current, cursor);
				ChAttrs (current, &cursor, default_attr);
				break;
				
			/* quit; aww =/ */
			case KEY_CTRL_Q:
				// asks if you really want to quit this tottally awesome SW
				if (IS_(TOUCHED) && !AskQuit ()) {
					break;
				}

				ENTER_(QUIT);
				// needed to jump the getch ()
				ungetch (0);
				break;
				
			/* erase entire line/column before cursor
			 * it actually calls enough times the backspace button */
			case KEY_CTRL_U:
				EraseLine (current);
				break;

			/* erase word (until space is found) */
			case KEY_CTRL_W:
				EraseWord (&cursor, current, REVERSE (default_direction));
				break;
			
			// WARNING: don't change BACKSPACE nor DC out of here nor out of
			//  order, as they deppend on 'default' 
			//  (so I guess you know we shouldn't put any 'break's either)
			/* Backspace: delete the char before (the curses definition 
			 * says something else, but in general it's 127) */
			case KEY_BACKSPACE: case 127:
				// in selection mode backspace acts just as delete, so no moving
				if (!IS_(SELECTION)) {
					Move (&cursor, current, REVERSE (default_direction));
				}
			/* delete: well, just erase the damn char 
			 * (put a ' ' in it, default takes care of this for us =P) */
			case KEY_DC:	
				ENTER_(NO_MOVING_CURSOR);
				c = ' ';

			/* write at the mosaic, and show it to us */
			default:
				if (isprint (c)) {
					InsertCh (current, &cursor, c, default_attr, default_direction);
					ChAttrs (current, &cursor, c != ' ' ? default_attr : Normal);
					ENTER_(TOUCHED);
					// didn't erase anything, so move to the next
					if (!IS_(NO_MOVING_CURSOR)) {
						Move (&cursor, current, default_direction);
					}
					else { 	// we erased something, so it erased and that's that
						UN_(NO_MOVING_CURSOR);
					}
				}
				break;
		}
		
		// paint mode: paint it right away!
		if (IS_(PAINT)) {
			ChAttrs (current, &cursor, default_attr);
			ENTER_(TOUCHED);
		}
		DisplayCurrent (current);
		UpdateHud (cursor, default_direction);
		
		c = getch ();
	}
	
	DestroyCopyBuffer (&buffer);
	DestroyIMGS (&everyone);
	DestroyWins ();

	return 0;
}
