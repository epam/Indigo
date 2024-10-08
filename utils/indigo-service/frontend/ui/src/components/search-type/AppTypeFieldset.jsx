import './AppTypeFieldset.scss';
import AppSelect from '../select/AppSelect';
import {Types, Values} from './constants/types';
import {SimMetrics} from './constants/simMetrics';
import AppInput from '../input/AppInput';
import AppTextArea from '../textarea/AppTextArea';

const AppTypeFieldset = ({updateType, params}) => {
    const placeholder = 'Options';
    const legend = 'Search Settings:';

    return (
        <fieldset>
            <legend>{legend}</legend>
            <AppSelect
                title="Search type"
                value={params.type}
                options={Types}
                onChange={(value) => updateType({type: value})}
            />
            {params.type === Values.Sim ? (
                <>
                    <AppSelect
                        title="Metric"
                        value={params.metric}
                        options={SimMetrics}
                        onChange={(value) => updateType({metric: value})}
                    />
                    <AppInput
                      title="Value"
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
              <AppTextArea
                title="Options"
                placeholder={placeholder}
                rows="1"
                value={params.options}
                onChange={(event) => updateType({options: event.target.value})}
              />
            )}
        </fieldset>
    );
};

export default AppTypeFieldset;
