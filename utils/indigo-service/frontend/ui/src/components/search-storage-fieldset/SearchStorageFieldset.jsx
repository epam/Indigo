import './SearchStorageFieldset.scss';
import { Endpoint_Types, Endpoint_Values } from '../search-type/constants/types';
import React, { useEffect, useState } from 'react';
import AppSelect from '../select/AppSelect';

const SearchStorageFieldset = ({updateType, params}) => {
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

    return (
        <fieldset>
            <legend>Storage:</legend>
            <div className='endpoint_toggle'>
              <AppSelect
                title="Database type"
                value={endpoint}
                options={Endpoint_Types}
                onChange={(value) => handleEndpointChange(value)}
              />
            </div>
        </fieldset>
    );
};

export default SearchStorageFieldset;
