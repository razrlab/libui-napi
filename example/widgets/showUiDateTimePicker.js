/* eslint-disable unicorn/filename-case */

const libui = require('../..');

const win = new libui.UiWindow('UiDateTimePicker example', 320, 60, true);
win.margined = true;

const widget = new libui.UiDateTimePicker();
win.setChild(widget);

win.onClosing(() => {
	win.close();
	libui.stopLoop();
});

win.show();

libui.startLoop();
