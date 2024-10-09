import AppLink from './AppLink';
import './AppNavbar.scss';
import React, {useEffect, useState} from "react";
import {Endpoint_Types, Endpoint_Values} from "../search-type/constants/types";


const AppNavbar = () => {
    const title = 'Bingo Search';
    return (
        <header>
            <nav>
                <div>
                    <h1>{title}</h1>
                </div>
            </nav>
        </header>
    );
};

export default AppNavbar;
