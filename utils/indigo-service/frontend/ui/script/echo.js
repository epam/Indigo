var m = require('mithril');

// Wrapper for client messages

function alertMessage(mes) {
	//Right now ugly alert
	//TODO: change to something user-friendly
	alert(mes);
}

module.exports = {
	alertMessage:alertMessage
};
