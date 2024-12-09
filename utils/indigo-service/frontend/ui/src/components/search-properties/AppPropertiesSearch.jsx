import React, {useEffect, useState} from 'react';
import {useFetch} from '../../hooks/useFetch';
import ApiService from '../../api/ApiService';
import {Values} from '../search-type/constants/types';
import {Values as MetricValues} from '../search-type/constants/simMetrics';

import './AppPropertiesSearch.scss';
import Block from '../block/Block';

const AppPropertiesSearch = ({params}) => {
  const [query] = useState('');
  const [result, setResult] = useState(null);
  // const [searchId, setSearchId] = useState('');
  const [searchImg, setSearchImg] = useState('');
  const [searchCount, setSearchCount] = useState(null);
  const [limit, setLimit] = useState(20);
  const [offset] = useState(20);
  const [isLoading, setIsLoading] = useState(false);

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
      setResult(null);
      setSearchImg('');
      setIsLoading(true);
      try {
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
        setIsLoading(false);
      } catch (e) {
        setIsLoading(false);
      }
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



        <button
          type="submit"
          value="Search"
          class="search-button"
          disabled={!(params.library_ids && params.library_ids.length)}
          onClick={search}
        >
          <svg width="12" height="12" viewBox="0 0 12 12" fill="none" xmlns="http://www.w3.org/2000/svg"><path fill-rule="evenodd" clip-rule="evenodd" d="M4.45315 0.619141C2.33605 0.619141 0.619812 2.33538 0.619812 4.45247C0.619812 6.56957 2.33605 8.28581 4.45315 8.28581C5.27088 8.28581 6.02881 8.02976 6.65113 7.59346L10.2431 11.1854L11.1859 10.2426L7.59398 6.65069C8.03037 6.02832 8.28648 5.27031 8.28648 4.45247C8.28648 2.33538 6.57024 0.619141 4.45315 0.619141ZM1.95315 4.45247C1.95315 3.07176 3.07243 1.95247 4.45315 1.95247C5.83386 1.95247 6.95315 3.07176 6.95315 4.45247C6.95315 5.83319 5.83386 6.95247 4.45315 6.95247C3.07243 6.95247 1.95315 5.83319 1.95315 4.45247Z" fill="white"/></svg>
          Search
        </button>
      </div>
        <section className="output">
          <Block
            title="Get results"
            number="3"
            className="get-results-block"
          >
          <header className="query">
            {isLoading && <div className="loader"></div>}
            {error && (
              <h2 style={{color: 'red', margin: '15px auto'}}>
                An error occurred: {error}
              </h2>
            )}
            {searchImg && (
              <>
                <div className="summary">
                  {result && result.length > 0 ? (
                    <>
                      <p>
                        { searchCount } structures found for:
                      </p>
                      {/*<a href={`${params.url}/libraries/search/${searchId}.sdf`}*/}
                      {/*   className="export">*/}
                      {/*    Export*/}
                      {/*</a>*/}
                    </>
                  ) : (result && result.length === 0 && (
                    <p>No results for:</p>
                  ))}
                </div>
                <img
                  alt="Search"
                  src={
                    'data:image/svg+xml;charset=utf-8,' +
                    encodeURIComponent(searchImg)
                  }
                ></img>
              </>
            )}
            {searchImg && query && <em>and</em>}
            {query && <code>{query}</code>}
          </header>
          {result && (
            <>
              {result.length > 0 && (
                <ul className="results">
                  {result.slice(0, limit).map((res) => (
                    <li key={res.id}>
                      <h3>
                        {res.library_id} #<var>{res.id}</var>
                      </h3>
                      <img alt={res.id} src={res.img}></img>
                      <div className="properties"><pre>{JSON.stringify(res.properties, null, 2) }</pre></div>
                    </li>
                  ))}
                  {shouldShowButton && (
                    <button className="loadMore" onClick={handleLimit}>Load More</button>
                  )
                  }
                </ul>
              )}
            </>
          )}
          </Block>
        </section>
    </>
  );
};

export default AppPropertiesSearch;
