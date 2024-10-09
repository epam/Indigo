import './App.css';
import AppRouter from './components/AppRouter';
import AppNavbar from "./components/navbar/AppNavbar";

const App = () => {
    return (
        <div className="app-wrapper">
            <AppNavbar/>
            <AppRouter/>
        </div>
    )
}

export default App
