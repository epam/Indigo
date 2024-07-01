import {Navigate, Route, Routes} from 'react-router-dom';
import NotFound from '../pages/NotFound';
import {app} from '../consts/app';

const AppRouter = () => {
    return (
        <main>
            <Routes>
                {app.pages.map((page) => (
                    <Route path={page.url} element={<page.component/>} key={page.url}/>
                ))}

                <Route path="/" element={<Navigate to="/search"/>}/>
                <Route path="*" element={<NotFound/>}/>
            </Routes>
        </main>
    );
};

export default AppRouter;
