var m = require('mithril'),
    murmurhash = require('murmurhash');

var choose = function (key, options) {
	var opt = options[key] || options[false];
	return opt ? (opt.call ? opt() : opt) : null;
};

var parseFloatStrict = function (value) {
    if(/^(\-|\+)?([0-9]+(\.[0-9]+)?|Infinity)$/
      .test(value))
      return Number(value);
  return NaN;
};

var numProp = function(init) {
	var store = parseFloatStrict(init);
	var res = function(val) {
		if (arguments.length)
			store = parseFloatStrict(val);
		return store;
	};
	res.toJSON = function() {
		return store === +store ? store : undefined;
	};
	return res;
};

var boolProps = function (init) {
	function apply(props) {
		return (props || []).reduce(function (store, prop) {
			store[prop] = true;
			return store;
		}, {});
	}
	function single(id, val) {
		if (val !== undefined) {
			if (!!val)
				this[id] = true; // the string here
			else
				delete this[id];
		}
		return this[id] || false;
	}
	function res(val) {
		if (arguments.length && !Array.isArray(val))
			return single.bind(res.store, val);
		if (val)
			res.store = apply(val);
		return Object.keys(res.store);
	}
	res.store = apply(init);
	res.toJSON = function () {
		return Object.keys(res.store);
	};
	return res;
};

var mapFactory = function() {
	var map = {},
      next = 1;

	var viewModel = function(signature) {
		return function(key) {
			key = key.key || key;
			if (!map[key]) {
				map[key] = {};
				for (var prop in signature)
					map[key][prop] = m.prop(signature[prop]());
			}
			return map[key];
		};
	};
	var holdKey = function() {
		return next++;
	};
	var releaseKey = function(key) {
		key = key.key || key;
		delete map[key];
	};
	return {
		holdKey: holdKey,
		releaseKey: releaseKey,
		viewModel: viewModel
	};
};

var pollDeferred = function(process, complete, timeGap, startTimeGap) {
	var res = m.deferred();
	function iterate() {
		process().then(function (val) {
			try {
				var finish = complete(val);
				if (finish)
					res.resolve(val);
				else
					window.setTimeout(iterate, timeGap);
			}
			catch (e) {
				res.reject(e);
			}
		}, function (err) {
			return res.reject(err);
		});
	}
	window.setTimeout(iterate, startTimeGap || 0);
	return res.promise;
};


var memorizeLast = function(fn) {
	// designed to be light and to not copy much
	// instead to be universal solution
	return function() {
		var args = [].slice.call(arguments),
		    cached = fn.last_args && args.length == fn.last_args.length &&
			    fn.last_args.every(function (val, i) {
				    return val === args[i];
			    });
		if (!cached) {
			fn.last_args = args;
			fn.last_res = fn.apply(this, fn.last_args);
		}
		return fn.last_res;
	};
};

function pollCache(size) {
	var hash = {},
	    poll = [];
	return function (key, value) {
		if (arguments.length > 1) {
			if (!hash.hasOwnProperty(key)) {
				poll.unshift(key);
				if (poll.length > size) {
					var dk = poll.pop();
					delete hash[dk];
				}
			}
			hash[key] = value;
		}
		return hash[key];
	};
}

function hash() {
	var str = [].filter.call(arguments,
	                         function (v) { return !!v; }).join('\0');
	return murmurhash.v3(str);
}

function spinner(className) {
	return m('.loading', { className: className }, 'Loading..');
}

module.exports = {
	spinner: spinner,
	choose: choose,
	boolProps: boolProps,
	mapFactory: mapFactory,
	pollDeferred: pollDeferred,
	memorizeLast: memorizeLast,
	pollCache: pollCache,
	numProp: numProp,
	hash: hash
};
