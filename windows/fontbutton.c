// 14 april 2016
#include "uipriv_windows.h"

// TODO implement a custom font dialog that uses DirectWrite?

struct uiFontButton {
	uiWindowsControl c;
	HWND hwnd;
	struct fontDialogParams params;
	BOOL already;
};

uiWindowsDefineControlWithOnDestroy(
	uiFontButton,							// type name
	uiFontButtonType,						// type function
	uiWindowsUnregisterWM_COMMANDHandler(this->hwnd);	// on destroy
)

static void updateFontButtonLabel(uiFontButton *b)
{
	// TODO make this dynamically sized
	// TODO also include the style, but from where? libui or DirectWrite?
	WCHAR text[1024];
	WCHAR *family;

	family = toUTF16(b->params.desc.Family);
	_snwprintf(text, 1024, L"%s %s %g", family, b->params.outStyleName, b->params.desc.Size);
	uiFree(family);
	// TODO error check
	SendMessageW(b->hwnd, WM_SETTEXT, 0, (LPARAM) text);

	// changing the text might necessitate a change in the button's size
	uiWindowsControlQueueRelayout(uiWindowsControl(b));
}

static BOOL onWM_COMMAND(uiControl *c, HWND hwnd, WORD code, LRESULT *lResult)
{
	uiFontButton *b = uiFontButton(c);
	HWND parent;

	if (code != BN_CLICKED)
		return FALSE;

	parent = GetAncestor(b->hwnd, GA_ROOT);		// TODO didn't we have a function for this
	if (showFontDialog(parent, &(b->params))) {
		updateFontButtonLabel(b);
		// TODO event
	}

	*lResult = 0;
	return TRUE;
}

// from http://msdn.microsoft.com/en-us/library/windows/desktop/dn742486.aspx#sizingandspacing
#define buttonHeight 14

static void minimumSize(uiWindowsControl *c, uiWindowsSizing *d, intmax_t *width, intmax_t *height)
{
	uiFontButton *b = uiFontButton(c);
	SIZE size;

	// try the comctl32 version 6 way
	size.cx = 0;		// explicitly ask for ideal size
	size.cy = 0;
	if (SendMessageW(b->hwnd, BCM_GETIDEALSIZE, 0, (LPARAM) (&size)) != FALSE) {
		*width = size.cx;
		*height = size.cy;
		return;
	}

	// that didn't work; fall back to using Microsoft's metrics
	// Microsoft says to use a fixed width for all buttons; this isn't good enough
	// use the text width instead, with some edge padding
	*width = uiWindowsWindowTextWidth(b->hwnd) + (2 * GetSystemMetrics(SM_CXEDGE));
	*height = uiWindowsDlgUnitsToY(buttonHeight, d->BaseY);
}

#if 0
TODO
static void defaultOnClicked(uiButton *b, void *data)
{
	// do nothing
}
#endif

#if 0
TODO
void uiButtonOnClicked(uiButton *b, void (*f)(uiButton *, void *), void *data)
{
	b->onClicked = f;
	b->onClickedData = data;
}
#endif

uiFontButton *uiNewFontButton(void)
{
	uiFontButton *b;

	b = (uiFontButton *) uiNewControl(uiFontButtonType());

	b->hwnd = uiWindowsEnsureCreateControlHWND(0,
		L"button", L"you should not be seeing this",
		BS_PUSHBUTTON | WS_TABSTOP,
		hInstance, NULL,
		TRUE);

	loadInitialFontDialogParams(&(b->params));
	updateFontButtonLabel(b);

	uiWindowsRegisterWM_COMMANDHandler(b->hwnd, onWM_COMMAND, uiControl(b));
//TODO	uiButtonOnClicked(b, defaultOnClicked, NULL);

	uiWindowsFinishNewControl(b, uiFontButton);

	return b;
}
