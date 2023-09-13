import {Link, useMatch, useResolvedPath} from 'react-router-dom';

const AppLink = ({children, to, ...props}) => {
    const resolved = useResolvedPath(to);
    const match = useMatch({path: resolved.pathname, end: true});

    return (
        <Link className={match ? `active` : ''} to={to} {...props}>
            {children}
        </Link>
    );
};

export default AppLink;
