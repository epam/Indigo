import React, { useState } from 'react';
import AppFrame from '../components/frame/AppFrame';
import AppTypeFieldset from '../components/search-type/AppTypeFieldset';
import AppSearchLibrariesTable from '../components/search-libraries-table/AppSearchLibrariesTable';
import AppPropertiesSearch from '../components/search-properties/AppPropertiesSearch';
import { Values } from '../components/search-type/constants/types';
import {Values as MetricValues} from '../components/search-type/constants/simMetrics';
import Block from '../components/block/Block';
import './Search.scss';
import SearchStorageFieldset from '../components/search-storage-fieldset/SearchStorageFieldset';

const Search = () => {
    const [searchParams, setSearchParams] = useState({
        type: Values.Sub,
        metric: MetricValues.Tanimoto
    });

    const updateLibParams = (ids) =>
        setSearchParams({...searchParams, library_ids: ids});

    const updateTypeParams = (param) => {
        setSearchParams({...searchParams, ...param});
    };

    return (
        <>
            <div className={"search"} id={"form_was"}>
              <div className="editor-block">
                <Block title="Add/select a structure" number="1">
                  <AppFrame/>
                </Block>
              </div>
                <form className="execute-search-form">
                  <Block title="Execute search" number="2">
                    <div className="execute-search-form-content-wrapper">
                      <SearchStorageFieldset />
                      <AppTypeFieldset
                        updateType={updateTypeParams}
                        params={searchParams}
                      />
                      <AppSearchLibrariesTable updateLibIds={updateLibParams}/>
                    </div>
                  </Block>
                </form>
            </div>
            <AppPropertiesSearch params={searchParams}/>
        </>
    );
};

export default Search;
