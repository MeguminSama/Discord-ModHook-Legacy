const { app } = require("electron");

const _setPath = app.setPath;

app.setPath = function (name, path) {
	if (name === "userData") {
		_setPath.call(app, name, app.getPath("appData") + "\\MeguCord");
	} else {
		_setPath.call(app, name, path);
	}
};

app.setPath("userData", app.getPath("appData") + "\\MeguCord");

require("C:\\Users\\megu\\Workspace\\Discord\\Vencord\\dist\\patcher.js");
