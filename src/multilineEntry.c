#include "ui.h"
#include "napi_utils.h"
#include "control.h"
#include "events.h"

static const char *MODULE = "MultilineEntry";

#ifdef __APPLE__
#include <objc/objc-runtime.h>
#include <CoreFoundation/CoreFoundation.h>
struct uiDarwinControl {
	uiControl c;
	uiControl *parent;
	BOOL enabled;
	BOOL visible;
	void *SyncEnableState;
	void *SetSuperview;
	void *HugsTrailingEdge;
	void *HugsBottom;
	void *ChildEdgeHuggingChanged;
	void *HuggingPriority;
	void *SetHuggingPriority;
	void *ChildVisibilityChanged;
};
struct uiMultilineEntry {
	struct uiDarwinControl c;
	id sv; // NSScrollView
	id tv; // intrinsicSizeTextView
};
inline void fix_darkmode(uiMultilineEntry *me)
{
	((void (*)(id, SEL, id))objc_msgSend)(
		me->tv, SELUID("setBackgroundColor:"),
		((id (*)(Class, SEL))objc_msgSend)(
			objc_getClass("NSColor"), SELUID("textBackgroundColor")
		)
	);
	((void (*)(id, SEL, id))objc_msgSend)(
		me->tv, SELUID("setTextColor:"),
		((id (*)(Class, SEL))objc_msgSend)(
			objc_getClass("NSColor"), SELUID("textColor")
		)
	);
	((void (*)(id, SEL, CFStringRef))objc_msgSend)(
		me->tv, SELUID("setString:"), CFSTR("\n")
	);
}
#endif

LIBUI_FUNCTION(create) {
	uiMultilineEntry *me = uiNewNonWrappingMultilineEntry();
	uiControl *ctrl = uiControl(me);
#ifdef __APPLE__
	fix_darkmode(me);
#endif
	return control_handle_new(env, ctrl, "multilineEntry");
}

LIBUI_FUNCTION(createWrapping) {
	uiMultilineEntry *me = uiNewMultilineEntry();
	uiControl *ctrl = uiControl(me);
#ifdef __APPLE__
	fix_darkmode(me);
#endif
	return control_handle_new(env, ctrl, "multilineEntry");
}

LIBUI_FUNCTION(onChanged) {
	INIT_ARGS(2);

	ARG_POINTER(struct control_handle, handle, 0);
	ENSURE_NOT_DESTROYED();
	ARG_CB_REF(cb_ref, 1);

	struct event_t *event = create_event(env, cb_ref, "onChanged");
	if (event == NULL) {
		return NULL;
	}

	install_event(handle->events, event);

	uiMultilineEntryOnChanged(uiMultilineEntry(handle->control),
							  CALLBACK_OF(uiMultilineEntry, control_event_cb), event);

	return NULL;
}

LIBUI_FUNCTION(setText) {
	INIT_ARGS(2);
	ARG_POINTER(struct control_handle, handle, 0);
	ENSURE_NOT_DESTROYED();
	ARG_STRING(value, 1);
	uiMultilineEntrySetText(uiMultilineEntry(handle->control), value);
	free(value);
	return NULL;
}

LIBUI_FUNCTION(getText) {
	INIT_ARGS(1);
	ARG_POINTER(struct control_handle, handle, 0);
	ENSURE_NOT_DESTROYED();
	char *char_ptr = uiMultilineEntryText(uiMultilineEntry(handle->control));
	napi_value result = make_utf8_string(env, char_ptr);
	uiFreeText(char_ptr);
	return result;
}

LIBUI_FUNCTION(setReadOnly) {
	INIT_ARGS(2);
	ARG_POINTER(struct control_handle, handle, 0);
	ENSURE_NOT_DESTROYED();
	ARG_BOOL(value, 1);

	uiMultilineEntrySetReadOnly(uiMultilineEntry(handle->control), value);
	return NULL;
}

LIBUI_FUNCTION(getReadOnly) {
	INIT_ARGS(1);
	ARG_POINTER(struct control_handle, handle, 0);
	ENSURE_NOT_DESTROYED();
	bool value = uiMultilineEntryReadOnly(uiMultilineEntry(handle->control));
	return make_bool(env, value);
}

LIBUI_FUNCTION(append) {
	INIT_ARGS(2);
	ARG_POINTER(struct control_handle, handle, 0);
	ENSURE_NOT_DESTROYED();
	ARG_STRING(value, 1);
	uiMultilineEntryAppend(uiMultilineEntry(handle->control), value);
	free(value);
	return NULL;
}

napi_value _libui_init_multilineEntry(napi_env env, napi_value exports) {
	DEFINE_MODULE();
	LIBUI_EXPORT(create);
	LIBUI_EXPORT(createWrapping);
	LIBUI_EXPORT(getText);
	LIBUI_EXPORT(setText);
	LIBUI_EXPORT(getReadOnly);
	LIBUI_EXPORT(setReadOnly);
	LIBUI_EXPORT(append);
	LIBUI_EXPORT(onChanged);
	return module;
}
