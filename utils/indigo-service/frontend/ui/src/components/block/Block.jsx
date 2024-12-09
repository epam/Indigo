import './Block.scss';
import React from 'react';

const Block = ({ children, title, number }) => {
    return (
        <div className="block">
          <div className="block-header">
            <div className="block-header-number">{ number }</div>
            { title }
          </div>
          <div className="block-body">
            {children}
          </div>
        </div>
    )
};

export default Block;
