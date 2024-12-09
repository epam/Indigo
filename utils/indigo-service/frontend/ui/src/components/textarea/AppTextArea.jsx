import './AppTextArea.scss';


const AppTextArea = ({ title, onChange, ...otherProps }) => {
    return (
      <div className="textarea">
        <div class="textarea-title">{ title }</div>
        <textarea onChange={onChange} {...otherProps} />
      </div>
    );
};

export default AppTextArea;
