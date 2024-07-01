const url_postgres = process.env.REACT_APP_API_POSTGRES;
const url_elastic = process.env.REACT_APP_API_ELASTIC;

const Values = {
    Sub: 'sub',
    Exact: 'exact',
    Sim: 'sim',
};

const Names = {
    Sub: 'Substructure',
    Exact: 'Exact',
    Sim: 'Similarity',
};

const Endpoint_Names = {
    Postgres: 'Postgres',
    Elastic: 'Elastic'
};
const Endpoint_Values = {
    Postgres: url_postgres,
    Elastic: url_elastic
};

const Endpoint_Types = [
    {name: Endpoint_Names.Postgres, value: Endpoint_Values.Postgres},
    {name: Endpoint_Names.Elastic, value: Endpoint_Values.Elastic},
];

const Types = [
    {name: Names.Sub, value: Values.Sub},
    {name: Names.Exact, value: Values.Exact},
    {name: Names.Sim, value: Values.Sim},
];

export {Types, Values, Names, Endpoint_Types, Endpoint_Values, Endpoint_Names};
