import 'react-app-polyfill/ie11'
import 'react-app-polyfill/stable'
import 'url-search-params-polyfill'
import './index.css'
import {BrowserRouter} from "react-router-dom"
import App from './App'
import {createRoot} from "react-dom/client";

const root = document.getElementById('root');
if (root !== null) {
  const reactRoot = createRoot(root);
  reactRoot.render(
    <BrowserRouter>
      <App/>
    </BrowserRouter>
  );
}

