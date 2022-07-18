var m = require('mithril'),
    x = require('./x');
var e = require('./echo');

// TODO: ctrl.mode to results vm
function outputView(opts) {
	if (!opts.result().mode)
		searchShift(opts);
	else
		console.info('Results count: ', opts.result().list.length, 'mode:', opts.result().mode);

	return m('section.output', [
		queryView(opts),
		opts.result().mode == 'empty' ?
			m('.results', m('strong', 'No results')) : [
				totalView(opts),
				m('ul.results', { config: outputConfig.bind(opts) },
				  opts.result().list.map(resultView.bind(null, opts))),
			],
		opts.result().mode == 'search' ? x.spinner() : null
	]);
}

var outputConfig = function(el, isInit, ctx) {
	console.info('config start', isInit, ctx);
	if (!ctx.onunload) {
		var checkLast = function() {
			var lastCh = el.lastElementChild;
			if (this.result().mode == 'pass' && lastCh &&
			    window.innerHeight > lastCh.getBoundingClientRect().top)
				searchShift();
		}.bind(this);
		console.info('set listener', ctx);
		window.addEventListener('scroll', checkLast);

		ctx.onunload = function() {
			console.info('unset listener', ctx);
			window.removeEventListener('scroll', checkLast);
		};
	}
};

function totalView(opts) {
	var rs = opts.result();
	return m('.summary', [
		rs.count ? m('p', ['Total results:', m('var', rs.count)]) : null,
		m('a.export', { href: opts.server.sdfExport.url({ search_id: rs.id }) }, 'Export')
	]);
};

function queryView(opts) {
	var structure = opts.request.query_structure,
	    text = opts.request.query_text;
	return m('header.query', [
		structure ? imgView(opts.server, { query: structure }) : null,
		text && structure ? m('em', 'and') : null,
		text ? m('code', text) : null
	]);
}

function resultView(opts, result) {
	var req = opts.request,
	    qs = (req.type == 'sub') ? req.query_structure : null,
	    libs = libsMap(opts.libs);

	var propKeys = Object.keys(result.properties),
	    toLongList = propKeys.length > 25;

	return m('li', { key: [result.library_id, result.id].join('\0') }, [
		m('h3', libs[result.library_id], ' #', m('var', result.id)),
		imgView(opts.server, {
			struct: result.structure,
			query: qs
		}),
		' ',
		m('dl', { 'class': toLongList && !result.all ? 'short' : '' },
		  propKeys.map(function (prop) {
			  return [
				  m('dt', {
					  'class': result.found_properties_keys && result.found_properties_keys.indexOf(prop) != -1 ?
						  'highlight' : ''
				  }, prop),
				  m('dd', result.properties[prop])
			  ];
		  })),
		  toLongList ? m('button', {
			  onclick: function() { result.all = !result.all; }
		  }, !result.all ? 'More..' : 'Less..') : null
	]);
}

function imgView(server, opts) {
	var key = x.hash(opts.query, opts.struct),
	    src = imgView.cache(key);
	if (!src) {
		src = server.render(opts);
		src.then(function(data) {
			return imgView.cache(key,
                             'data:image/svg+xml;charset=utf-8,' + encodeURIComponent(data));
		});
		imgView.cache(key, src);
	}
	return src.then ? x.spinner : m('img', { src: src });
}
imgView.cache = x.pollCache(1000);

var libsMap = x.memorizeLast(function (libs) {
	console.info('libsMap: no memorize');
	return libs.reduce(function (res, lib) {
		res[lib.id] = lib.name;
		return res;
	}, {});
});

var BULK_SIZE = 20;
var searchOpts = null;
var searchShift = function (options) {
	if (options) {
		searchOpts = options;
		searchOpts.result({ mode: 'search', list: []});
	}
	var rs = searchOpts.result();
	var request = Object.assign({
		limit: BULK_SIZE,
		offset: rs.list.length
	}, searchOpts.request);
	var search = searchOpts.server.search(request, { background: true });
	var apply = search.then(function (data) {
		var res = data.result;
		if (!rs.id)
			rs.id = data.search_id;
		console.info('Fetched ', res.length, 'results, id: ', data.search_id);
		if (res.length === 0)
			rs.mode = rs.list.length ? 'end' : 'empty';
		else {
			[].push.apply(rs.list, res);
			rs.mode = rs.list.length % BULK_SIZE == 0 ? 'pass' : 'end';
		}
		return rs;
	}, function (err) {
		console.error(err);
		//e.alertMessage(JSON.stringify(err));
		// Change mode to empty
		rs.mode = 'empty';
		//rs.mode = 'error';
	});
	apply.then(m.redraw);
	if (options) {
		var count = apply.then(function () {
			if(!rs.id) {
				rs.count = 0;
				rs.mode = 'empty';
				m.redraw();
				return null;
			}
			return x.pollDeferred(
				searchOpts.server.searchCount.bind(searchOpts.server,
				                                   { search_id: rs.id }, null,
				                                   { background: true }),
				function complete(res) {
					if (res.state == 'FAILURE')
						throw res;
					return res.state == 'SUCCESS';
				}, 500, 300);
		});
		count.then(function (res) {
			if(res && res.result) {
				rs.count = res.result.count;
				m.redraw();
				console.info('Total results:', rs.count);
			}
		});
	}
};


module.exports = outputView;
