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

  const checkedLen = () => checkStates.filter(Boolean).length;

  const handleCheckAllState = (state) => {
    setCheckStates(new Array(libraries.length).fill(state));
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
      <fieldset>
        <legend>Libraries</legend>
        <table className="libs">
          <thead>
          <tr>
            <th className="check">
              <input
                className={
                  checkedLen() < libraries.length ? 'intermediate' : ''
                }
                type="checkbox"
                checked={checkedLen() > 0}
                onChange={(event) =>
                  handleCheckAllState(event.target.checked)
                }
              />
            </th>
            <th className="lib-name">Name</th>
            <th className="count">Size</th>
          </tr>
          </thead>
          <tbody>
          {libraries.map(({name, structures_count}, i) => (
            <tr key={i}>
              <td className="check">
                <input
                  type="checkbox"
                  checked={checkStates[i] || false}
                  onChange={() => handleCheckState(i)}
                />
              </td>
              <td className="lib-name">{name}</td>
              <td className="count">{structures_count}</td>
            </tr>
          ))}
          </tbody>
        </table>
      </fieldset>
    </>
  );
};

export default AppSearchLibrariesTable;
