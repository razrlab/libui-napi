#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ui.h"
#include "napi_utils.h"
#include "control.h"
#include "event-loop.h"
#include "events.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#ifdef __APPLE__
#include <objc/objc-runtime.h>
#include <CoreFoundation/CoreFoundation.h>
#define NSLocalizedString(key) \
	((CFStringRef (*)(id, SEL, CFStringRef, CFStringRef, id))objc_msgSend)( \
	((id (*)(Class, SEL))objc_msgSend)(objc_getClass("NSBundle"), SELUID("mainBundle")), \
	SELUID("localizedStringForKey:value:table:"), CFSTR(key), CFSTR(""), nil)
#define CREATE_MENU(title) \
	((id (*)(id, SEL, CFStringRef))objc_msgSend)( \
		((id (*)(Class, SEL))objc_msgSend)( \
			objc_getClass("NSMenu"), SELUID("alloc") \
		), SELUID("initWithTitle:"), NSLocalizedString(title))
#define SET_SUBMENU(menu, submenu, title) \
	((void (*)(id, SEL, id, id))objc_msgSend)( \
		menu, SELUID("setSubmenu:forItem:"), submenu, \
		((id (*)(id, SEL, CFStringRef, SEL, CFStringRef))objc_msgSend)( \
		menu, SELUID("addItemWithTitle:action:keyEquivalent:"), \
		CFSTR(title), nil, CFSTR("")))
#define ADD_MENU_ITEM(menu, label, sel, key) \
	((id (*)(id, SEL, CFStringRef, SEL, CFStringRef))objc_msgSend)( \
		menu, SELUID("addItemWithTitle:action:keyEquivalent:"), \
		NSLocalizedString(label), SELUID(sel), CFSTR(key))
#define ADD_MENU_SEPARATOR(menu) \
	((id (*)(id, SEL, id))objc_msgSend)( \
		menu, SELUID("addItem:"), ((id (*)(Class, SEL))objc_msgSend)( \
			objc_getClass("NSMenuItem"), SELUID("separatorItem")))
#endif

static const char *MODULE = "App";

struct children_list *visible_windows = NULL;

static int c_wrap_cb(void *data) {
	struct event_t *event = (struct event_t *)data;
	napi_value ret = fire_event(event);
	if (ret == NULL) {
		return 0;
	}
	int ret_int;
	napi_get_value_int32(event->env, ret, &ret_int);
	return ret_int;
}

LIBUI_FUNCTION(onShouldQuit) {
	INIT_ARGS(1);
	ARG_CB_REF(cb_ref, 0);

	struct event_t *event = create_event(env, cb_ref, "onShouldQuit");
	if (event == NULL) {
		return NULL;
	}

	uiOnShouldQuit(c_wrap_cb, event);

	return NULL;
}

LIBUI_FUNCTION(startTimer) {
	INIT_ARGS(2);
	ARG_INT32(ms, 0);
	ARG_CB_REF(cb_ref, 1);

	struct event_t *event = create_event(env, cb_ref, "startTimer");
	if (event == NULL) {
		return NULL;
	}

	uiTimer(ms, c_wrap_cb, event);

	return NULL;
}

LIBUI_FUNCTION(init) {
	uiInitOptions o;
	memset(&o, 0, sizeof(uiInitOptions));
	const char *err = uiInit(&o);
	if (err != NULL) {
		napi_throw_error(env, NULL, err);
		uiFreeInitError(err);
	}
	controls_map = ctrl_map_create(0, 1);
	visible_windows = create_children_list();
	ln_init_loop_status();

	napi_value global;
	napi_status status = napi_get_global(env, &global);
	CHECK_STATUS_THROW(status, napi_get_global);

	status = napi_create_reference(env, global, 1, &null_ref);
	CHECK_STATUS_THROW(status, napi_create_reference);

#ifdef _WIN32
	SetProcessDPIAware();
#endif

#ifdef __APPLE__
	id application = ((id (*)(Class, SEL))objc_msgSend)(
		objc_getClass("NSApplication"), SELUID("sharedApplication")
	);
	id main_menu = ((id (*)(id, SEL))objc_msgSend)(
		application, SELUID("mainMenu")
	);
	id submenu = CREATE_MENU("Edit");
	ADD_MENU_ITEM(submenu, "Undo", "undo:", "z");
	ADD_MENU_ITEM(submenu, "Redo", "redo:", "Z");
	ADD_MENU_SEPARATOR(submenu);
	ADD_MENU_ITEM(submenu, "Cut", "cut:", "x");
	ADD_MENU_ITEM(submenu, "Copy", "copy:", "c");
	ADD_MENU_ITEM(submenu, "Paste", "paste:", "v");
	ADD_MENU_ITEM(submenu, "Delete", "delete:", "\b");
	ADD_MENU_ITEM(submenu, "Select All", "selectAll:", "a");
	ADD_MENU_SEPARATOR(submenu);
	id find_menu = CREATE_MENU("Find");
	ADD_MENU_ITEM(find_menu, "Find…", "performTextFinderAction:", "f");
	ADD_MENU_ITEM(find_menu, "Find Next", "performTextFinderAction:", "g");
	ADD_MENU_ITEM(find_menu, "Find Previous", "performTextFinderAction:", "G");
	ADD_MENU_ITEM(find_menu, "Use Selection for Find", "performTextFinderAction:", "e");
	ADD_MENU_ITEM(find_menu, "Jump to Selection", "centerSelectionInVisibleArea:", "j");
	SET_SUBMENU(submenu, find_menu, "Find");
	SET_SUBMENU(main_menu, submenu, "Edit");
#endif

	return NULL;
}

napi_value _libui_init_core(napi_env env, napi_value exports) {
	DEFINE_MODULE();
	LIBUI_EXPORT(onShouldQuit);
	LIBUI_EXPORT(init);
	LIBUI_EXPORT(startTimer);
	return module;
}
