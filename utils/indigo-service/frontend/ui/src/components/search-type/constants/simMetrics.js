const Values = {
    Tanimoto: 'tanimoto',
    Tversky: 'tversky',
    EuclidSub: 'euclid-sub',
};

const Names = {
    Tanimoto: 'Tanimoto',
    Tversky: 'Tversky',
    EuclidSub: 'Euclid-sub',
};

const SimMetrics = [
    {name: Names.Tanimoto, value: Values.Tanimoto},
    {name: Names.Tversky, value: Values.Tversky},
    {name: Names.EuclidSub, value: Values.EuclidSub},
];

export {Values, Names, SimMetrics};
