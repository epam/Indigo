var m = require('mithril');

function api(base) {
	var apiPath = !base || /\/$/.test(base) ? base : base + '/';

	var format_url = function(string, params) {
		return string.replace(/:(\w+)/g, function(_, val) {
			return params[val];
		});
	};

	var ajax = function(options, callback) {
		var xhr = new XMLHttpRequest();
		var headers = options.headers || {};

		xhr.open(options.method, options.url,
		         !!callback, options.user, options.password);

		for (var k in headers) {
			if (headers.hasOwnProperty(k)) {
				xhr.setRequestHeader(k, headers[k]);
			}
		}
		if (typeof options.config === 'function') {
			var maybeXhr = options.config(xhr, options);
			if (maybeXhr !== undefined) {
				xhr = maybeXhr;
			}
		}
		if (options.timeout > 0) {
			setTimeout(function () {
				xhr.status = -1;
				xhr.abort();
			}, options.timeout);
		}
		if (callback) {
			xhr.onreadystatechange = function () {
				if (xhr.readyState === 4) {
					callback(xhr);
				}
			};
		}
		xhr.send(options.data);
		return xhr;
	};

	function raw_request(options) {
		var deferred = m.deferred();
		ajax(options, function (xhr) {
			var data = (options.deserialize || JSON.parse)(xhr.responseText);
			if (xhr.status >= 200 && xhr.status < 300)
				deferred.resolve(data);
			else
				deferred.reject(data);
		});
		return deferred.promise;
	}

	var request = function(options) {
		return (!options.data || options.data.constructor == Object) ?
			   m.request(options) : raw_request(options);
	};

	var apiCall = function(method, path, defaultOptions) {
		var hasParams = path.search(/:\w+/) != -1;
		var req = function(params, data, options) {
			if (!hasParams) {
				options = data;
				data = params;
			}
			return request(Object.assign({
				method: method,
				url: req.url(params),
				data: data
			}, defaultOptions, options));
		};
		req.url = function(params) {
			return apiPath + (hasParams ? format_url(path, params) : path);
		};
		return req;
	};

	return {
		libList: apiCall('GET', 'libraries/libraries'),
		libNew: apiCall('POST', 'libraries/libraries'),
		libInfo: apiCall('GET', 'libraries/libraries/:id'),
		libUpdate: apiCall('PUT', 'libraries/libraries/:id'),
		libDelete: apiCall('DELETE', 'libraries/libraries/:id'),
		libUpload: apiCall('POST', 'libraries/libraries/:id/uploads'),
		libUploadStatus: apiCall('GET', 'libraries/libraries/:id/uploads/:upload'),
		search: apiCall('POST', 'libraries/search'),
		searchCount: apiCall('GET', 'libraries/search/:search_id'),
		sdfExport: apiCall('GET', 'libraries/search/:search_id.sdf'),
		render: apiCall('POST', 'indigo/render', {
			deserialize: function(data) { return data; }
		})
	};
}

module.exports = api;
