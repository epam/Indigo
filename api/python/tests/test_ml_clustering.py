import pytest
from sklearn.cluster import KMeans
from indigo.ml.clustering import clustering


@pytest.mark.parametrize(
    "args, kwargs, expecting",
    [
        (
            [[[1, 1], [2, 1], [1, 0], [4, 7], [3, 5]]],
            {
                "n_clusters": 2,
                "assign_labels": "discretize",
                "random_state": 0,
            },
            [0, 0, 0, 1, 1],
        ),
        (
            [[[1, 1], [2, 1], [1, 0], [4, 7], [3, 5]]],
            {"method": KMeans, "n_clusters": 2, "random_state": 0},
            [1, 1, 1, 0, 0],
        ),
    ],
)
def test_clustering_spectral(args, kwargs, expecting):
    assert clustering(*args, **kwargs) == expecting
