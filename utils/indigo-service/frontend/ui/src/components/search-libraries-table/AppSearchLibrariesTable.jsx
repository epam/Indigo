import React, {useEffect, useState} from 'react';
import {useFetch} from '../../hooks/useFetch';
import ApiService from '../../api/ApiService';
import './AppSearchLibrariesTable.scss';

const AppSearchLibrariesTable = ({updateLibIds}) => {
  const [libraries, setLibraries] = useState([]);
  const [checkStates, setCheckStates] = useState([]);

  const [getLibraries] = useFetch(() => {
    async function callMe() {
      const {data: libraries} = await ApiService.getLibraries();
      setLibraries(libraries);
      setCheckStates(new Array(libraries.length).fill(false));
    }

    callMe();
  });

  const handleCheckState = (i) => {
    const updatedCheckState = checkStates.map((item, index) =>
      index === i ? !item : item
    );

    setCheckStates(updatedCheckState);
  };

  useEffect(() => {
    updateLibIds(
      libraries.filter((_, id) => checkStates[id]).map((lib) => lib.id)
    );
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [checkStates]);

  useEffect(() => {
    getLibraries();
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

  return (
    <>
      <fieldset className="libraries-fieldset">
        <legend>Source dataset:</legend>
        <div>
          {libraries.map(({name, structures_count}, i) => (
            <div className="checkbox-wrapper">
              <input
                type="checkbox"
                className="app-checkbox"
                checked={checkStates[i] || false}
                onChange={() => handleCheckState(i)}
              />
              <div className="checkbox-title">{ name }</div>
            </div>
          ))}

        </div>
      </fieldset>
    </>
  );
};

export default AppSearchLibrariesTable;
