var m = require('mithril');
var x = require('./x');
var e = require('./echo');

var libView = function (config) {
	if (vm.libs) {
		config.libs = vm.libs;
		vm.libs = null;
	}
	return [
		libraryView(config),
		!vm.log.length ? null : m('aside.log', [
			m('h2', 'Action log'),
			vm.log.slice(0, 10).map(function (out) {
				return m('li', out);
			})
		])
	];
};

var libraryView = function (config) {
	var libs = config.libs;
	return m('table.libs', {
		// onclick: function () { vm.edit.id = null; }
	},[
		m('thead', [
			m('tr', [
				m('th', 'Caption'),
				m('th.count', 'Size'),
				m('th.action', vm.state['new'] != 'edit' ? m('button.new', {
					title: 'Add new library',
					onclick: edit.bind(null, config.server, 'new')
				}, 'Create new') : null)
			]),
			vm.state['new'] == 'edit' ? m('tr[class="new"]',
			                              { key: 'new' }, editView()) : null
		]),
		m('tbody', libs.length ? libs.map(function (lib) {
			return m('tr', {
				key: lib.id,
				class: vm.state[lib.id] || ''
			}, x.choose(vm.state[lib.id], {
				edit: editView.bind(null, config, lib),
				delete: deleteView.bind(null, config, lib),
				false: baseView.bind(null, config, lib)
			}));
		}) : m('tr.empty', m('td[colspan=3]', 'No libraries'))) // empty to tbody
	]);
};

var baseView = function (config, lib) {
	return [
		m('td', [
			lib.name,
			m('small', lib.info() && lib.info().user_data.comment)
		]),
		m('td.count', lib.structures_count),
		m('td.action', x.choose(vm.state[lib.id], {
			upload: x.spinner,
			false: [
				m('label.upload', {
					title: 'Upload to library',
					onchange: upload.bind(null, config.server, lib)
				}, ['Upload', m('input[type=file]')]),
				m('button.edit', {
					title: 'Edit library',
					onclick: edit.bind(null, config.server, lib)
				}, 'Edit'),
				m('button.delete', {
					title: 'Delete library',
					onclick: remove.bind(null, config.server, lib)
				}, 'Delete')
			]
		}))
	];
};

var editView = function (config, lib) {
	return [
		m('td[colspan="2"]', [
			m('label', [
				'Name',
				m('input[type=text]', {
					value: vm.edit.name(),
					placeholder: 'Name',
					onchange: m.withAttr('value', vm.edit.name)
				})
			]),
			m('label', [
				'Comment',
				m('textarea', {
					value: vm.edit.comment() || '',
					placeholder: 'Comment',
					onchange: m.withAttr('value', vm.edit.comment)
				})
			])
		]), m('td.action', [
			m('input[type=reset][value="Cancel"]', {
				onclick: function() {
					vm.defmod.reject();
					return false;
				}
			}),
			m('input[type=submit]', {
				value: lib ? 'Update' : 'Create',
				onclick: function() {
					if (!lib || lib.name != vm.edit.name() ||
					    lib.info().user_data.comment != vm.edit.comment())
						vm.defmod.resolve(vm.edit);
					else
						vm.defmod.reject();
					return false;
				}
			})
		])
	];
};

var deleteView = function (config, lib) {
	return [
		m('td[colspan="2"]', m('em', [
			'Do you really want to delete ',
			m('strong', lib.name),
			' library?'
		])),
		m('td.action', [
			m('input[type=reset][value="Cancel"]', {
				onclick: function() {
					vm.defmod.reject();
					return false;
				}
			}),
			m('input[type=submit][value="Delete"]', {
				onclick: function() {
					vm.defmod.resolve(lib.id);
					return false;
				}
			})
		])
	];
};

var vm = {
	edit: {},

	defmod: null,   // single access
	state: {},

	log: []
};

