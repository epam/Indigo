import React, {useEffect, useState} from 'react';
import {useFetch} from '../../hooks/useFetch';
import ApiService from '../../api/ApiService';
import {Values} from '../search-type/constants/types';
import {Values as MetricValues} from '../search-type/constants/simMetrics';

import './AppPropertiesSearch.scss';

const AppPropertiesSearch = ({params}) => {
  const [query] = useState('');
  const [result, setResult] = useState(null);
  // const [searchId, setSearchId] = useState('');
  const [searchImg, setSearchImg] = useState('');
  const [searchCount, setSearchCount] = useState(null);
  const [limit, setLimit] = useState(20);
  const [offset] = useState(20);

  const getParams = (molFile) => {
    if (params.options === undefined) {
      params.options = ""
    }
    if (params.type !== Values.Sim) {
      delete params.min_sim;
      delete params.metric;
    } else {
      delete params.options;
      if (params.metric === undefined) {
        params.metric = MetricValues.Tanimoto;
      }
    }
    return {
      ...params,
      limit,
      offset,
      query_text: query,
      query_structure: molFile,
    };
  };

  function handleLimit() {
    setLimit(prevLimit => prevLimit + offset);
  }

  async function libUpdate() {
    try {
      var molFile = '';
      await (global).ketcher.getMolfile().then(result => {
        molFile = result
      })
      const {data} = await ApiService.search(getParams(molFile)).then(
        ({data: searchResp}) => {
          // setSearchId(searchResp.search_id);

          Promise.all(
            searchResp.result.map((res) =>
              ApiService.render({query: molFile, struct: res.structure})
            )
          ).then((imgs) =>
            setResult(
              imgs.map(({data: img}, i) => {
                return Object.assign(
                  {
                    img:
                      'data:image/svg+xml;charset=utf-8,' +
                      encodeURIComponent(img),
                  },
                  searchResp.result[i]
                );
              })
            )
          );

          return ApiService.searchById(searchResp.search_id);
        }
      );
      return {molFile, data};
    } catch (error) {
      // ignore me
    }
  }

  // eslint-disable-next-line react-hooks/exhaustive-deps
  useEffect(() => {
    async function callLib() {
      await libUpdate();
    }

    callLib();
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [limit]);

  const {0: search, 2: error} = useFetch(() => {
    async function callMe() {
      const lib_response = await libUpdate();
      const molFile = lib_response.molFile;
      const data = lib_response.data;

      if (data.state === 'SUCCESS') {
        setSearchCount(data.result.count);
      }

      const isEmpty = molFile.split('\n').length <= 6;
      if (!isEmpty) {
        const {data: searchImg} = await ApiService.render({query: molFile});
        setSearchImg(searchImg);
      }
      setLimit(20)
    }

    callMe();
  });
  const shouldShowButton = limit <= searchCount;
  return (
    <>
      <div className="props__block">
        {/*TODO not working on python+elastic and not correct working on python+postgres*/}
        {/* <label htmlFor="properties" className="props__label">*/}
        {/*     Properties*/}
        {/* </label>*/}
        {/* <input*/}
        {/*    name="properties"*/}
        {/*    className="props__input"*/}
        {/*    type="text"*/}
        {/*    placeholder="e.g. mass > 300"*/}
        {/*    value={query}*/}
        {/*    onChange={(event) => setQuery(event.target.value)}*/}
        {/* />*/}

        <input
          type="submit"
          value="Search"
          disabled={!(params.library_ids && params.library_ids.length)}
          onClick={search}
        />
      </div>
      <section className="output">
        <header className="query">
          {error && (
            <h2 style={{color: 'red', margin: '15px auto'}}>
              An error occurred: {error}
            </h2>
          )}
          {searchImg && (
            <img
              alt="Search"
              src={
                'data:image/svg+xml;charset=utf-8,' +
                encodeURIComponent(searchImg)
              }
            ></img>
          )}
          {searchImg && query && <em>and</em>}
          {query && <code>{query}</code>}
        </header>
        {result && (
          <>
            <div className="summary">
              {result.length > 0 && (
                <>
                  <p>
                    Total results:<var>{searchCount}</var>
                  </p>
                  {/*<a href={`${params.url}/libraries/search/${searchId}.sdf`}*/}
                  {/*   className="export">*/}
                  {/*    Export*/}
                  {/*</a>*/}
                </>
              )}
            </div>
            {result.length > 0 ? (
              <ul className="results">
                {result.slice(0, limit).map((res) => (
                  <li key={res.id}>
                    <h3>
                      {res.library_id} #<var>{res.id}</var>
                    </h3>
                    <img alt={res.id} src={res.img}></img>
                  </li>
                ))}
                {shouldShowButton && (
                  <button className="loadMore" onClick={handleLimit}>Load More</button>
                )
                }
              </ul>
            ) : (
              <strong>No results</strong>
            )}
          </>
        )}
      </section>
    </>
  );
};

export default AppPropertiesSearch;
