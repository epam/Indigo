var m = require('mithril'),
    x = require('./x'),
    outputView = require('./output'),
    propedit = require('./propedit');

var request = {
	type: m.prop('sub'),
	options: m.prop(''),
	metric: m.prop('tanimoto'),
	min: x.numProp(),
	max: x.numProp(),
	query_text: m.prop(''),
	library_ids: x.boolProps([])
};
var query = null;
var result = m.prop();

var types = {
	'sub': "Substructure",
	'exact': "Exact",
	'sim': "Similarity"
//	'molFormula': "Molecular formula"
};

var simMetrics = {
	'tanimoto': "Tanimoto",
	'tversky': "Tversky",
	'euclid-sub': "Euclid-sub"
};

var searchView = function(config) {
	return [
		m('form.request', [
			searchTypeView(),
			librariesView(config.libs),
			m('label[class="props"]', [
				'Properties',
				propedit({
					placeholder: 'e.g. mass > 300',
					properties: propsAllowed(config.libs, request.library_ids()),
					value: request.query_text(),
					onchange: m.withAttr('value', request.query_text)
				})
			]),
			m("input[type=submit][value='Search']", {
				disabled: request.library_ids().length == 0,
				onclick: submit.bind(null, config.ketcher)
			})
		]),
		!result() ? null : outputView({
			request: query,
			result: result,
			server: config.server,
			libs: config.libs
		})
	];
};

var searchTypeView = function() {
	return m('fieldset', [
		m('legend', 'Search Type'),
		selectView({ options: types }, request.type),
		request.type() == 'sim' ? m('fieldset.options', [
			selectView({ options: simMetrics }, request.metric),
			m("input[name='min'][placeholder='Min'][size='3']" +
			  "[type='number'][min='0'][max='1'][step='0.1']", {
				value: request.min(),
				onchange: m.withAttr('value', request.min)
			}),
			m("input[name='max'][placeholder='Max'][size='3']" +
			  "[type='number'][min='0'][max='1'][step='0.1']", {
				value: request.max(),
				onchange: m.withAttr('value', request.max)
			}),
		]) :
		m("textarea[cols='20'][placeholder='Options'][rows='1']", {
			value: request.options(),
			onchange: m.withAttr('value', request.options)
		})
	]);
};

var checkAllView = function (libs) {
	var checkedLen = request.library_ids().length;
	return m('input[type=checkbox]', {
		class: checkedLen && checkedLen < libs.length ? 'intermediate' : '',
		checked: checkedLen > 0,
		onchange: function() {
			request.library_ids(!this.checked ? [] : libs.map(function (lib) {
				return lib.id;
			}));
		}
	});
};

var librariesView = function (libs) {
	return m('fieldset',
	         m('legend', 'Libraries'),
	         m('table.libs', [
		         m('thead', m('tr', [
			         m('th.check', libs.length ? checkAllView(libs) : null),
			         m('th.libname', 'Name'), m('th.count', 'Size')
		         ])),
		         m('tbody',
		           libs.length ? libs.map(function (lib) {
			           return m('tr', [
				           m('td.check', m('input[type=checkbox]', {
					           checked: request.library_ids(lib.id)(),
					           onchange: m.withAttr('checked', request.library_ids(lib.id))
				           })),
				           m('td.libname', lib.name),
				           m('td.count', lib.structures_count)
			           ]);
		           }) :
		           m('tr.empty', m('td[colspan=3]', 'No libraries'))
		          )
	         ]) 
	        );

};

var propsAllowed = x.memorizeLast(function(libs, libs_id) {
	console.info('propsAllowed: no memorize');
	var res = libs.reduce(function (res, lib) {
		if (libs_id.indexOf(lib.id) != -1 ) {
      	var props = lib.info().service_data.properties;
			if(props && props.forEach) {
				props.forEach(function (prop) {
					if (res.indexOf(prop) == -1)
						res.push(prop);
				});
			}
		}
		return res;
	}, []);
	return res;
});

// nested prop
var selectView = function(attrs, prop) {
	var name = (attrs.label || '').toLowerCase();
	var sel = m('select', {
		name: name,
		value: prop(),
		onchange: m.withAttr('value', prop)
	}, Object.keys(attrs.options).map(function (val) {
		return m('option', { value: val }, attrs.options[val]);
	}));
	return !attrs.label ? sel :
		m('label', { 'class': name }, [attrs.label, sel]);
};

function submit(ketcher, event) {
	var molfile = ketcher.getMolfile(),
	    isEmpty = molfile.split('\n').length <= 6;
//     var molfile = "";
//     ketcher.getMolfile().then(m => { 
//         molfile = m; 
//     });
//     var isEmpty = molfile.split('\n').length <= 6;
	query = JSON.parse(JSON.stringify(request)); // poor man's clone
	query.query_structure = !isEmpty ? molfile : undefined;
	result('search');
	event.preventDefault();
	return false;
}

module.exports = searchView;
