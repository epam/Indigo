import './AppTypeFieldset.scss';
import AppSelect from '../select/AppSelect';
import {Types, Values} from './constants/types';
import {SimMetrics} from './constants/simMetrics';

const AppTypeFieldset = ({updateType, params}) => {
    const placeholder = 'Options';
    const legend = 'Search Type';

    return (
        <fieldset>
            <legend>{legend}</legend>
            <AppSelect
                value={params.type}
                options={Types}
                onChange={(value) => updateType({type: value})}
            />
            {params.type === Values.Sim ? (
                <>
                    <AppSelect
                        value={params.metric}
                        options={SimMetrics}
                        onChange={(value) => updateType({metric: value})}
                    />
                    <input
                        name="min_sim"
                        placeholder="Min"
                        size="3"
                        type="number"
                        min="0"
                        step="0.1"
                        value={params.min_sim || ''}
                        onChange={(event) => updateType({min_sim: +event.target.value})}
                    />
                </>
            ) : (
                <textarea
                    placeholder={placeholder}
                    rows="1"
                    value={params.options}
                    onChange={(event) => updateType({options: event.target.value})}
                ></textarea>
            )}
        </fieldset>
    );
};

export default AppTypeFieldset;