function edit(server, lib, event) {   // prevents helpers
	if (vm.defmod)
		vm.defmod.reject();
	vm.defmod = m.deferred();

	var id = (lib == 'new') ? 'new' : lib.id;
	vm.state[id] = 'edit';

	vm.edit = {
		name: m.prop(lib != 'new' ? lib.name: ''),
		comment: m.prop(lib != 'new' ? lib.info().user_data.comment: '')
	};

	var confirm = vm.defmod.promise;
	var operate = confirm.then(function (nlib) {
		console.info('confirmed');
		vm.defmod = null;
		vm.state[id] = null;
		var req = lib == 'new' ? server.libNew : server.libUpdate.bind(server, { id: id });
		return req({
			name: nlib.name(),
			user_data: { comment: nlib.comment() }
		}).then(function log() {
			if (lib == 'new' || lib.name == nlib.name())
				actionLog([
					lib == 'new' ? 'Created ' : 'Updated ',
					m('em', nlib.name())
				]);
			else
				actionLog(['Renamed ', m('em', lib.name),
				           ' to ', m('em', nlib.name())]);
		});
	}, function () {
		console.info('declined');
		vm.defmod = null;
		vm.state[id] = null;
	});
	// TODO: updateList only if nessesary
	operate.then(updateList.bind(null, server));
	event.stopPropagation();
	return false;
}

function remove(server, lib, event) {
	console.assert(lib && lib.id, 'No lib to remove');
	if (vm.defmod)
		vm.defmod.reject();
	vm.defmod = m.deferred();
	vm.state[lib.id] = 'delete';

	var confirm = vm.defmod.promise;
	var operate = confirm.then(function (id) {
		console.info('confirmed');
		vm.defmod = null;
		vm.state[lib.id] = null;
		return server.libDelete({ id: lib.id }).then(function log() {
			actionLog(['Deleted ', m('em', lib.name)]);
		});
	}, function () {
		console.info('declined');
		vm.defmod = null;
		vm.state[lib.id] = null;
	});
	operate.then(updateList.bind(null, server));
	event.stopPropagation();
	return false;
}

// TODO: refactor to libs method
function updateList(server) {
	return server.libList().then(function (res) {
		res.forEach(function (lib) {
			lib.info = server.libInfo({ id: lib.id });
		});
		res.sort(function (a, b) {
			return b.created_timestamp - a.created_timestamp;
		});
		vm.libs = res;
	});
}

function knownMime(filename) {
	var mimeMap = {
		'chemical/x-mdl-sdfile': ['sdf', 'sd'],
		'application/gzip': ['gz', 'gzip']
	};
	var ext = filename.split('.').pop();
	for (var type in mimeMap) {
		if (mimeMap[type].indexOf(ext) != -1)
			return type;
	}
	return null;
}

function actionLog(data) {
	vm.log.unshift(typeof data == 'string' ? m.trust(data) : data);
}

function uploadLog(lib, res) {
	var prefix = res.state == 'INDEXING' ? 'insert' : 'index',
	    action = res.state == 'INDEXING' ? 'uploaded' : 'indexed',
	    stat = res.metadata;
	actionLog([
		'Library ', m('em', lib.name), ' was ', action, ' with ',
		m('var', stat.structures_count), ' structures in ',
		m('var', stat[prefix + '_time']), ' seconds (',
		m('var', stat[prefix + '_speed']), ' structures/sec).'
	]);
}

function upload(server, lib, event) {
	console.assert(lib && lib.id, 'No lib to upload to');
	if (vm.defmod)
		vm.defmod.reject();
	vm.state[lib.id] = 'upload';

	var file = event.target.files[0];
	var request = server.libUpload({ id: lib.id }, file, {
		headers: {
			'Content-Type': knownMime(file.name) || file.type || 'application/octet-stream'
		},
		background: true
	});
	var poll = request.then(function (res) {
		var oldstate = null;
		return x.pollDeferred(
			server.libUploadStatus.bind(server, {
				id: lib.id,
				upload: res.upload_id
			}, null, { background: true }),
			function complete(res) {
				if (res.state == 'FAILURE')
					throw res;
				if (res.metadata && res.state != oldstate) {
					oldstate = res.state;
					m.startComputation();
					uploadLog(lib, res);
					m.endComputation();
				}
				return res.state == 'SUCCESS';
			}, 500, 300);
	});
	poll.then(function(res) {
		console.info('upload completed');
		vm.state[lib.id] = null;
		return updateList(server);
	}, function (res) {
		m.startComputation();
		console.info('upload failed', res);
		e.alertMessage(JSON.stringify(res));
		vm.state[lib.id] = null;
		m.endComputation();
	});
	return false;
}

module.exports = libView;
