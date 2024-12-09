import './AppInput.scss';


const AppInput = ({ title, onChange, ...otherProps }) => {
    return (
      <div className="input">
        <div class="input-title">{ title }</div>
        <input type="text" onChange={onChange} {...otherProps} />
      </div>
    );
};

export default AppInput;
