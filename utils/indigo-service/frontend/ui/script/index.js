var m = require('mithril');
var qs = require('query-string');

var searchView = require('./search');
var libsView = require('./libs');
var api = require('./api');

var app = {
  version: '__VERSION__',
  api_path: '__API_PATH__',
  libs: [],
  pages: [{
    url: '/search',
    view: searchView,
    title: 'Search'
  }, {
    url: '/libs',
    view: libsView,
    title: 'Libraries'
  }]
};

app.view = function (page) {
  console.info('redraw', page.url.slice(1));
  return [
    m('nav', [
      m('h1', 'Indigo Online'),
      m('ul', app.pages.map(function (pg) {
        return m('li', {'class': page.url == pg.url ? 'active' : ''},
          m('a', {href: pg.url, config: m.route}, pg.title));
      }))
    ]),
    m('main', {'class': page.url.slice(1)}, [
      m('iframe', {
        src: '/ketcher/?api_path=/v2',
        onload: function () {
          app.ketcher = this.contentWindow.ketcher;
        }
      }),
      page.view(app)
    ])
  ];
};

//initialize
window.onload = function () {
  //document.title += ' v' + app.version;
  var opts = qs.parse(location.search);
  app.api_path = opts.api_path || app.api_path;
  app.server = api(app.api_path);

  app.server.libList().then(function (res) {
    res.forEach(function (lib) {
      lib.info = app.server.libInfo({id: lib.id});
    });
    res.sort(function (a, b) {
      return b.created_timestamp - a.created_timestamp;
    });
    app.libs = res;
  });

  m.route.mode = "hash";
  m.route(document.body, '/search', app.pages.reduce(function (res, page) {
    res[page.url] = {
      view: app.view.bind(app, page),
      controller: function () {
        m.redraw.strategy('diff');
      }
    };
    return res;
  }, {}));
};
