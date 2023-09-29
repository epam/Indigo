import AppLink from './AppLink';
import './AppNavbar.scss';
import React, {useEffect, useState} from "react";
import {Endpoint_Types, Endpoint_Values} from "../search-type/constants/types";


const AppNavbar = () => {
    const [endpoint, setEndpoint] = useState(() => {
        const storedValue = localStorage.getItem("endpoint");
        if (storedValue) {
            return storedValue
        } else {
            localStorage.setItem("endpoint", Endpoint_Values.Postgres)
            return Endpoint_Values.Postgres
        }
    });

    useEffect(() => {
        localStorage.setItem("endpoint", endpoint);
    }, [endpoint]);

    function handleEndpointChange(value) {
        setEndpoint(value);
        window.location.reload()
    }

    const title = 'Indigo Online';
    return (
        <header>
            <nav>
                <div>
                    <h1>{title}</h1>
                </div>
                <div>
                    <AppLink to="/search">Search</AppLink>
                </div>
                <div className='endpoint_toggle'>
                    <select value={endpoint} onChange={(event) => handleEndpointChange(event.target.value)}>
                        {Endpoint_Types.map(({name, value}) => (
                            <option value={value} key={value}>
                                {name}
                            </option>
                        ))}
                    </select>
                </div>
            </nav>
        </header>
    );
};

export default AppNavbar;
