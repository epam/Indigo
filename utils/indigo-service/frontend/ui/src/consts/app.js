import Search from '../pages/Search';
import Libraries from '../pages/Libraries';

export const app = {
    version: '__VERSION__',
    api_path: '__API_PATH__',
    libs: [],
    pages: [
        {
            url: '/search',
            component: Search,
            title: 'Search',
        },
        {
            url: '/libs',
            component: Libraries,
            title: 'Libraries',
        },
    ],
};
