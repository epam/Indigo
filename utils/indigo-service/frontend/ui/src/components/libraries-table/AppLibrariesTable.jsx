import React, {useEffect, useState} from 'react';
import {useFetch} from '../../hooks/useFetch';
import ApiService from '../../api/ApiService';
import './AppLibrariesTable.scss';
import {CircleLoader} from "react-spinners";
import {Endpoint_Values} from "../search-type/constants/types";

const AppLibrariesTable = () => {
  const [isActive, setIsActive] = useState(false);
  const [libraries, setLibraries] = useState([]);
  const [name, setName] = useState('');
  const [id, setId] = useState('');
  const [comment, setComment] = useState('');
  const [isLoading, setIsLoading] = useState(false);

  function checkPassword() {
    const enteredPassword = window.prompt('Enter password:');
    if (enteredPassword !== process.env.REACT_APP_LIBS_PASSWORD) {
      window.alert('Invalid password.');
      window.location.href = '/';
    }
  }


  useEffect(() => {
    checkPassword();
    getLibraries();
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);


  const handleClick = () => {
    setIsActive((current) => !current);
  };

  const [getLibrary] = useFetch(async (rawLibs) => {
    Promise.all(rawLibs.map((lib) => ApiService.getLibraryById(lib.id))).then(
      (libs) => {
        setLibraries(
          rawLibs.map((rawLib, i) => Object.assign({}, rawLib, libs[i].data))
        );
      }
    );
  });

  const [getLibraries] = useFetch(() => {
    async function callMe() {
      await ApiService.getLibraries().then(({data: libraries}) =>
        getLibrary(libraries)
      );
    }

    callMe();
  });

  const [addLibrary] = useFetch(() => {
    async function callMe() {
      const data = {name, user_data: {comment}};
      await ApiService.addLibrary(data).then(() => getLibraries());
      handleClick();
      setName('');
      setComment('');
    }

    callMe()
  });

  const [uploadToLibrary] = useFetch((event) => {
    async function callMe() {
      setIsLoading(true)
      await ApiService.uploadToLibrary(event.target.files[0], id).then(() => {
          if (localStorage.getItem("endpoint") === Endpoint_Values.Elastic) {
            ApiService.recount(id)
          }
          setId('')
          getLibraries()
          setIsLoading(false)
        }
      );
    }

    callMe();
  })

  const [deleteLibrary] = useFetch((id) => {
    async function callMe() {
      await ApiService.deleteLibrary(id).then(() => getLibraries());
      setId('')
    }

    callMe();
  })

  return (
    <main className="libs-page">
      {isLoading && (<div className="spinner"><CircleLoader color={"#123abc"} loading={true} size={150}/></div>)}

      <table className="libs">
        <thead>
        <tr>
          <th>Caption</th>
          <th className="count">Size</th>
          <th className="action">
            <button
              title="Add new library"
              className={isActive ? 'hidden new' : 'new'}
              onClick={handleClick}
            >
              Create new
            </button>
          </th>
        </tr>
        <tr className={isActive ? 'new' : 'hidden new'}>
          <td colSpan="2">
            <label>
              Name
              <input
                type="text"
                placeholder="Name"
                value={name}
                onChange={(event) => setName(event.target.value)}
              />
            </label>
            <label>
              Comment
              <textarea
                placeholder="Comment"
                value={comment}
                onChange={(event) => setComment(event.target.value)}
              ></textarea>
            </label>
          </td>
          <td className="action">
            <input type="reset" value="Cancel" onClick={handleClick}/>
            <input type="submit" value="Create" onClick={addLibrary}/>
          </td>
        </tr>
        </thead>
        <tbody>
        {libraries.map(({id, name, structures_count, user_data}, i) => (
          <tr key={i}>
            <td>
              {name}
              <small>{user_data.comment}</small>
            </td>
            <td className="count">{structures_count}</td>
            <td className="action">

              <label title="Upload to library" className="upload" onClick={() => setId(id)}>
                Upload
                <input type="file" onChange={uploadToLibrary}/>
              </label>

              <button title="Delete library" className="delete" onClick={() => deleteLibrary((id))}>
                Delete
              </button>
            </td>
          </tr>
        ))}
        </tbody>
      </table>
    </main>
  );
};

export default AppLibrariesTable;
