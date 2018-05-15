const libui = require(`${__dirname}/ui.node`);
const async_hooks = require('@creditkarma/async-hooks');
const EventLoop = libui.EventLoop;
delete libui.EventLoop;

let wakingup = false;
let asyncHook = null;

class Point {
	constructor(x, y) {
		this.x = x;
		this.y = y;
	}
}

class Color {
	constructor(r, g, b, a) {
		this.r = r;
		this.g = g;
		this.b = b;
		this.a = a;
	}
}

class Size {
	constructor(w, h) {
		this.width = w;
		this.height = h;
	}
}

module.exports = libui;
const {UiCheckbox} = require('./js/checkbox');
const {
	UiArea,
	UiAreaMouseEvent,
	UiAreaKeyEvent,
	AreaDrawParams,
	AreaDrawContext,
	DrawBrush,
	UiDrawPath,
	DrawStrokeParams,
	UiDrawMatrix,
	AreaDrawBrushGradient,
	BrushGradientStop
} = require('./js/area');
const {UiButton} = require('./js/button');
const {UiWindow} = require('./js/window');
const {UiSlider} = require('./js/slider');
const {UiForm} = require('./js/form');
const {UiLabel} = require('./js/label');
const {UiVerticalBox} = require('./js/vertical-box');
const {UiHorizontalBox} = require('./js/horizontal-box');
const {UiMultilineEntry} = require('./js/multiline-entry');
const {UiColorButton} = require('./js/color-button');
const {UiCombobox} = require('./js/combobox');
const {UiEditableCombobox} = require('./js/editable-combobox');
const {UiTimePicker} = require('./js/time-picker');
const {UiDatePicker} = require('./js/date-picker');
const {UiEntry} = require('./js/entry');
const {UiSearchEntry} = require('./js/search-entry');
const {UiPasswordEntry} = require('./js/password-entry');
const {UiDateTimePicker} = require('./js/date-time-picker');
const {UiGroup} = require('./js/group');
const {UiSpinbox} = require('./js/spinbox');
const {UiProgressBar} = require('./js/progress-bar');
const {UiRadioButtons} = require('./js/radio-buttons');
const {UiHorizontalSeparator} = require('./js/horizontal-separator');
const {UiVerticalSeparator} = require('./js/vertical-separator');
const {UiTab} = require('./js/tab');
const {UiGrid} = require('./js/grid');
const {UiMenu, UiMenuItem} = require('./js/menu');

Object.assign(libui, {
	UiGrid,
	UiMenuItem,
	UiMenu,
	UiSpinbox,
	UiHorizontalSeparator,
	UiVerticalSeparator,
	UiRadioButtons,
	UiProgressBar,
	UiGroup,
	UiEntry,
	UiPasswordEntry,
	UiSearchEntry,
	UiEditableCombobox,
	UiTimePicker,
	UiDatePicker,
	UiDateTimePicker,
	UiCombobox,
	UiColorButton,
	UiCheckbox,
	UiWindow,
	UiButton,
	UiLabel,
	UiForm,
	UiSlider,
	UiMultilineEntry,
	UiHorizontalBox,
	UiVerticalBox,
	UiTab,
	UiArea,
	DrawBrush,
	BrushGradientStop,
	UiDrawPath,
	DrawStrokeParams,
	UiDrawMatrix,
	Point,
	Color,
	Point,
	UiAreaKeyEvent,
	UiAreaMouseEvent,
	fillMode: UiDrawPath.fillMode,
	brushType: DrawBrush.type,
	modifierKeys: UiAreaKeyEvent.modifierKeys,
	extKeys: UiAreaKeyEvent.extKeys
});

libui.App.init();

libui.Area.init(UiAreaMouseEvent, UiAreaKeyEvent, AreaDrawParams, AreaDrawContext,
				BrushGradientStop, Color, Point, Size);

libui.onShouldQuit = libui.App.onShouldQuit;

libui.startLoop = () => {
	asyncHook = async_hooks.createHook({init: initAsyncResource});

	// Allow callbacks of this AsyncHook instance to call. This is not an
	// implicit action after running the constructor, and must be explicitly run
	// to begin executing callbacks.
	asyncHook.enable();

	EventLoop.start();
};

libui.stopLoop = () => {
	// Allow callbacks of this AsyncHook instance to call. This is not an
	// implicit action after running the constructor, and must be explicitly run
	// to begin executing callbacks.
	asyncHook.disable();
	asyncHook = null;

	EventLoop.stop();
};

// This is called when a new async handle
// is created. It's used to signal the background
// thread to stop awaiting calls and upgrade it's list of handles
// it's awaiting for.
function initAsyncResource(asyncId, type, triggerAsyncId, resource) {
	if (wakingup) {
		return;
	}
	wakingup = true;
	setImmediate(() => {
		EventLoop.wakeupBackgroundThread();
		wakingup = false;
	});
}
