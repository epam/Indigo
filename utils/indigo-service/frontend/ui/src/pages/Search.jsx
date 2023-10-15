import React, {useState} from 'react';
import AppFrame from '../components/frame/AppFrame';
import AppTypeFieldset from '../components/search-type/AppTypeFieldset';
import AppSearchLibrariesTable from '../components/search-libraries-table/AppSearchLibrariesTable';
import AppPropertiesSearch from '../components/search-properties/AppPropertiesSearch';
import {Values} from '../components/search-type/constants/types';
import {Values as MetricValues} from '../components/search-type/constants/simMetrics';

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
                <AppFrame/>
                <form>
                    <div>
                        <AppTypeFieldset
                            updateType={updateTypeParams}
                            params={searchParams}
                        />
                        <AppSearchLibrariesTable updateLibIds={updateLibParams}/>
                    </div>
                </form>
            </div>
            <AppPropertiesSearch params={searchParams}/>
        </>
    );
};

export default Search;
