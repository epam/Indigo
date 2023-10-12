import './AppSelect.scss';


const AppSelect = ({options, onChange}) => {
    return (
        <select onChange={(event) => onChange(event.target.value)}>
            {options.map(({name, value}) => (
                <option value={value} key={value}>
                    {name}
                </option>
            ))}
        </select>
    );
};

export default AppSelect;
