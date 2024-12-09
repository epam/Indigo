import './AppSelect.scss';


const AppSelect = ({ options, title, value, onChange }) => {
    return (
      <div className="select">
        <div class="select-title">{ title }</div>
        <div class="select-dropdown">
          <select value={value} onChange={(event) => onChange(event.target.value)}>
            {options.map(({name, value}) => (
              <option value={value} key={value}>
                {name}
              </option>
            ))}
          </select>
          <div className="select-arrow"></div>
        </div>
      </div>
    );
};

export default AppSelect;
