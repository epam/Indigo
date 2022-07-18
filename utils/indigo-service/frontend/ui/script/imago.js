var m = require('mithril');
var x = require('./x');
var e = require('./echo');

var imago_result = {};

var imagoView = function (config) {
	return [
		m('h1', 'Imago'),
		m('h3', 'Please choose PNG or JPG file to Recognize'),
		m('label.upload', {
			title: 'Upload image',
			onchange: upload.bind(null, config.server, config)
			}, m('input[type=file]')),
		imago_result.img_data == null ?
		m('p', 'No Results') :
		m('table', [
			m('thead', m('tr', [
				m('th', 'Loaded Image'),
				m('th', 'Recognized Image')])),
			m('tbody',
				m('tr', [
					m('td', m('img', { src: imago_result.img_data})),
					m('td', imago_result.mol_img == null ? 
						m('p','Recognizing...') : 
						[
							m('img', { src: imago_result.mol_img}),
							m('p', m('button.transfer', {onclick: imago_result.transfer_cb} , 'Transfer to Ketcher'))
						]
					)
				])
			)
		])
	];
};

function upload(server, app, event) {
	var file = event.target.files[0];

	var request = server.imagoUpload(file, {
		headers: {
			'Content-Type': file.type
		},
		background: true
	});
	imago_result.img_data = URL.createObjectURL(file);
	var poll = request.then(function (res) {
		m.redraw('imago');
		return x.pollDeferred(
			server.imagoUploadStatus.bind(server, {
				upload: res.upload_id
			}, null, { background: true }),
			function complete(res) {
				if (res.state === 'FAILURE')
					throw res;
				return res.state === "SUCCESS";
			}, 500, 300);
	});
	poll.then(function(res) {
		console.info('upload completed');
		if (res.state === 'SUCCESS') {
			console.info('SUCCESS');
			imago_result.mol_str = res.metadata.mol_str;
			res = server.render(
				{struct: imago_result.mol_str, output_format: "image/svg+xml"}, {
				headers: { 'Content-Type': 'application/json'}
			});
			res.then(function(data) {
				// console.info(data);
				imago_result.mol_img='data:image/svg+xml;charset=utf-8,' + encodeURIComponent(data);
			});
			
			imago_result.transfer_cb = function() { 
				app.ketcher.setMolecule(imago_result.mol_str); 
				m.route('search'); 
			};
			m.redraw('imago');
		}
		return true;
	}, function (res) {
		console.info('upload failed', res);
		e.alertMessage(JSON.stringify(res));
	});
	return false;
}
module.exports = imagoView;
