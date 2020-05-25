var m = require('mithril'),
    CodeMirror = require('codemirror');

function view(ctrl, opts, _) {
	return m('textarea[name=props]', Object.assign(opts, {
		properties: null, // don't pollute to textarea
		config: cmConfig.bind(null, opts.properties)
	}));
}

function cmConfig(props, el, isInit, config) {
	if (!isInit) {
		var cm = CodeMirror.fromTextArea(el),
		    update = function() {
			    cm.save();
			    el.onchange({ currentTarget: el });
			    console.info('update', el.value);
		    },
		    hint = function() {
			    cm.showHint({
				    hint: propHint.bind(null, config.properties),
				    //completeOnSingleClick: true,
				    completeSingle: false
			    });
		    },
		    debounce_hint = function() { // underscore btw
			    if (config.timeout) clearTimeout(config.timeout);
			    config.timeout = setTimeout(hint, 250);
		    };
		cm.on('change', function () {
			var v1 = el.value.trim(),
			    v2 = cm.getValue().trim();
			if (!!v1.length != !!v2.length)
				update();
		});
		cm.on('inputRead', debounce_hint);
		cm.on('keyup', function(cm, event) {
			var keyCode = event.keyCode || event.which;
			// TODO: put this to keymap
			if(keyCode == 8 || keyCode == 46) // del or backspace
				debounce_hint();
		});
		cm.on('focus', hint);
		cm.on('blur', function (event) {
			//console.info('blur', arguments, cm.state.completionActive);
			//if (!cm.state.completionActive)
			update();
		});
	}
	config.properties = props.map(function (prop) {
		var notExcaped = /^[a-z0-9_]+$/i;
		return {
			text: /*prop.match(notExcaped) ? prop :*/ '"' + prop + '"',
			displayText: prop
		};
	});
}

function ctrl(opts, _) {
}

function propHint(props, cm) {
	// var cur = cm.getCursor(), token = cm.getTokenAt(cur);
    // var to = CodeMirror.Pos(cur.line, token.end);
    // if (token.string && /\w/.test(token.string[token.string.length - 1])) {
    //   var term = token.string, from = CodeMirror.Pos(cur.line, token.start);
    // } else {
    //   var term = "", from = to;
	// }
    var cur = cm.getCursor(), curLine = cm.getLine(cur.line);
    var end = cur.ch, start = end;
	while (start && /[\w\d_$]+/.test(curLine.charAt(start - 1))) --start;
	var found = [];

	if (start == end)
		found = props;
	else {
		var term = RegExp(curLine.slice(start, end), 'i');
		for (var i = 0; i < props.length; i++) {
			var prop = props[i],
			    text = prop.displayText;
			if (text.search(term) != -1 && prop != term)
				found.push(prop);
		}
	}
	return !found.length ? null : {
		list: found.slice(0, 20),
		from: CodeMirror.Pos(cur.line, start),
		to: CodeMirror.Pos(cur.line, end)
	};
}

Object.assign(ctrl.prototype, {
	config: cmConfig,
	propHint: propHint
});

module.exports = m.component.bind(m, {
	view: view,
	controller: ctrl
});
